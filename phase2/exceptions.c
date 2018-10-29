#include "../h/const.h"
#include "../h/types.h"
/* e files to include */
#include "../e/initial.e"
#include "../e/scheduler.e"
#include "../e/pcb.e"
#include "../e/asl.e"



/************************************************************************************************************************/
/******************************************** HELPER FUNCTIONS  *********************************************************/
/************************************************************************************************************************/
/* Function: a helper function to kill a process' child.
* It deals with each individual pcb. here, there are three cases
* that exists. first, the process is the current process. In which case,
* this is the root child - so simply call outChild(). Second, the process
* is on the ready queue. Here, if the process isn't the current process 
* and it's semaphore address is 0, it is on the ready queue and thus 
* outProcQ() is called. Third, there's only so many places the current 
* process can be - and we have already accounted for most of them. Now,
* the process must be on the active semaphre list. Therefore, in this case,
* call outBlocked() to remove it from teh semaphore. But here, there are two 
* subcases. First subcase, the device is a device semaphore. Here, softblocked
* count is decreased by 1. Second subcase,
* the easy case, it is not a device semaphore. Therefore, the semaphore address 
* is incremented by 1 */
static void terminateProgeny(pcb_PTR p) {
    /* first, kill all of the parents children - time to get violent */
    while(!emptyChild(p)) {
        /* perform head recursion on all of the 
        process's children */
        terminateProgeny(removeChild(p));
    }
    int* semaphore = p->p_semAdd;
    /* if the semaphore is not null, that means that the process on the ASL 
    and is blocked */
    if(semaphore != NULL) {
        /* here, if the process is not null, then we need to do all of the work.
        Beause these steps are mutex with the I/O interrupt handler, if the process
        is not null, we do the following. If it IS null, the I/O interrupt handler already
        took care of this for us */
        outBlocked(semaphore);
        if(semaphore >= semdTable[0]) {
            softBlockedCount--;
        } else {
            (*semaphore)++;
        }
    } else {
        /* here, the semaphore is null, meaning that the I/O handler already took care of 
        decrementing the softblocked count, incrementing the semaphore and calling outblocked. 
        now, we handle the case of if the process is the current process or if the process
        is on the ready queue */
        if(p == currentProcess) {
            /* yank the child from its parent */
            outChild(currentProcess);
        } else {
            /* yank it from the ready queue */
            outProcQ(&(readyQueue), p);
        }
    }
    /* free the process block and decrement the process count regardless of what 
    case it is */
    freePcb(p);
    processCount--;
}
/* Function: context switch 
* ROM instruction that will change the state of the 
* processor */
static void contextSwitch(state_PTR s) {
    LDST(s);
}

/* Fucntion: Pass up or die 
* the syscall 5 helper mechanism. Whenever an exception
* or interrupts occur, the current processor state 
* is loaded. The sys5 passup or die provides this functionality 
* and a new processor state is loaded. It allows the caller to store
* the address of two processor states */
static void passUpOrDie(state_PTR old, int callNumber) {
    /* has a sys5 for that trap type been called?
    if not, terminate the process and all its progeny */
    switch(callNumber) {
        /* if yes, copy the state the caused the exception to 
        the location secified in the pcb. context switch */
        case SYSTRAP:
            copyState(old, currentProcess->oldSys);
            contextSwitch(currentProcess->newSys);
            break;
        case TLBTRAP:
            copyState(old, currentProcess->oldTlb);
            contextSwitch(currentProcess->newTlb);
            break;
        case PROGTRAP:
            copyState(old, currentProcess->oldPgm);
            contextSwitch(currentProcess->newPgm);
            break;
    }
    terminateProcess();
}

/* Function: delegate syscall
* Issues a switch statement to determine which 
* syscall occurs 
*/
static void delegateSyscall(int callNumber, pcb_PTR caller) {
     switch(callNumber) {
            case WAITFORIODEVICE: /* SYSCALL 8 */
                waitForClock();
                break;
            case WAITFORCLOCK: /* SYSCALL 7 */
                waitForClock();
                break;
            case GETCPUTIME: /* SYSCALL 6 */
                getCpuTime();
                break;
            case SPECIFYEXCEPTIONSTATEVECTOR: /* SYSCALL 5 */
                specifyExceptionsStateVector();
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
                passUpOrDie(caller, callNumber);
                break;
        }
}

/* Function copy state 
* copies the state of the processor from one new area
*to another 
*/
copyState(state_PTR old, state_PTR new) {
    new->s_asid = old->s_asid;
    new->s_cause = old->s_cause;
    new->s_pc = old->s_pc;
    new->s_status = old->s_status;
    int i;
    for(i = 0; i < STATEREGNUM; i++) {
        new->s_reg[i] = old->s_reg[i];
    }
}

/************************************************************************************************************************/
/********************************************* SYSTEM CALLS *************************************************************/
/************************************************************************************************************************/


static void waitForDevice() {

}

static void waitForClock() {

}

static void getCpuTime() {

}

static void specifyExceptionsStateVector() {

}

/* Function: Syscall 4 - Passeren
* When this service is requested, it is interpreted by the nucleus as a request to 
* perform a V operation on a semaphore.
* The V or SYS3 service is requested by the calling process by 
* placing the value 3 in a0, the physical address of the semaphore 
* to be V’ed in a1, and then executing a SYSCALL instruction.
*/
static void passeren(state_PTR state) {
    /* place the value of the physical address of the
    semaphore to be passerened into register a1 */
    int* semaphore = (int*) state->s_a1;
    /* decrement the semaphore */
    (*(semaphore))--;
    if(*(semaphore) < 0) {
        insertBlocked(semaphore, currentProcess);
        invokeScheduler();
    }
    contextSwitch(state);
}

/* Function: Syscall 3 - Verhogen 
* When this service is requested, it is interpreted by the nucleus as 
* a request to perform a V operation on a semaphore.
* The V or SYS3 service is requested by the calling process by 
* placing the value 3 in a0, the physical address of the semaphore to be V’ed in a1, 
* and then executing a SYSCALL instruction.
*/
static void verhogen(state_PTR state) {
    int* semaphore = (int*) state->s_a1;
    (*(semaphore))++;
    if(*(semaphore) <= 0) {
        pcb_PTR newProcess = NULL;
        newProcess = removeBlocked(semaphore);
        if(newProcess != NULL) {
            insertProcQ(&(readyQueue), newProcess);
        }
    }
}

/* Function: Syscall 2 - Terminate Process 
*This services causes the executing process to cease to exist. In addition, 
* recursively, all progeny of this process are terminated as well. This is done through 
* the use of a helper function. Execution of this instruction does not 
* complete until all progeny are terminated. The SYS2 service is requested by the 
* calling process by placing the value 2 in a0 and then executing a SYSCALL instructio
*/
static void terminateProcess() {
    /* invoke the helper function */
    terminateProgeny(currentProcess);
    /* The current process is over */
    currentProcess = NULL;
    /* resechdule */
    invokeScheduler();
}

/* Function: Syscall 1 - Create Process 
* When requested, this service causes a new process, said to be a progeny of the caller, 
* to be created. a1 should contain the physical address of a processor state area at the time this 
* instruction is executed. This processor state should be used as the initial state for the newly created process. 
* The process requesting the SYS1 service continues to exist and to execute. 
* If the new process cannot be created due to lack of resources (for example no more free ProcBlk’s), an error 
* code of -1 is placed/returned in the caller’s v0, otherwise, return the value 0 in the caller’s v0.
* The SYS1 service is requested by the calling process by placing the value 1 in a0, 
* the physical address of a processor state in a1, and then executing a SYSCALL instruction.
*/
static void createProcess(state_PTR caller) {
    /* create the new process */
    pcb_PTR p = allocPcb();
    if(p == NULL) {
        /* we don't have enough resources to start this process, 
        for whatever reason that may be - most likley no more free 
        process blocks - so add -1 in the state's vo register */
        caller->s_v0 = -1;
    } else {
        /* we hace a sucessful running process, so v0 is now 1 */
        caller->s_v0 = 0;
        /* we have a new process, so add it to the count */
        processCount++;
        /* add the new process to the current process's child - how cute */
        insertChild(currentProcess, p);
        /* TODO: copy state */
    }
    /* context switch */
    LDST(caller);
}


/************************************************************************************************************************/
/*************************************** EXCEPTION HANDLERS *************************************************************/
/************************************************************************************************************************/
 void syscallHandler() {
    /* get the address of the old syscall area, since we
    wake up in the syscall handler */
    state_PTR caller = (state_PTR) SYSCALLOLDAREA;
    /* increment program count */
    caller->s_pc = caller->s_pc + 4;
    /* in order to execute syscals 1-9, we
    must be in kernel mode */
    int kernelMode = FALSE;    
    /* since the value of the syscall is placed in the a0 register
    we read the a0 register to see wht value it is. The system supports up
    to 255 syscalls */
    const int callNumber = caller->s_a0;
    const unsigned int status = caller->s_status;
    if((status & KERNELMODEON) == ALLOFF) {
        /* in kernel mode */
        kernelMode = TRUE;
    }
    if(kernelMode) {
        /* call our helper function to assist with handling the syscalls IF we are
        in kernel mode */
        delegateSyscall(callNumber, callNumber);
    } else if(!kernelMode && callNumber < 9) {
        state_PTR programTrapOldArea = (state_PTR) PRGMTRAPOLDAREA;
        programTrapOldArea->s_cause = RESERVED;
        copyState(caller, programTrapOldArea);
        programTrapHandler();
    } else {
        passUpOrDie(caller, callNumber);
    }
 }

 void programTrapHandler() {
     state_PTR oldState = (state_PTR) PRGMTRAPOLDAREA;
     passUpOrDie(PROGTRAP, oldState);
     /* TODO program handler */
 }

 void tableHandler() {
     /* TODO table handler */  

 }


 