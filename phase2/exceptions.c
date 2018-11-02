#include "../h/const.h"
#include "../h/types.h"
/* e files to include */
#include "../e/initial.e"
#include "../e/scheduler.e"
#include "../e/interrupts.e"
#include "../e/pcb.e"
#include "../e/asl.e"
#include "/usr/local/include/umps2/umps/libumps.e"

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
void contextSwitch(state_PTR s) {
    debugB(99);
    LDST(s);
    debugB(1009);
}

/* Fucntion: Pass up or die 
* the syscall 5 helper mechanism. Whenever an exception
* or interrupts occur, the current processor state 
* is loaded. The sys5 passup or die provides this functionality 
* and a new processor state is loaded. It allows the caller to store
* the address of two processor states */
static void passUpOrDie(int callNumber, state_PTR old) {
    /* has a sys5 for that trap type been called?
    if not, terminate the process and all its progeny */
    switch(callNumber) {
        /* if yes, copy the state the caused the exception to 
        the location secified in the pcb. context switch */
        case SYSTRAP:
            if(currentProcess->newSys != NULL) {
                copyState(old, currentProcess->oldSys);
                contextSwitch(currentProcess->newSys);
            } else {
                terminateProcess();
            }
            break;
        case TLBTRAP:
            if(currentProcess->newTlb != NULL) {
                copyState(old, currentProcess->oldTlb);
                contextSwitch(currentProcess->newTlb);
            } else {
                terminateProcess();
            }
            break;
        case PROGTRAP:
            if(currentProcess->newPgm != NULL) {
                copyState(old, currentProcess->oldPgm);
                contextSwitch(currentProcess->newPgm);
            } else {
                terminateProcess();
            }
            break;
    }
}

/* Function: delegate syscall
* Issues a switch statement to determine which 
* syscall occurs 
*/
static void delegateSyscall(int callNumber, state_PTR caller) {
     switch(callNumber) {
            case WAITFORIODEVICE: /* SYSCALL 8 */
                waitForClock(caller);
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
                passUpOrDie(caller, callNumber);
                break;
        }
}

/* Function: Copy state 
* copies the state of the processor from one new area
*to another 
*/
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

/*
* Function: Find semaphore index
* Finds the index of the semaphore. It is important to know that
* Note that terminal devices are two independent sub-devices and are handled by 
* the SYS8 service as two independent devices. Hence each terminal device 
* has two nucleus maintained semaphores for it; one for character 
* receipt and one for character transmission.
*/
static int findSemaphoreIndex(int lineNumber, int deviceNumber, int flag) {
     int offset;
    if(flag == TRUE) {
        offset = (lineNumber - NOSEM + flag); 
    } else {
        offset = lineNumber - offset;
    }
    int calculation = DEVPERINT * NOSEM + deviceNumber;
}

/************************************************************************************************************************/
/********************************************* SYSTEM CALLS *************************************************************/
/************************************************************************************************************************/

/*********************************************** SYS 8 **************************************************/
/*
* Function: Syscall 8 - Wait for IO Device
*/
static void waitForIODevice(state_PTR state) {
    /* compute the index of the appropriate semaphore */
    int lineNumber = state->s_a1;
    int deviceNumber = state->s_a2; 
    /* is the command read or write? */
    int terminalReadFlag = (state->s_a3 == TRUE);
    if(lineNumber < DISKINT || lineNumber > TERMINT) {
        /* kill the process */
        terminateProcess();
    } 
    /* each i/o device has a globa phase 2 semaphore associated 
    with it. Here, we compute the index */
    int i = findSemaphoreIndex(lineNumber, deviceNumber, terminalReadFlag);
    /* we found the semaphore */
    int* semaphore = &(semdTable[i]);
    /* P operation */
    (*semaphore)--;
    if((*semaphore) < 0) {
        copyState(semaphore, &(currentProcess->p_state));
        insertBlocked(semaphore, currentProcess);
        softBlockedCount++;
        invokeScheduler();
    } 
    /* context switch */
    contextSwitch(state);
}

/*********************************************** SYS 7 **************************************************/
/*
* Function: Syscall 7 - Wait for clock
*/
static void waitForClock(state_PTR state) {
    int* semaphore = &(semdTable[48]);
    (*semaphore)--;
    softBlockedCount++;
    copyState(state, &(currentProcess->p_state));
    insertBlocked(semaphore, currentProcess);

    invokeScheduler();
}

/*********************************************** SYS 6 **************************************************/
/*
* Function: Syscall 6 - Get CPU time 
* When requested, sys6 will cuse the processor local 
* time to be placed in the caller's v0 register
*/
static void getCpuTime(state_PTR state) {
    /* when a process' turn with the cpu is over,
    the value of teh clock is stored again and is 
    added to the elapsed cpu */
    cpu_t stopTOD;
    STCK(stopTOD);
    /* the elasped time */
    cpu_t elapsedTime = stopTOD - startTOD;
    /* store the time in the pcb_t */
    currentProcess->p_time = currentProcess->p_time = elapsedTime;
    /* store the processor time in the caller's v0 */
    state->s_v0 = currentProcess->p_time;
    /* context switch */
    contextSwitch(state);
}

/*********************************************** SYS 5 **************************************************/
/*
* Function: Syscall 5 - Specify Exception State Vector
* When this service is requested, three pieces of information need
* to be supplied to the nucleus: the type of exception the 
* ESV will be established for. The address into which the 
* old processor state is to be stored while running the current process
* and the new processor state into which to be taken. Specify ESV
* will save the contents of a2 and a3 to facilitate the pass up 
* or die function if and when an exception occurs. Each process 
* may request the sys5 sepcify ESV exactly once at most, and 
* is treated like a sys2 terminate process if this is not the case 
*/
static void specifyExceptionsStateVector(state_PTR state) {
    const unsigned int callNumber = state->s_a1;
    switch(callNumber) {
        case TLBTRAP:
            if(currentProcess->newTlb != NULL) {
                /* the area has already been specified, so treat
                this operation like a sys2 i.e. terminate the process */
                terminateProcess();
            }
            /* store the new area in the a3 register */
            currentProcess->newTlb = (state_PTR) state->s_a3;
            /* store the old area in the a2 register */
            currentProcess->oldPgm = (state_PTR) state->s_a2;
        case PROGTRAP:
            if(currentProcess->newPgm != NULL) {
                /* the area has already been specified, so treat
                this operation like a sys2 i.e. terminate the process */
                terminateProcess();
            }
            /* store the new area in the a3 register */
            currentProcess->newPgm = (state_PTR) state->s_a3;
            /* store the old area in the a2 register */
            currentProcess->oldPgm = (state_PTR) state->s_a2;
        case SYSTRAP:
          if(currentProcess->newSys != NULL) {
                /* the area has already been specified, so treat
                this operation like a sys2 i.e. terminate the process */
                terminateProcess();
            }
            /* store the new area in the a3 register */
            currentProcess->newSys = (state_PTR) state->s_a3;
            /* store the old area in the a2 register */
            currentProcess->oldSys = (state_PTR) state->s_a2;
        default:
            /* should never happen */
            terminateProcess();
    }
    /* context switch */
    contextSwitch(state);
}

/*********************************************** SYS 4 **************************************************/
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
    /* decrement the semaphore - per the protocol of a p oeration */
    (*(semaphore))--;
    if(*(semaphore) < 0) {
        /* wait for the operation */
        insertBlocked(semaphore, currentProcess);
        /* copy the current processor state to the
        new processor state pointed to by the current process'
        p_state field */
        copyState(state, &(currentProcess->p_state));
        /* reschedule */
        invokeScheduler();
    }
    /* context switch */
    contextSwitch(state);
}

/*********************************************** SYS 3 **************************************************/
/* Function: Syscall 3 - Verhogen 
* When this service is requested, it is interpreted by the nucleus as 
* a request to perform a V operation on a semaphore.
* The V or SYS3 service is requested by the calling process by 
* placing the value 3 in a0, the physical address of the semaphore to be V’ed in a1, 
* and then executing a SYSCALL instruction.
*/
static void verhogen(state_PTR state) {
    /* place the value of the physical address of the
    semaphore to be verhogened into register a1 */
    int* semaphore = (int*) state->s_a1;
    /* increment the semaphore - per the protocol of a v oeration */
    (*(semaphore))++;
    if(*(semaphore) <= 0) {
        /* signal that the operation is finished */
        pcb_PTR newProcess = removeBlocked(semaphore);
        if(newProcess != NULL) {
            /* if the ASL is not empty */
            insertProcQ(&(readyQueue), newProcess);
        }
    }
    /* context switch */
    contextSwitch(state);
}

/*********************************************** SYS 2 **************************************************/
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
    /* no context switch, invoke the scheduler */
}
/*********************************************** SYS 1 **************************************************/
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
static void createProcess(state_PTR state) {
    /* create the new process */
    pcb_PTR p = allocPcb();
    if(p == NULL) {
        /* we don't have enough resources to start this process, 
        for whatever reason that may be - most likley no more free 
        process blocks - so add -1 in the state's v0 register */
        state->s_v0 = -1;
        /* context switch */
    } else {
        /* we hace a sucessful running process, so v0 is now 1 */
        state->s_v0 = 0;
        /* we have a new process, so add it to the count */
        processCount++;
        /* add the new process to the current process's child - how cute */
        insertChild(currentProcess, p);
        /* insert the process into the ready queue */
        insertProcQ(&(readyQueue), p);
        /* a1 register contains the physical address of a processor state 
        area at the time this instruction is executed */
        state_PTR temp = (state_PTR) state->s_a1;
        /* processor state, stored as a temporary variable as temp
        is used as the initial state for the newly created process */
        copyState(temp, &(p->p_state));
    }
    /* context switch */
    contextSwitch(state);
}

/************************************************************************************************************************/
/*************************************** EXCEPTION HANDLERS *************************************************************/
/************************************************************************************************************************/
 /*
 * Function: The Syscall Handler
 * 
 */ 
 void syscallHandler() {
     debugC(6969420);
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
    if((status & KERNELMODEON) != ALLOFF) {
        /* in kernel mode */
        kernelMode = TRUE;
    }
    if(kernelMode) {
        /* call our helper function to assist with handling the syscalls IF we are
        in kernel mode */
        delegateSyscall(callNumber, caller);
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
     state_PTR oldState = (state_PTR) TBLMGMTOLDAREA;
     passUpOrDie(PROGTRAP, oldState);
     /* TODO table handler */  

 }


 