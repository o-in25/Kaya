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


static int terminalHandler(device_PTR devAddrBase) {
    int status = (devAddrBase->t_transm_status & TRANSREADY);
    if(status != READY) {
        /* acknowledge that the command is a transmit command 
        by providing the acknowledge bit */
        (*devAddrBase).t_transm_command = ACK;
        /* set the status to be a transmit status */
        return (*devAddrBase).t_transm_status;
    } else {
        /* acknowledge that the command is a recieve command 
        by providing the acknowledge bit */
        (*devAddrBase).t_recv_command = ACK;
        /* set the status to be a recieve status */
        return (*devAddrBase).t_recv_status;
    }
}

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


/*
* Function: The interrupt handler 
* 
*/
void interruptHandler() {
    debugA(8079);
    /* the old interrupt */
    state_PTR oldInterupt = (state_PTR) INTRUPTOLDAREA;
    device_PTR devAddrBase;
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
    } else if((cause & FOURTH) != 0) {
        lineNumber = DISKINT;
    } else if((cause & FIFTH) != 0) {
        lineNumber = TAPEINT;
    } else if((cause & TAPEINT) != 0) {
        lineNumber = NETWINT;
    } else if((cause & SEVENTH) != 0) {
        lineNumber = PRNTINT;
    } else if((cause & EIGHTH) != 0) {
        lineNumber = TERMINT;
    }
    /* since the find device number helper function does not save
    the modified line number, it must be done outside the function */
    debugA(lineNumber);
    lineNumber = lineNumber - NOSEM;
    /* DEBUG NOTES: makes it to here */
    deviceNumber = getDeviceNumber(lineNumber);

    /* given an interrupt line number and a device number, the
    starting address of the device's devreg by using...
    0x10000050 + line number - 3 * 0x80 + device number * 0x10 */
    devAddrBase = DEVREG + lineNumber * DEVPERINT * DEVREGSIZE + (deviceNumber * DEVREGSIZE);
    debugA(8092);


    if(lineNumber == TERMINT) {
        /* skip for now */
        status = terminalHandler(devAddrBase);
        /* was it was a transmit command? */
        if(status == devAddrBase->t_transm_status) {
            /* get the device index */
            index = (DEVPERINT * (lineNumber)) + deviceNumber;
        } else {
            index = (DEVPERINT * (lineNumber + 1)) + deviceNumber;
        }
    } else {
        /* not a terminal interrupt - assign the index */
        status = devAddrBase->d_status;
        devAddrBase->d_command = ACK;
        index = DEVPERINT * lineNumber + deviceNumber;
    }
    
    debugA(8093);
    /* perform a V operation on the semaphore */
    int* semaphore = &(semdTable[index]);
    (*semaphore)--;
    if((*semaphore) <=0) {
        pcb_PTR p = removeBlocked(semaphore);
        if(p != NULL) {
            p->p_state.s_v0 = status;
            insertProcQ(&(readyQueue), p);
            softBlockedCount--;
        }
    }
    debugA(8094);
    /* exit */
    exitInterruptHandler(startTime);
}