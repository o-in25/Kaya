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
static int getDeviceNumber(int lineNumber) {
    /* get the address of the device bit map. Per secion 5.2.4 of pops, the 
    physical address of the bit map is 0x1000003C. When bit i is in word j is 
    set to one then device i attached to interrupt line j + 3 */
    devregarea_PTR temp = (devregarea_PTR) RAMBASEADDR;
    unsigned int deviceBitMap = temp->interrupt_dev[(lineNumber - NOSEM)];
    /* start at the first device */
    unsigned int candidate = FIRST;
    int deviceNumber = 0;
    /* for searching for the device number */
    int found = FALSE;
    /* search each 8 bits */
    for(deviceNumber; deviceNumber < DEVPERINT; deviceNumber++) {
        if((deviceBitMap & candidate) != 0) {
            break;
        } else {
            candidate << 1;
        }
    }
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
    state_PTR oldInterrupt = (memaddr) INTRUPTOLDAREA;
    cpu_t endTime;
    if(currentProcess != NULL) {
        STCK(endTime);
        currentProcess->p_time += (endTime - startTime);
        copyState(oldInterrupt, &(currentProcess->p_state));
        insertProcQ(&(readyQueue), currentProcess);
    }
    invokeScheduler();
}


int map(unsigned int cause) {
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
    return finding;
}

static void intervalTimerHandler(cpu_t startTime, cpu_t endTime) {
    LDIT(INTERVAL);
    int *semaphore = &(semdTable[MAXSEMALLOC - 1]);
    while (headBlocked(semaphore) != NULL) {
        STCK(endTime);
        pcb_PTR p = removeBlocked(semaphore);
        if (p != NULL) {
            cpu_t elapsedTime = (endTime - startTime);
            /* handle the charging of time */
            currentProcess->p_time += elapsedTime;
            insertProcQ(&(readyQueue), p);
            softBlockedCount--;
        }
    }
    /*handle the charging of time */
    (*semaphore) = 0;
    exitInterruptHandler(startTime);
}


/*
* Function: The interrupt handler 
* 
*/
void interruptHandler() {
    /* the old interrupt */
    state_PTR oldInterupt = (state_PTR) INTRUPTOLDAREA;
    device_PTR devReg;
    unsigned int cause = oldInterupt->s_cause;
    debugThisSonOfAFuck(cause);
    cause += (cause & IM) >> 8;
    cpu_t startTime;
    cpu_t endTime;
    int deviceNumber = 0;
    int lineNumber = 0;
    int index = 0;
    int status = 0;
    if ((cause & FIRST) != 0) {
        PANIC();
    } else if((cause & SECOND) != 0) {
        exitInterruptHandler(startTime, endTime);
        /* skip for now */
    } else if((cause & THIRD) != 0) {
       intervalTimerHandler(startTime, endTime);
    } else if((cause & FOURTH) != 0) {
        lineNumber = DISKINT;
    } else if((cause & FIFTH) != 0) {
        lineNumber = TAPEINT;
    } else if((cause & SIXTH) != 0) {
        lineNumber = NETWINT;
    } else if((cause & SEVENTH) != 0) {
        lineNumber = PRNTINT;
    } else if((cause & EIGHTH) != 0) {
        lineNumber = TERMINT;
    } else {
        PANIC();
    }
    /* since the find device number helper function does not save
    the modified line number, it must be done outside the function */
    /* DEBUG NOTES: makes it to here */
    deviceNumber = getDeviceNumber(lineNumber);
    debugThisSonOfAFuck(softBlockedCount);
    debugThisSonOfAFuck(((lineNumber - 2) * 8) + deviceNumber);
    /* have both line and device numbers, calculate the device register */
    devReg = (device_PTR) (INTDEVREG + ((lineNumber - NOSEM) * DEVREGSIZE * DEVPERINT) + (deviceNumber * DEVREGSIZE));
    /* handle the terminal, if the terminal is causing the interrupt. else, acknowledge the 
    reception of the terminal interrupt in the overwritten command recieved field */
    if(lineNumber == TERMINT) {
        int receive = TRUE;
        if((devReg->t_transm_status & 0x0F) != READY) {
            index = DEVPERINT * (lineNumber - NOSEM) + deviceNumber;
            receive = FALSE;
        } else {
            index = DEVPERINT * ((lineNumber - NOSEM) + 1) + deviceNumber;
        }
        int* semaphore = &(semdTable[index]);
        (*semaphore)++;
        if((*semaphore) <= 0) {
            pcb_PTR p = removeBlocked(semaphore);
            if(p != NULL) {
                if(receive) {
                    /* acknowledge the transmission */
                    p->p_state.s_v0 = devReg->t_recv_status;
                    devReg->t_recv_command = ACK;
                } else {
                    p->p_state.s_v0 = devReg->t_transm_status;
                    devReg->t_transm_command = ACK;
                    /* acknowledge the transmission */
                }
                softBlockedCount--;
                insertProcQ(&(readyQueue), p);
            }
        }
        exitInterruptHandler(startTime);
    } else {
        index = DEVPERINT * (lineNumber - NOSEM) + deviceNumber;
        /* DEBUG NOTES: ended up here */
        int* semaphore = &(semdTable[index]);
        (*semaphore)++;
        if ((*semaphore) <= 0) {
            pcb_PTR p = removeBlocked(semaphore);
            if(p != NULL) {
                p->p_state.s_v0 = devReg->d_status;
                devReg->d_command = ACK;
                softBlockedCount--;
                insertProcQ(&(readyQueue), p);
            }
        }
        exitInterruptHandler(startTime);
    }
}