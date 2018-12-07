/* h files to include */
#include "../h/const.h"
#include "../h/types.h"
/* e files to include */
#include "../e/initial.e"
#include "../e/scheduler.e"
#include "../e/interrupts.e"
#include "../e/scheduler.e"
#include "../e/pcb.e"
#include "../e/asl.e"
/* include the µmps2 library */
#include "/usr/local/include/umps2/umps/libumps.e"

void contextSwitch(state_PTR s) {
    LDST(s);
}

void copyState(state_PTR from, state_PTR to) {
    to->s_asid = from->s_asid;
    to->s_cause = from->s_cause;
    to->s_pc = from->s_pc;
    to->s_status = from->s_status;
    int i;  
    for(i = 0; i < STATEREGNUM; i++) {
        to->s_reg[i] = from->s_reg[i];
    }
}


static int findSemaphoreIndex(int lineNumber, int deviceNumber, int flag) {
    int offset;
    if(flag == TRUE) {
        offset = (lineNumber - NOSEM + flag); 
    } else {
        offset = (lineNumber - NOSEM);
    }
    int calculation = (DEVPERINT * offset) + deviceNumber;
    return calculation;
}

static void passUpOrDie(int callNumber, state_PTR old) {
    switch(callNumber) {
    case SYSTRAP:
        if(currentProcess->newSys != NULL) {
            copyState(old, currentProcess->oldSys);
            contextSwitch(currentProcess->newSys);
        }
        break;
    case TLBTRAP:
        if(currentProcess->newTlb != NULL){
            copyState(old, currentProcess->oldTlb);
            contextSwitch(currentProcess->newTlb);
        }
        break;
    case PROGTRAP:
        if(currentProcess->newPgm != NULL){
            copyState(old, currentProcess->oldPgm);
            contextSwitch(currentProcess->newPgm);
            break;
        }
    }
    terminateProcess();
}


 static void terminateProgeny(pcb_PTR p) {
     while (!emptyChild(p)) {
         terminateProgeny(removeChild(p));
     }
     if (p->p_semAdd != NULL) {
         int* semaphore = p->p_semAdd;
         outBlocked(p);
        if(semaphore >= &(semdTable[0]) && semaphore <= &(semdTable[MAXSEMALLOC - 1])) {
            softBlockedCount--;
        } else {
            (*semaphore)++;
        }
     } else if(p == currentProcess){
         outChild(currentProcess);
     } else {
         outProcQ(&(readyQueue), p);
     }
     freePcb(p);
     processCount--;
 }


static void waitForIODevice(state_PTR state) {
    int lineNumber = state->s_a1;
    int deviceNumber = state->s_a2; 
    int terminalReadFlag = (state->s_a3 == TRUE);
    if(lineNumber < DISKINT || lineNumber > TERMINT) {
        terminateProcess();
    }
    int i = findSemaphoreIndex(lineNumber, deviceNumber, terminalReadFlag);
    int* semaphore = &(semdTable[i]);
    (*semaphore)--;
    if((*semaphore) < 0) {
        insertBlocked(semaphore, currentProcess);
        softBlockedCount++;
        copyState(state, &(currentProcess->p_state));
        invokeScheduler();
    }
    contextSwitch(state);
}

/* 
* Function: Wait For Clock
* This instruction performs a P operation on the kernel maintained pseudo-clock timer semaphore. 
* This semaphore is V’ed every 100 milliseconds automatically by the kernel.
*/
static void waitForClock(state_PTR state) {
    /* get the semaphore index of the clock timer */
    int* semaphore = (int*) &(semdTable[(MAXSEMALLOC - 1)]);
    /* perform a passeren operation */
    (*semaphore)--;
    if((*semaphore) < 0) {
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
* Function: Get CPU Time
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
* Function: Specify the Exceptions State Vector
* when this service is requested, will save the contents of a2 and a3 (in the invoking process’es ProcBlk) 
* to facilitate “passing up” handling of the respective exception when (and if) one occurs while this
* process is executing. When an exception occurs for which an Exception State Vector has been 
* specified for, the nucleus stores the processor state at the time of the exception in the area 
* pointed to by the address in a2, and loads the new processor state from the area pointed to by the address given in a3.
* Each process may request a SYS5 service at most once for each of the three exception types.
* An attempt to request a SYS5 service more than once per exceptpion it is construed as an error and treated as a SYS2.
* If an exception occurs while running a process which has not specified an Exception State Vector for that exception type, 
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
static void passeren(state_PTR state)
{
    int *semaphore = (int *)state->s_a1;
    (*(semaphore))--;
    if ((*(semaphore)) < 0)
    {
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

static void userModeHandler(state_PTR state) {
    state_PTR programTrapOldArea = (state_PTR)PRGMTRAPOLDAREA;
    copyState(state, programTrapOldArea);
    unsigned int placeholder = (programTrapOldArea->s_cause) & ~(FULLBYTE);
    (programTrapOldArea->s_cause) = (placeholder | RESERVED);
    programTrapHandler();
}

static void syscallDispatch(int callNumber, state_PTR caller) {
    switch (callNumber) {
        case WAITFORIODEVICE: /* SYSCALL 8 */
            waitForIODevice(caller);
            break;
        case WAITFORCLOCK: /* SYSCALL 7 */
            waitForClock(caller);
            break;
        case GETCPUTIME: /* SYSCALL 6 */
            getCpuTime(caller);
            break;
        case SPECIFYEXCEPTIONSTATEVECTOR: /* SYSCALL 5 */
            specifyExceptionsStateVector(caller);
            break;
        case PASSEREN: /* SYSCALL 4 */
            passeren(caller);
            break;
        case VERHOGEN: /* SYSCALL 3 */
            verhogen(caller);
            break;
        case TERMINATEPROCESS: /* SYSCALL 2 */
            terminateProcess();
            break;
        case CREATEPROCESS: /* SYSCALL 1 */
            createProcess(caller);
            break;
        default:
            passUpOrDie(SYSTRAP, caller);
    }
}

 void programTrapHandler() {
    state_PTR oldState = (state_PTR) PRGMTRAPOLDAREA;
    passUpOrDie(PROGTRAP, oldState);
 }

 void translationLookasideBufferHandler() { 
    state_PTR oldState = (state_PTR) TBLMGMTOLDAREA;
    passUpOrDie(TLBTRAP, oldState);
 }

 /*
 * Function: The Syscall Handler
 * 
 */ 
 void syscallHandler() {
    state_PTR caller = (state_PTR) SYSCALLOLDAREA;
    caller->s_pc = caller->s_pc + 4;
    int userMode = FALSE;    
    unsigned int callNumber = caller->s_a0;
    unsigned int status = caller->s_status;
    if((status & KUp) != ALLOFF) {
        userMode = TRUE;
    }
    if((callNumber < 9) && (callNumber > 0) && userMode) {
        userModeHandler(caller);
    } else {
        syscallDispatch(callNumber, caller);
    }
 }




