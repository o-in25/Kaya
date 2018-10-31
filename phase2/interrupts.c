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
    const unsigned int* deviceBitMap = (memaddr) DEVREGLEN + (lineNumber - NOSEM) * WORDLEN;
    unsigned int* map;
 
    /* start at the first device */
    unsigned int candidate = STARTDEVICE;
    int i = 0;
    /* search each 8 bits */
    for(i; i < STARTDEVICE + 7; i++) {
        /* if the bit i in word j is set to 1, then
        the device attached to interrupt j + 3 has a pending 
        interrupt */
        if((candidate & (*deviceBitMap) == candidate)) {
            /* since this index is equal to 1, we found it */
            return i;
        } else {
            /* bitwise shift right and go to the next one */
            candidate = candidate << 1;
        }
    }
}

/*
*
*/
static int getLineNumber(int cause) {
int lineNumbers[LINECOUNT - 2] = {
        LINETHREE,
        LINEFOUR,
        LINEFIVE,
        LINESIX,
        LINESEVEN
    };
    int i; 
    for(i = 2; i < LINECOUNT; i++) {
        if((cause & lineNumbers[i]) == lineNumbers[i]) {
            /* found the line number */
                return lineNumbers[i];
        }
    }
}

static unsigned int terminalHandler(device_PTR devAddrBase) {
    unsigned int status = (devAddrBase->t_transm_status & TRANSREADY);
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
    state_PTR oldInterrupt = (memaddr) INTRUPTOLDAREA;
    cpu_t endTime;
    if(currentProcess != NULL) {
        STCK(startTime);
        currentProcess->p_time += (endTime - startTime);
        
        copyState(oldInterrupt, &(currentProcess.p_state));
        insertProcQ(&(readyQueue), currentProcess);
        softBlockedCount--;
    }
    invokeScheduler();
}

void interruptHandler() {
    /* the old interrupt */
    state_PTR oldInterupt = (state_PTR) INTRUPTOLDAREA;
    device_PTR devAddrBase;
    const unsigned int cause = oldInterupt->s_cause;
    cpu_t startTime;
    cpu_t endTime;
    int deviceNumber = 0;
    int lineNumber = 0;
    int index = 0;
    int status = 0;
    if((cause & LINEZERO) != 0) {
        PANIC();
    } else if((cause & LINEONE) != 0) {
        exitInterruptHandler(startTime);
        /* skip for now */
    } else if((cause * LINETWO) != 0){
        /* get the last device */
        int* semaphore = &(semdTable[48]);
        while(headBlocked(semaphore) != NULL) {
            STCK(endTime);
            pcb_PTR p = removeBlocked(semaphore);
            if(p != NULL) {
                insertBlocked(&(readyQueue), p);
                softBlockedCount--;
                /* handle the charging of time */
                STCK(endTime);
                currentProcess->p_time += endTime - startTime;
            }
            /* handle the charging of time */
            exitInterruptHandler(startTime);
        }
    } else {
        lineNumber = getLineNumber(cause);
    }
    deviceNumber = getDeviceNumber(cause);
    /* since the find device number helper function does not save
    the modified line number, it must be done outside the function */
    lineNumber = lineNumber - NOSEM;
    /* given an interrupt line number and a device number, the
    starting address of the device's devreg by using...
    0x10000050 + line number - 3 * 0x80 + device number * 0x10 */
    devAddrBase = DEVREG + lineNumber * DEVICECOUNT + (deviceNumber * DEVREGSIZE); 
    if(lineNumber == TERMINT) {
        /* skip for now */
        status = terminalHandler(devAddrBase);
        /* was it was a transmit command? */
        if(status == devAddrBase->t_transm_status) {
            /* get the device index */
            index = (DEVPERINT * (lineNumber - NOSEM)) + deviceNumber;
        } else {
            index = (DEVPERINT * (lineNumber - (NOSEM + 1))) + deviceNumber;
        }
    } else {
        /* not a terminal interrupt - assign the index */
        status = devAddrBase->d_status;
        devAddrBase->d_command = ACK;
        index = DEVPERINT + (lineNumber - NOSEM) + deviceNumber;
    }
    /* perform a V operation on the semaphore */
    int* semaphore = &(semdTable[index]);
    (*semaphore)--;
    if((*semaphore) <=0) {
        pcb_PTR p = removeBlocked(semaphore);
        if(p != NULL) {
            p.p_state.s_v0 = status;
            insertProcQ(&(readyQueue), p);
        }
    }

    exitInterruptHandler(startTime);

    

    /* line number found, now find the corresponding device 
    number */


}