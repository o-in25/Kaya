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

static void createProcess() {

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
            case WAITFORIODEVICE:
                waitForClock();
            case WAITFORCLOCK:
                waitForClock();
            case GETCPUTIME:
                getCpuTime();
            case SPECIFYEXCEPTIONSTATEVECTOR:
                specifyExceptionsStateVector();
            case PASSEREN:
                passeren();
            case VERHOGEN:
                verhogen();
            case TERMINATEPROCESS:
                terminateProcess();   
            case CREATEPROCESS:
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

 