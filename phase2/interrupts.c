/*************************************************** interrupts.c ************************************************************
	Interrupts.c is the I/O handler. In the I/O handler, the following steps are taken: first, the
    interrupt line that is on is determined (for convience and organization, the devices 3-7 are stored
    as constants in constants.h). Next, with the line number, the device instance must be determined, 
    and the address of that device's regsiter must be acquired.  

***************************************************** interrupts.c ************************************************************/

/* h files to include */
#include "../h/const.h"
#include "../h/types.h"
/* e files to include */
#include "../e/pcb.e"
#include "../e/asl.e"
#include "../e/initial.e"
#include "../e/exceptions.e"
#include "../e/scheduler.e"
#include "/usr/local/include/umps2/umps/libumps.e"

/************************************************************************************************************************/
/******************************************** HELPER FUNCTIONS  *********************************************************/
/************************************************************************************************************************/

/*
* Function: Get Device Number 
* The interrupting devices bit map IDBM is a read-only 5 word 
* area that indicates which devices have an interrupt pending.
* When bit i in word j is equal to 1, the device associated with
* that corresponding bit has an interrupt pending. Since the word
* will be supplied prior to this function call, this will simply 
* calculate the device number  
*/
static unsigned int getDeviceNumber(int lineNumber) {
    /* get the address of the device bit map. Per secion 5.2.4 of pops, the 
    physical address of the bit map is 0x1000003C. When bit i is in word j is 
    set to one then device i attached to interrupt line j + 3 */
    debugA(999);
    debugA(lineNumber);
    unsigned int deviceBitMap = (unsigned int*) INTBITMAP + ((lineNumber - NOSEM) * DEVREGLEN);
    debugA(991);
    /* start at the first device */
    int candidate = FIRST;
    int deviceNumber = 0;
    /* for searching for the device number */
    int found = FALSE;
    debugA(992);
    /* search each 8 bits */
    while(!found) {
        debugA(993);
        /* if the bit i in word j is set to 1, then
        the device attached to interrupt j + 3 has a pending 
        interrupt */
        if((deviceBitMap & candidate) != 0) {
            /* since this index is equal to 1, we found it */
            found = TRUE;
        } else {
            /* it's not this device, so increment and try again */
            deviceNumber++;
            /* bitwise shift right and go to the next one */
            candidate = candidate << 1;
        }
    }
    debugA(995);
    return deviceNumber;
}

/*
* Function: Get line number
* Gets the line number of the non-cpu and non-clock devices,
* given the cause integer. This function gets invoked whenever the 
* cause is for the interrupt is not a cpu interrupt, a local timer 
* interrupt, or a interval time interrupt. If the cause is neither of
* these, this simply determines which line it is
*/

static void exitInterruptHandler(cpu_t startTime) {
    debugA(444);
    state_PTR oldInterrupt = (memaddr) INTRUPTOLDAREA;
    debugA(445);
    cpu_t endTime;
    debugA(446);
    if(currentProcess != NULL) {
        debugA(447);
        STCK(startTime);
        debugA(448);
        currentProcess->p_time += (endTime - startTime);
        debugA(449);
        copyState(oldInterrupt, &(currentProcess->p_state));
        debugA(450);
        insertProcQ(&(readyQueue), currentProcess);
        debugA(451);
    }
    debugA(452);
    invokeScheduler();
}


int map(int cause) {
    /* declare the array of possible line numbers */
    int lineNumbers[(DEVPERINT - NOSEM)] = {FOURTH, FIFTH, SIXTH, SEVENTH, EIGHTH};
    int devices[(DEVPERINT - NOSEM)] = {DISKINT, TAPEINT, NETWINT, PRNTINT, TERMINT};
    int i;
    int finding = 0;
    for(i = 0; i < (DEVPERINT - NOSEM); i++) {
        if((cause & lineNumbers[i]) != 0) {
            finding = devices[i];
        }
    }
    debugA(666666666);
    debugA(finding);
    return finding;
}


/*
* Function: The interrupt handler 
* 
*/
void interruptHandler() {
    debugA(40000);
    /* the old interrupt */
    state_PTR oldInterupt = (state_PTR) INTRUPTOLDAREA;
    device_PTR devReg;
    const unsigned int cause = oldInterupt->s_cause;
    cause += (cause & IM) >> 8;
    debugA(cause);
    cpu_t startTime;
    cpu_t endTime;
    int deviceNumber = 0;
    int lineNumber = 0;
    int index = 0;
    int status = 0;
    debugA(40001);
    if ((cause & FIRST) != 0) {
        debugA(40002);
        PANIC();
    } else if((cause & SECOND) != 0) {
        debugA(40003);
        exitInterruptHandler(startTime);
        /* skip for now */
    } else if((cause & THIRD) != 0){
        debugA(40004);
        int *semaphore = &(semdTable[MAXSEMALLOC - 1]);
        debugA(40005);
        while (headBlocked(semaphore) != NULL)
        {
            STCK(endTime);
            debugA(40006);
            pcb_PTR p = removeBlocked(semaphore);
            if(p != NULL) {
                debugA(40007);
                insertProcQ(&(readyQueue), p);
                softBlockedCount--;
                /* handle the charging of time */
                STCK(endTime);
                currentProcess->p_time += endTime - startTime;
            }
            debugA(40009);
            /* handle the charging of time */
            exitInterruptHandler(startTime);
        }
        debugA(40010);
    } else {
        debugA(300012);
        lineNumber = map(cause);
    }
    /* since the find device number helper function does not save
    the modified line number, it must be done outside the function */
    debugA(40098);
    debugA(lineNumber);
    debugA(40011);
    /* DEBUG NOTES: makes it to here */
    deviceNumber = getDeviceNumber(lineNumber);
    debugA(6969696);
    debugA(lineNumber);
    debugA(40012);
    /* have both line and device numbers, calculate the device register */
    devReg = (device_PTR) (INTDEVREG + ((lineNumber) - NOSEM * DEVREGSIZE * DEVPERINT) + (deviceNumber * DEVREGSIZE));
    /* handle the terminal, if the terminal is causing the interrupt. else, acknowledge the 
    reception of the terminal interrupt in the overwritten command recieved field */
    debugA(40013);
    debugA(lineNumber);
    if(lineNumber == TERMINT) {
        int receive = TRUE;
        debugA(40014);
        if((devReg->t_transm_status & 0x0F) != READY) {
            index = DEVPERINT * (lineNumber - NOSEM) + deviceNumber;
            receive = FALSE;
        } else {
            index = DEVPERINT * ((lineNumber - NOSEM) + 1) + deviceNumber;
        }
        int* semaphore = &(semdTable[index]);
        debugA(40015);
        (*semaphore)++;
        if((*semaphore) <= 0) {
            pcb_PTR p = removeBlocked(semaphore);
            debugA(40016);
            if(p != NULL) {
                if(receive) {
                    debugA(40017);
                    /* acknowledge the transmission */
                    devReg->t_recv_command = ACK;
                    p->p_state.s_v0 = devReg->t_recv_status;
                } else {
                    debugA(40018);
                    devReg->t_transm_status = ACK;
                    /* acknowledge the transmission */
                    p->p_state.s_v0 = devReg->t_transm_status;
                }
                debugA(40019);
                softBlockedCount--;
                insertProcQ(&(readyQueue), p);
            }
        }
        debugA(40020);
        exitInterruptHandler(startTime);
    } else {
        debugA(40021);
        index = DEVPERINT * (lineNumber - NOSEM) + deviceNumber;
        debugA(40020);
        /* DEBUG NOTES: ended up here */
        devReg->d_command = ACK;
        debugA(40022);
        int* semaphore = &(semdTable[index]);
        debugA(40023);
        (*semaphore)++;
        debugA(40023);
        if ((*semaphore) <= 0) {
            pcb_PTR p = removeBlocked(semaphore);
            if(p != NULL) {
                debugA(40024);
                p->p_state.s_v0 = devReg->d_status;
                softBlockedCount--;
                insertProcQ(&(readyQueue), p);
            }
            debugA(40025);
        }
        exitInterruptHandler(startTime);
    }
}