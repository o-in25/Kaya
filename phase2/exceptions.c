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

static void delegateSyscall(int callNumber, pcb_PTR caller) {
     switch(callNumber) {
            case WAITFORIODEVICE: /* SYSCALL 8 */
                waitForClock();
            case WAITFORCLOCK: /* SYSCALL 7 */
                waitForClock();
            case GETCPUTIME: /* SYSCALL 6 */
                getCpuTime();
            case SPECIFYEXCEPTIONSTATEVECTOR: /* SYSCALL 5 */
                specifyExceptionsStateVector();
            case PASSEREN: /* SYSCALL 4 */
                passeren(caller);
            case VERHOGEN: /* SYSCALL 3 */
                verhogen(caller);
            case TERMINATEPROCESS: /* SYSCALL 2 */ 
                terminateProcess();   
            case CREATEPROCESS: /* SYSCALL 1 */
                createProcess(caller);
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
    LDST(state);
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
    /* load the processor state with the privalged ROM isntruction */
    LDST(caller);
}


 void syscallHandler() {
    pcb_PTR process;
    int* semaphoreAddress;
    int* semaphoreDevice;
    int userMode = FALSE;
    state_PTR syscallOldArea;
    state_PTR programTrapOldArea;
    state_PTR caller = (state_PTR) SYSCALLOLDAREA;
    if(!userMode) {
        int callNumber = 0; /* TODO: properly assign the number and handle case  */
        delegateSyscall(callNumber, caller);
    } else {
        passUpOrDie();
    }
 }

 

 void programTrapHandler() {
     /* TODO program handler */
 }

 void tableHandler() {
     /* TODO table handler */  

 }

 static void passUpOrDie() {
    
 }

 