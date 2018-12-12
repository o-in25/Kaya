/*************************************************** exceptions.c ********************************************************
	Handles the interrupts that occur in the Kaya OS/ When an interrupt occurs, assuming that the interrupt bit is 
    turned on (otherwise an interrupt cannot occur), the interval handler will be invoked to dermine the cause of 
    the interrupt, as well as the appropriate actions to be taken henceforth. The cause of the interrupt can either 
    be a device that requires to be acknowledged as part of umps2's handshake protocol, or for from a clock interrupt
    caused by either a quantum ending or a psuedo clock timer. For semaphore devices, i.e. a disk, tape, network, printer 
    or terminal device, causes an interupt, a V operation is performed on that device's semaphore and implements the 
    shandshake. Furthermore, for all devices, the interrupt handler will insure that running processes' will not be
    charged for time spent in the the interupt handler. 

    This module contributes function definitions and a few sample fucntion implementations to the contributors put 
    forth by the Kaya OS project.

***************************************************** exceptions.c ******************************************************/

/* h files to include */
#include "../h/const.h"
#include "../h/types.h"
/* e files to include */
#include "../e/initial.e"
#include "../e/scheduler.e"
#include "../e/interrupts.e"
#include "../e/scheduler.e"
#include "../e/exceptions.e"
#include "../e/pcb.e"
#include "../e/asl.e"
/* include the µmps2 library */
#include "/usr/local/include/umps2/umps/libumps.e"

/************************************************************************************************************************/
/******************************************** HELPER FUNCTIONS  *********************************************************/
/************************************************************************************************************************/

/*
* Function: Find Semaphore Index 
* Will compute the index of a the device semaphore given
* the line number and the device number. An additional flag 
* is provided that signifies a terminal read. If so, the device 
* number will be the n+1th device. 
*/
static int findSemaphoreIndex(int lineNumber, int deviceNumber, int flag) {
    int offset;
    /* is it a terminal read? */
    if(flag == TRUE) {
        /* compute the index with the flag offset */
        offset = (lineNumber - NOSEM + flag); 
    } else {
        /* compute the index without the flag offset */
        offset = (lineNumber - NOSEM);
    }
    /* get the index from the offset and the deice number */
    int calculation = (DEVPERINT * offset) + deviceNumber;
    /* found the calculation, now return it */
    return calculation;
}

/*
* Function: Pass Up Or Die 
* The syscall 5 (specify state exceptions vector) helper will
* check if a new exception vector has been set up for that particular
* exception. If so, that processes is "passed up" to the appropriate 
* handler, copys into the processor's state the old exception, and 
* performs a context switch. Otherwise, the process dies
*/
static void passUpOrDie(int callNumber, state_PTR old) {
    /* get the call number */
    switch(callNumber) {
        /* System trap exception */
        case SYSTRAP:
            /* has the systrap handler been set up? */
            if(currentProcess->newSys == NULL) {
                /* no - it dies */
                terminateProcess();
            } else {
                /* pass it up to the appropriate handler */
                copyState(old, currentProcess->oldSys);
                /* context switch */
                contextSwitch(currentProcess->newSys);
            }
            break;
        /* Translation Lookaside Buffer Exception */
        case TLBTRAP:
            if(currentProcess->newTlb == NULL) {
                /* no - it dies */
                terminateProcess();
            } else {
                /* pass it up to the appropriate handler */
                copyState(old, currentProcess->oldTlb);
                /* context switch */
                contextSwitch(currentProcess->newTlb);
            }
            break;
        /* Program Trap Exception */
        case PROGTRAP:
            if(currentProcess->newPgm == NULL) { 
                /* no - it dies */
                terminateProcess();
            } else {
                /* pass it up to the appropriate handler */
                copyState(old, currentProcess->oldPgm);
                /* context switch */
                contextSwitch(currentProcess->newPgm);
            }
            break;
        }
}

/* 
* Function: Terminate Progeny
* The syscall 2 - terminate process - helper function.
* Will beform tail recursion on the current process, killing 
* all of it's progeny until there are none remaining. Then, it wil
* check if the process is in a semaphore, is the curren process,
* or is in the ready queue. 
*/
static void terminateProgeny(pcb_PTR p) {
    /* kill each progeny */
     while(!emptyChild(p)) {
        /* perform tail recursion */
        terminateProgeny(removeChild(p));
     }
     /* n-1 processes left */
     processCount--;
     /* check of the pcb_t has a semaphore address */
     if (p->p_semAdd != NULL) {
        /* get the semaphore */
        int* semaphore = p->p_semAdd;
        /* call outblocked on the pcb_t */
        outBlocked(p);
        /* if the semaphore greater than 0 and less than 48, then
        it is a device semapore */
        if(semaphore >= &(semdTable[0]) && semaphore <= &(semdTable[CLOCK])) {
            /* we have 1 less waiting process */
            softBlockedCount--;
        } else {
            /* not a device semaphore */
            (*semaphore)++;
        }
     } else if(p == currentProcess){
         /* yank the process from the parent */
         outChild(currentProcess);
     } else {
         /* yank the process from the ready queue */
         outProcQ(&(readyQueue), p);
     }
     /* there are no mo children, so the process itself is free */
     freePcb(p);
 }

/*
* This service performs a P operation on the semaphore that the kernel 
* maintains for the I/O device indicated by the values in a1, a2, and optionally a3.
* Each terminal device has two kernel maintained semaphores for it; one for character 
* receipt and one for character transmission.
* The kernel performs a V operation on the kernel maintained semaphore whenever 
* that (sub)device generates an interrupt.
* Once the process resumes after the occurrence of the anticipated interrupt, 
* the (sub)device’s status word is returned in v0. For character transmission and receipt,
* the status word, in addition to containing a device completion code, will also contain the character
*  transmitted or received. It is possible that the interrupt can occur prior to the request 
* for the SYS8 service. In this case the requesting process will not block as a result of 
* the P operation and the interrupting device’s status word, which was stored off,
* is placed in v0 prior to resuming execution
*/
static void waitForIODevice(state_PTR state) {
    /* get the line number in the a1 register */
    int lineNumber = state->s_a1;
    /* get the device number in the a2 register */
    int deviceNumber = state->s_a2; 
    /* set the terminal read/write flag to be the contents of a3 */
    int terminalReadFlag = (state->s_a3 == TRUE);
    /* if the requesting device is not in the 3-8 device range,
    the process is simply terminated */
    if(lineNumber < DISKINT || lineNumber > TERMINT) {
        /* kill the process */
        terminateProcess();
    }
    /* get the index of the device semaphore */
    int i = findSemaphoreIndex(lineNumber, deviceNumber, terminalReadFlag);
    int* semaphore = &(semdTable[i]);
    /* perform a P operation */
    (*semaphore)--;
    if((*semaphore) < 0) {
        /* block the current process */
        insertBlocked(semaphore, currentProcess);
        /* we have 1 more waiting process */
        softBlockedCount++;
        /* copy the old syscall area to the new pcb_t state_t */
        copyState(state, &(currentProcess->p_state));
        /* get a new process */
        invokeScheduler();
    }
    /* if no P operation can be done, simply context switch */
    contextSwitch(state);
}

/* 
* Function: Wait For Clock - Syscall 7
* This instruction performs a P operation on the kernel maintained pseudo-clock timer semaphore. 
* This semaphore is V’ed every 100 milliseconds automatically by the kernel.
*/
 static void waitForClock(state_PTR state) {
     /* get the semaphore index of the clock timer */
     int *semaphore = (int*) &(semdTable[CLOCK]);
     /* perform a passeren operation */
     (*semaphore)--;
     if ((*semaphore) < 0)
     {
         /* block the process */
         insertBlocked(semaphore, currentProcess);
         /* copy from the old syscall area into the new pcb_state */
         copyState(state, &(currentProcess->p_state));
         /* increment the number of waiting processes */
         softBlockedCount++;
     }
     invokeScheduler();
}

/* 
* Function: Get CPU Time - Syscall 6
* When this service is requested, it causes the processor time (in microseconds) used by the 
* requesting process to be placed/returned in the caller’s v0. The kernel records (in the ProcBlk) the amount
* of processor time used by each process.
*/
 static void getCpuTime(state_PTR state) {
        /* copy the state from the old syscall into the pcb_t's state */
        copyState(state, &(currentProcess->p_state));
        /* the clock can be started by placing a new value in the 
        STCK ROM function */
        cpu_t stopTOD;
        /* start the clock  for the stop */ 
        STCK(stopTOD);
        /* get the time that has passed */
        cpu_t elapsedTime = stopTOD - startTOD;
        currentProcess->p_time = (currentProcess->p_time) + elapsedTime;
        /* store the state in the pcb_t's v0 register */
        currentProcess->p_state.s_v0 = currentProcess->p_time;
        /* start the clock for the start TOD */
        STCK(startTOD);
        contextSwitch(&(currentProcess->p_state));
}

/*
* Function: Specify the Exceptions State Vector - Syscall 5
* when this service is requested, will save the contents of a2 and a3 
* (in the invoking process’es ProcBlk) 
* to facilitate “passing up” handling of the respective exception 
* when (and if) one occurs while this
* process is executing. When an exception occurs for which an
8  Exception State Vector has been 
* specified for, the nucleus stores the processor state at the time of 
* the exception in the area 
* pointed to by the address in a2, and loads the new processor state from 
* the area pointed to by the address given in a3.
* Each process may request a SYS5 service at most once for each of the 
* three exception types.
* An attempt to request a SYS5 service more than once per exceptpion 
* it is construed as an error and treated as a SYS2.
* If an exception occurs while running a process which has not specified an 
* Exception State Vector for that exception type, 
* then the nucleus should treat the exception as a SYS2 as well.
*/
static void specifyExceptionsStateVector(state_PTR state) {
    /* get the exception from the a1 register */
    switch(state->s_a1) {
        /* check if the specified exception is a translation 
        look aside buffer exception */
        case TLBTRAP:
            /* if the new tlb has already been set up,
            kill the process */
            if(currentProcess->newTlb != NULL) {
                terminateProcess();
            }
            /* store the syscall area state in the new tlb */
            currentProcess->newTlb = (state_PTR) state->s_a3;
            /* store the syscall area state in the old tlb*/
            currentProcess->oldTlb = (state_PTR) state->s_a2;
            break;
        case PROGTRAP:
            /* if the new pgm has already been set up,
            kill the process */
            if(currentProcess->newPgm != NULL) {
                terminateProcess();
            }
            /* store the syscall area state in the new pgm */
            currentProcess->newPgm = (state_PTR) state->s_a3;
            currentProcess->oldPgm = (state_PTR) state->s_a2;
            break;
        case SYSTRAP:
            /* if the new systrap has already been set up,
            kill the process */
            if(currentProcess->newSys != NULL) {
                terminateProcess();
            }
            /* store the syscall area state in the new pgm */
            currentProcess->newSys = (state_PTR) state->s_a3;
            /* store the syscall area state in the old pgm*/
            currentProcess->oldSys = (state_PTR) state->s_a2;
            break;
    }
    contextSwitch(state);
}

/*
* Function: Verhogen - Syscall 4
* When this service is requested, it is interpreted by the kernel as a 
* request to perform a V operation on a syncronization semaphore. The SYS3 service 
* is requested by the calling process by placing the value 3 in a0, 
* the physical address of the semaphore to be V’ed in a1, and then 
* executing a SYSCALL instruction.
*/
static void passeren(state_PTR state) {
    /* get the semaphore in the s_a1 */
    int *semaphore = (int*) state->s_a1;
    /* decrement teh semaphore */
    (*(semaphore))--;
    if ((*(semaphore)) < 0) {
        cpu_t stopTOD;
        STCK(stopTOD);
        /*Store elapsed time*/
        int elapsedTime = stopTOD - startTOD;
        /* add the elapsed time to the current process */
        currentProcess->p_time = currentProcess->p_time + elapsedTime;
        /* copy from the old syscall area to the new process's state */
        copyState(state, &(currentProcess->p_state));
        /* the process now must wait */
        insertBlocked(semaphore, currentProcess);
        /* get a new job */
        invokeScheduler();
    }
    /* if the semaphore is not less than zero, do not 
    block the process, just load the new state */
    contextSwitch(state);
}

/*
* Function: Verhogen - Syscall 3
* When this service is requested, it is interpreted by the kernel as a 
* request to perform a V operation on a syncronization semaphore. The SYS3 service 
* is requested by the calling process by placing the value 3 in a0, 
* the physical address of the semaphore to be V’ed in a1, and then 
* executing a SYSCALL instruction.
*/
static void verhogen(state_PTR state) {
    /* the semaphore is placed in the a1 register of the 
    passed in state_t */
    int* semaphore = (int*) state->s_a1;
    /* increment the semaphore - the V operation on 
    the semaphore */
    (*(semaphore))++;
    /* if the synchronization semaphore description is <= 0, 
    then it will remove the process from the blocked processes 
    and place it in the ready queue - which synchronizes the processes */
    if(*(semaphore) <= 0) {
        /* unblock the next process */
        pcb_PTR newProcess = removeBlocked(semaphore);
        /* the current process is then placed in the ready 
        queue - baring its not null */
        if(newProcess != NULL) {
            /* place it in the ready queue */
            insertProcQ(&(readyQueue), newProcess);
        }
    }
    /* perform a context switch on the requested process */
    contextSwitch(state);
}

/* Function: Terminate Process - Syscall 2
* This services causes the executing process to cease to exist.
*  In addition, recurively, all progeny of this process
*  are terminated as well. Execution of this intruction does not 
* complete until all progeny are terminated.
*/
static void terminateProcess() {
    /* if there are no children, simply decrement 
    the process count, remove the current process, and 
    free up a pcb_t */
    if(emptyChild(currentProcess)) {
        /* n-1 processes remaining */
        processCount--;
        outChild(currentProcess);
        /* free the process */
        freePcb(currentProcess);
    } else {
        /* calls the terminate progeny helper function */
        terminateProgeny(currentProcess);
    }
    /* in either case, we have no current processes, and 
    the scheduler needs to be called in order to get a new
    processes */
    currentProcess = NULL;
    /* reschedule a new process */
    invokeScheduler();
}

/*
* Function: Create Process - Syscall 1
* When requested, this service causes a new process, 
* said to be a progeny of the caller, to be created. 
* a1 should contain the physical address of a processor state 
* area at the time this instruction is executed. 
* This processor state is used as the initial state for 
* the newly created process. The process requesting 
* the SYS1 service continues to exist and to execute. 
* If the new process cannot be created due to lack of 
* resources (for example no more free ProcBlk’s), an error code 
* of -1 is placed in the caller’s v0, otherwise, the value 0 is 
* placed the caller’s v0.
*/
static void createProcess(state_PTR state) {
    /* grab a new process */
    pcb_PTR p = allocPcb();
    if(p != NULL) {
        /* there is now n+1 running processes */
        processCount++;
        /* since there is a free process, if the process 
        has a parent, it is inserted into the parent, and then
        placed in the ready queue */
        insertChild(currentProcess, p);
        insertProcQ(&(readyQueue), p);
        /* copy the content from the state's 
        $a1 register to the new pcb_t's state */
        state_PTR temp = (state_PTR) state->s_a1;
        copyState(temp, &(p->p_state));
        /* acknowledge the success of the new process
        by placing 0 in the state's $v0 register */ 
        state->s_v0 = SUCCESS;
    } else {
        /* if there are no free processes, acknowledge 
        the failure of a new allocated pcb_t by placing 
        -1 in the state's $v0 register */
        state->s_v0 = FAILURE;
    }
    /* context switch */
    contextSwitch(state);
}

/*
* Function: User Mode Handler 
* Gets called when the system is in user mode and 
* attempts to make a syscall request 1-8. Here, the syscall 
* old area is copied into the program trap old area, the
* program state_t's cause register will contain the RESERVED
* mask, and will then enter a program trap 
*/
static void userModeHandler(state_PTR state) {
    /* get the old program trap area */
    state_PTR programTrapOldArea = (state_PTR) PRGMTRAPOLDAREA;
    /* copy the old syscall area into the old program trap area */
    copyState(state, programTrapOldArea);
    /* set teh cause register to contain the RESERVED MASK */
    unsigned int placeholder = (programTrapOldArea->s_cause) & ~(FULLBYTE);
    (programTrapOldArea->s_cause) = (placeholder | RESERVED);
    /* enter a program trap */
    programTrapHandler();
}

/*
* Function: Syscall Dispatch
* Takes a state_t pointer, in this case the old syscall area, and 
* a call number and dispatches the appropriate syscall. 
*/
static void syscallDispatch(int callNumber, state_PTR caller) {
    switch (callNumber) {
        /* SYSCALL 8 */
        case WAITFORIODEVICE:
            waitForIODevice(caller);
            break;
        /* SYSCALL 7 */
        case WAITFORCLOCK: 
            waitForClock(caller);
            break;
        /* SYSCALL 6 */
        case GETCPUTIME: 
            getCpuTime(caller);
            break;
        /* SYSCALL 5 */
        case SPECIFYEXCEPTIONSTATEVECTOR: 
            specifyExceptionsStateVector(caller);
            break;
        /* SYSCALL 4 */
        case PASSEREN:
            passeren(caller);
            break;
        /* SYSCALL 3 */
        case VERHOGEN:
            verhogen(caller);
            break;
        /* SYSCALL 2 */
        case TERMINATEPROCESS:
            terminateProcess();
            break;
        /* SYSCALL 1 */
        case CREATEPROCESS:
            createProcess(caller);
            break;
        /* no valid syscall - set a program trap */
        default:
            passUpOrDie(SYSTRAP, caller); 
    }
}

/************************************************************************************************************************/
/********************************************** SYSCALL/BREAKPOINT*******************************************************/
/************************************************************************************************************************/

/* 
* Function: Context Switch 
* A simple wrapper function that will place the 
* passed in state_t pointer into the ROM-issued 
* Load State (LDST) function
*/
void contextSwitch(state_PTR s) {
    /* load the new processor state */
    LDST(s);
}

/* 
* Function: Copy State
* Simple helper function that will take the contents
* of the state_t pointer from the first argument 
* and will copy it to the second state_t pointer 
* argument
*/
void copyState(state_PTR from, state_PTR to) {
    /* copy id */
    to->s_asid = from->s_asid;
    /* copy cause register */
    to->s_cause = from->s_cause;
    /* copy program counter */
    to->s_pc = from->s_pc;
    /* copy status register */
    to->s_status = from->s_status;
    int i;
    /* copy each register */
    for (i = 0; i < STATEREGNUM; i++) {
        /* copy the register */
        to->s_reg[i] = from->s_reg[i];
    }
}
/*
* Function: Program Trap Handler 
* Gets the old program trap area and memory and 
* passes the exception number and the old program
* trap area to pass up or die. There, pass up or
* die will determine if an exception state vector has been
* set up for that process 
*/
 void programTrapHandler() {
    /* get the area in memory */
    state_PTR oldState = (state_PTR) PRGMTRAPOLDAREA;
    /* pass up the process to its appropriate handler
    or kill it */
    passUpOrDie(PROGTRAP, oldState);
 }

 /*
* Function: Translation Lookaside Buffer (TLB)  Handler 
* Gets the old tlb trap area and memory and 
* passes the exception number and the old program
* trap area to pass up or die. There, pass up or
* die will determine if an exception state vector has been
* set up for that process 
*/
 void translationLookasideBufferHandler() {
    /* get the area in memory */
    state_PTR oldState = (state_PTR)TBLMGMTOLDAREA;
    /* pass up the process to its appropriate handler
    or kill it */
    passUpOrDie(TLBTRAP, oldState);
 }

 /*
 * Function: The Syscall Handler
 * The handler for syscalls 1-8 when the user is
 * in kernel mode. If the user is not in kernel mode, the
 * program is set to be a reserved instruction and will 
 * pass the responsibility to the user mode handler. 
 * In the case that a syscall >9 is issued in kernel,
 * mode, a program trap is set up
 */ 
 void syscallHandler() {
    /* get the old syscall area in memory */
    state_PTR caller = (state_PTR) SYSCALLOLDAREA;
    /* increment the program counter by 1 word */
    caller->s_pc = caller->s_pc + 4;
    /* assume the system is in kernel mode */
    int userMode = FALSE;    
    /* get the call number */
    unsigned int callNumber = caller->s_a0;
    /* get the status regitser */
    unsigned int status = caller->s_status;
    /* check if the system is in user or kernel mode */
    if((status & KUp) != ALLOFF) {
        /* in user mode */
        userMode = TRUE;
    }
    /* if the system is in user mode and makes a syscall 1-8 
    request, it is then passed down to the user mode handler, 
    where the cause register will be set to the reserved address; 
    otherwise, if we are in kernel mode but a syscall >8 is made,
    the syscall dispatch will account for this */
    if((callNumber < 9) && (callNumber > 0) && userMode) {
        /* pass responsibility to the user mode handler */
        userModeHandler(caller);
    } else {
        /* make the syscall */
        syscallDispatch(callNumber, caller);
    }
 }




