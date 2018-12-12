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
    int deviceNumber;
    /* for searching for the device number */
    /* search each 8 bits */
    for(deviceNumber = 0; deviceNumber < DEVPERINT; deviceNumber++) {
        if((deviceBitMap & candidate) != 0) {
            /* the candidate has been found */
            break;
        } else {
            /* find the next candidate */
            candidate = candidate << 1;
        }
    }
    /* we found the device numner */
    return deviceNumber;
}

/*
* Function: Exit Interrupt Handler
* Ensures that the current process will not be charged for
* the time in the interrupt handler - if an interrupt occured 
* whem the current process is running
*/
static void exitInterruptHandler(cpu_t startTime) {
    /* get the old interrupt area */
    state_PTR oldInterrupt = (memaddr) INTRUPTOLDAREA;
    /* the process should not be null */
    if(currentProcess != NULL) {
        cpu_t endTime;
        /* start the clock by placing a new value in 
        the ROM supported STCK function */
        STCK(endTime);
        /* find the startedTOD */
        cpu_t elapsedTime = (endTime - startTime);
        startTOD = startTOD + elapsedTime;
        /* copy the state from the old interrupt area to the current state */
        copyState(oldInterrupt, &(currentProcess->p_state));
        /* insert the new pricess in the ready queue */
        insertProcQ(&(readyQueue), currentProcess);
    }
    /* get a new process */
    invokeScheduler();
}

/*
* Function: Get line number
* Gets the line number of the non-cpu and non-clock devices,
* given the cause integer. This function gets invoked whenever the 
* cause is for the interrupt is not a cpu interrupt, a local timer 
* interrupt, or a interval time interrupt. If the cause is neither of
* these, this simply determines which line it is
*/
int getLineNumber(unsigned int cause) {
    /* declare the array of possible line numbers */
    unsigned int lineNumbers[(DEVPERINT - NOSEM)] = {FOURTH, FIFTH, SIXTH, SEVENTH, EIGHTH};
    /* declare the array of possible line numbers */
    unsigned int devices[(DEVPERINT - NOSEM)] = {DISKINT, TAPEINT, NETWINT, PRNTINT, TERMINT};
    int i;
    int finding = 0;
    /* loop through each possible device */
    for(i = 0; i < (DEVPERINT - NOSEM); i++) {
        if(((cause) & (lineNumbers[i])) != 0) {
            /* match the line number with the device */
            finding = devices[i];
        }
    }
    return finding;
}

/*
* Function: Interval Timer Handler
* The handler for the interrupting interval timer handler. 
* Performs a V operation on the nucleus maintained semaphore associated with the interrupting (sub)device. 
* The kernel maintains two semaphores for each terminal subdevice. For Interval 
* Timer interrupts that represent a pseudo-clock tick, perform the V operation on the kernel
* maintained pseudo-clock timer semaphore.
*/
static void intervalTimerHandler(cpu_t startTime, cpu_t endTime) {
    LDIT(INTERVAL);
    /* get the index of the last device in the device 
    semaphore list - which is the interval timer */
    int* semaphore = &(semdTable[MAXSEMALLOC - 1]);
    /* reset the  */
    (*semaphore) = 0;
    /* get all of the blocked devices*/
    pcb_PTR blocked = headBlocked(semaphore);
    while(blocked != NULL) {
        pcb_PTR p = removeBlocked(semaphore);
        STCK(endTime);
        if(p != NULL) {
            /* a process has been freed up */
            insertProcQ(&(readyQueue), p);
            /* the elapsed time is the start minus the end */
            cpu_t elapsedTime = (endTime - startTime);
            /* handle the charging of time */
            (p->p_time) = (p->p_time) + elapsedTime;
            /* one less device waiting */
            softBlockedCount--;
            /* go again */
            blocked = headBlocked(semaphore);
        }
    }
    /* exit the interrupt handler - from which this process had 
    come from */
    exitInterruptHandler(startTime);
}

/*
* Function: The interrupt handler 
* Will handler interrupts that are caused by various interrupting devices, such 
* as terminal devices, printer devices, network devices (though this is not implemented
* at this phase, tape devices, and disk devices. Additionally, it handles interrupts from a 
* psuedo-clock timer to signify a process' specific quantum is over. The interval timer handler 
* will analyze the contents of the cause register to see what happened - i.e. what is the cause line
* number for this particular interrupt. Based on the cause, if it is line number is 0, it is an 
* inter-processor interrupt and handled with a kernel panic - since it is not supported in Kaya.
* If the line number is the processor local timer, the interrupt handler will then exit the 
* interrupt handler by entering the exit handler - which will then get a new job from the scheduler. If
* the interrupting line was the interal timer bus, then it will be passed to the interval timer handler 
* which will treat the interrupt as a pseudo-clock tick. Finally, for all other interrupts, their 
* line and device numbers are computed through subroutines. From their line and device numbers, their 
* synchronization semaphore index is calculated as well as their device register area. For terminal
* interrupts, a distinction must be made between a send command and a recieve command. Finally, once
* these have been computed, the interupt handler will perform a V operation on that semaphore and 
* implement the umps2 interrupt-driven handshake protocol 
*/
void interruptHandler() {
    /* the old interrupt area */
    state_PTR oldInterupt = (state_PTR) INTRUPTOLDAREA;
    /* the device register */
    device_PTR devReg;
    /* the cause for the interrupt is stored in the cause register */
    unsigned int cause = oldInterupt->s_cause;
    /* bitwside shift the cause by eight */
    cause += (cause & IM) >> 8;
    /* the start timer */
    cpu_t startTime;
    /* the end time */
    cpu_t endTime;
    /* start the clock by placing a new value in the ROM-dedicated 
    STCK function */
    STCK(startTime);
    /* start both device and line number at 0 */
    int deviceNumber = 0;
    int lineNumber = 0;
    /* initialize the index */
    int i = 0;
    /* what happened? */
    if ((cause & FIRST) != 0) {
        /* the cause should not be 0 - since it is not supported 
        in Kaya */
        PANIC();
    } else if((cause & SECOND) != 0) {
        /* processor local timer */
        exitInterruptHandler(startTime);
    } else if((cause & THIRD) != 0) {
        /* go to the interval timer handler */
        intervalTimerHandler(startTime, endTime);
    } else {
        /* if it was not lines 0-2, get the line number */
        lineNumber = getLineNumber(cause);
    }
    /* get the device number */
    deviceNumber = getDeviceNumber(lineNumber);
    /* now that we have the device number and the line number, we compute the well-known
    address in memory */
    devReg = (device_PTR) (INTDEVREG + ((lineNumber - NOSEM) * DEVREGSIZE * DEVPERINT) + (deviceNumber * DEVREGSIZE));
    /* assume the receive is true */
    int receive = TRUE;
    /* if the interrupting line is a terminal interupt,
    get the semaphore index */
    if(lineNumber == TERMINT) {
        /* check if the transmission status is recieve command */
        if((devReg->t_transm_status & FULLBYTE) != READY) {
            /* get the index - where NOSEM is the offset of -3 */
            i = DEVPERINT * (lineNumber - NOSEM) + deviceNumber;
            /* mark the flag as false - turn off recieve */
            receive = FALSE;
        } else {
            /* get the index - where NOSEM is the offset of -3 */
            i = DEVPERINT * ((lineNumber - NOSEM) + 1) + deviceNumber;
        }
    } else {
        /* the interrupt is not a terminal interrupt, so 
        simply compute the index */
        i = DEVPERINT * (lineNumber - NOSEM) + deviceNumber;
    }
    int *semaphore = &(semdTable[i]);
    /* perform a V operation on the semaphore */
    (*semaphore)++;
    if((*semaphore) <= 0) {
        /* release synchronization on the process */
        pcb_PTR p = removeBlocked(semaphore);
        if (p != NULL) {
            if(receive && (lineNumber == TERMINT)) {
                p->p_state.s_v0 = devReg->t_recv_status;
                /* the reception has been acknowledged */
                devReg->t_recv_command = ACK;
            } else if(!receive && (lineNumber == TERMINT)) {
                p->p_state.s_v0 = devReg->t_transm_status;
                /* the transmission has been acknowledged */
                devReg->t_transm_command = ACK;
            } else {
                p->p_state.s_v0 = devReg->d_status;
                /* the command has been acknowledged */
                devReg->d_command = ACK;
            }
            /* we have one less process wairing */
            softBlockedCount--;
            /* insert into the ready queue */
            insertProcQ(&(readyQueue), p);
        }
    }
    /* exit the interrupt handler */
    exitInterruptHandler(startTime);
}
