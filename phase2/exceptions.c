#include "../h/const.h"
#include "../h/types.h"
/* e files to include */
#include "../e/initial.e"
#include "../e/scheduler.e"
#include "../e/interrupts.e"
#include "../e/pcb.e"
#include "../e/asl.e"
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

static void waitForClock(state_PTR state) {
    int* semaphore = (int*) &(semdTable[(MAXSEMALLOC - 1)]);
    (*semaphore)--;
    if((*semaphore) < 0) {
        softBlockedCount++;
        insertBlocked(semaphore, currentProcess);
        copyState(state, &(currentProcess->p_state));
    }
    invokeScheduler();
}

static void getCpuTime(state_PTR state) {
    copyState(state, &(currentProcess->p_state));
    cpu_t stopTOD;
    STCK(stopTOD);
    cpu_t elapsedTime = stopTOD - startTOD;
    currentProcess->p_time = (currentProcess->p_time) + elapsedTime;
    currentProcess->p_state.s_v0 = currentProcess->p_time;
    STCK(startTOD);
    contextSwitch(&(currentProcess->p_state));
}

static void specifyExceptionsStateVector(state_PTR state) {
    switch(state->s_a1) {
        case TLBTRAP:
            if(currentProcess->newTlb != NULL) {
                terminateProcess();
            }
            currentProcess->newTlb = (state_PTR) state->s_a3;
            currentProcess->oldTlb = (state_PTR) state->s_a2;
            break;
        case PROGTRAP:
            if(currentProcess->newPgm != NULL) {
                terminateProcess();
            }
            currentProcess->newPgm = (state_PTR) state->s_a3;
            currentProcess->oldPgm = (state_PTR) state->s_a2;
            break;
        case SYSTRAP:
          if(currentProcess->newSys != NULL) {
                terminateProcess();
            }
            currentProcess->newSys = (state_PTR) state->s_a3;
            currentProcess->oldSys = (state_PTR) state->s_a2;
            break;
    }
    contextSwitch(state);
}

static void passeren(state_PTR state) {
    int* semaphore = (int*) state->s_a1;
    (*(semaphore))--;
    if((*(semaphore)) < 0) {
        copyState(state, &(currentProcess->p_state));
        insertBlocked(semaphore, currentProcess);
        invokeScheduler();
    }
    contextSwitch(state);
}

static void verhogen(state_PTR state) {
    int* semaphore = (int*) state->s_a1;
    (*(semaphore))++;
    if(*(semaphore) <= 0) {
        pcb_PTR newProcess = removeBlocked(semaphore);
        if(newProcess != NULL) {
            insertProcQ(&(readyQueue), newProcess);
        }
    }
    contextSwitch(state);
}

static void terminateProcess() {
    if(!emptyChild(currentProcess)) {
        terminateProgeny(currentProcess);
    } else {
        processCount--;
        outChild(currentProcess);
        freePcb(currentProcess);
    }

    currentProcess = NULL;
    invokeScheduler();
}

static void createProcess(state_PTR state) {
    pcb_PTR p = allocPcb();
    if(p == NULL) {
        state->s_v0 = -1;
        contextSwitch(state);
    }
    processCount++;
    insertChild(currentProcess, p);
    insertProcQ(&(readyQueue), p);
    state_PTR temp = (state_PTR) state->s_a1;
    copyState(temp, &(p->p_state));
    state->s_v0 = 0;
    contextSwitch(state);
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

static void userModeHandler(state_PTR state) {
    state_PTR programTrapOldArea = (state_PTR)PRGMTRAPOLDAREA;
    copyState(state, programTrapOldArea);
    unsigned int placeholder = (programTrapOldArea->s_cause) & ~(FULLBYTE);
    (programTrapOldArea->s_cause) = (placeholder | RESERVED);
    programTrapHandler();
}

/************************************************************************************************************************/
/*************************************** EXCEPTION HANDLERS *************************************************************/
/************************************************************************************************************************/
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

 void programTrapHandler() {
     state_PTR oldState = (state_PTR) PRGMTRAPOLDAREA;
     passUpOrDie(PROGTRAP, oldState);
 }

 void translationLookasideBufferHandler() { 
     state_PTR oldState = (state_PTR) TBLMGMTOLDAREA;
     passUpOrDie(TLBTRAP, oldState);
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

 static void passUpOrDie(int callNumber, state_PTR old) {
     switch(callNumber) {
        case SYSTRAP:
            if(currentProcess->newSys != NULL) {
                copyState(old, currentProcess->oldSys);
                contextSwitch(currentProcess->newSys);
            }
            break;
        case TLBTRAP:
            if(currentProcess->newTlb != NULL) {
                copyState(old, currentProcess->oldTlb);
                contextSwitch(currentProcess->newTlb);
            }
            break;
        case PROGTRAP: 
            if(currentProcess->newPgm != NULL) {
                copyState(old, currentProcess->oldPgm);
                contextSwitch(currentProcess->newPgm);
            break;
        }
    }
    terminateProcess();
 }
