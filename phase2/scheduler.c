#include "../h/const.h"
#include "../h/types.h"

#include "../e/pcb.e"
#include "../e/asl.e"
#include "initial.c"

void invokeScheduler() {

    if(processCount < 1) {

    } else {
        if(softBlockedCount < 1) {
            /* invoke privilaged ROM instruction */
            PANIC();
        } else {
            WAIT();
        }
    }
    currentProcess = removeProcQ(&(readyQueue));




    /* privlaged ROM instruction */
    LDST();
}

void schedule() {

}

