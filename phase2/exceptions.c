#include "../h/const.h"
#include "../h/types.h"


static void waitForDevice() {

}

static void waitForClock() {

}

static void getCpuTime() {

}

static void specifyExceptionsStateVector() {

}


static void passeren(state_PTR state) {
    int* semaphore = (int*) state->s_a1;
    (*(semaphore))++;
    if(*(semaphore) < 1) {

    }
    LDST(state);
}

static void verhogen(state_PTR state) {
    int* semaphore = (int*) state->s_a1;
    (*(semaphore))--;
    LDST(state);

}

static void terminateProcess() {

}

static void createProcess() {

}


 void syscallHandler() {
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
 }

 void programTrapHandler() {
     /* TODO program handler */
 }

 void tableHandler() {
     /* TODO table handler */  

 }

 static void passUpOrDie() {
    
 }

 