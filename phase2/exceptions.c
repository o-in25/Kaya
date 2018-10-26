#include "../h/const.h"
#include "../h/types.h"
/* e files to include */
#include "../e/initial.e"
#include "../e/scheduler.e"
#include "../e/pcb.e"
#include "../e/asl.e"


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

static void terminateProcess() {

}


/* Function: Syscall 1 - Create Process 
*
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

    if(!userMode) {
        int callNumber = 0; /* TODO: properly assign the number and handle case  */
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
                passeren();
            case VERHOGEN: /* SYSCALL 3 */
                verhogen();
            case TERMINATEPROCESS: /* SYSCALL 2 */ 
                terminateProcess();   
            case CREATEPROCESS: /* SYSCALL 1 */
                createProcess();
        }
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

 