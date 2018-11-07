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
    devregarea_PTR devReg = (devregarea_PTR) RAMBASEADDR;
    unsigned int* deviceBitMap = (unsigned int*) INTBITMAP + ((lineNumber - NOSEM) * DEVREGLEN);
    debugA(991);
    /* start at the first device */
    unsigned int candidate = STARTDEVICE;
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
        if((((*deviceBitMap) & candidate) == candidate)) {
            /* since this index is equal to 1, we found it */
            found = TRUE;
        } else {
            /* it's not this device, so increment and try again */
            deviceNumber++;
            /* bitwise shift right and go to the next one */
            candidate = candidate << 1;
        }
    }
    return candidate;
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
    int lineNumbers[(DEVPERINT - NOSEM)] = {FOURTH, FIFTH, SIXTH, SEVENTH};
    int devices[(DEVPERINT - NOSEM)] = {DISKINT, TAPEINT, NETWINT, PRNTINT, TERMINT};
    int i;
    for(i = 0; i < (DEVPERINT - NOSEM); i++) {
        if ((cause & lineNumbers[i] != 0)) {
            result = devices[i];
        }
    }
}


/*
* Function: The interrupt handler 
* 
*/
void interruptHandler() {
    debugA(8079);
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
    debugA(8080);
    if((cause & FIRST) != 0) {
        debugA(8081);
        PANIC();
    } else if((cause & SECOND) != 0) {
        debugA(8082);
        exitInterruptHandler(startTime);
        /* skip for now */
    } else if((cause & THIRD) != 0){
        debugA(8083);
        int* semaphore = &(semdTable[MAXSEMALLOC - 1]);
        debugA(8084);
        while(headBlocked(semaphore) != NULL) {
            STCK(endTime);
            debugA(8085);
            pcb_PTR p = removeBlocked(semaphore);
            if(p != NULL) {
                debugA(8086);
                insertBlocked(&(readyQueue), p);
                softBlockedCount--;
                /* handle the charging of time */
                STCK(endTime);
                currentProcess->p_time += endTime - startTime;
            }
            debugA(8087);
            /* handle the charging of time */
            exitInterruptHandler(startTime);
        }
        debugA(8088);
    } else {
        lineNumber = map(cause);
    }
    /* since the find device number helper function does not save
    the modified line number, it must be done outside the function */
    debugA(lineNumber);
    /* DEBUG NOTES: makes it to here */
    deviceNumber = getDeviceNumber(lineNumber);
    lineNumber -= NOSEM;
    /* have both line and device numbers, calculate the device register */
    devReg = (device_PTR) (INTDEVREG + lineNumber * DEVREGSIZE * DEVPERINT) + (deviceNumber * DEVREGSIZE);
    /* handle the terminal, if the terminal is causing the interrupt. else, acknowledge the 
    reception of the terminal interrupt in the overwritten command recieved field */
    if(lineNumber == TERMINT) {
        int receive = TRUE;
        if((devReg->t_transm_status & 0x0F) != READY) {
            index = DEVPERINT * (lineNumber) + deviceNumber;
            recieve = FALSE;
        }
        int* semaphore = &(semdTable[index]);
        (*semaphore)++;
        if((*semaphore) <= 0) {
            pcb_PTR p = removeBlocked(semaphore);
            if(p != NULL) {
                if(receieve) {
                    /* acknowledge the transmission */
                    devReg->t_recv_command = ACK;
                    p->p_state.s_v0 = devReg->t_recv_status;
                } else {
                    devReg->t_transm_status = ACK;
                    /* acknowledge the transmission */
                    p->p_state.s_v0 = devReg->t_transm_status;
                }
                softBlockedCount--;
                insertProcQ(&(readyQueue), p);
            }
        }
        exitInterruptHandler(startTime);
    } else {
        index = DEVPERINT * (lineNumber - NOSEM) + deviceNumber;
        devReg->d_command = ACK;
        int* semaphore = &(semdTable[index]);
        (*semaphore)++;
        if((*semaphore) <= 0) {
            pcb_PTR p = removeBlocked(semaphore);
            if(p != NULL) {
                p->p_state.s_v0 = devReg->d_status;
                softBlockedCount--;
                insertProcQ(&(readyQueue), p);
            }
        }
        exitInterruptHandler(startTime);
    }
}