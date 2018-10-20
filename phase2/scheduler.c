#include "../h/const.h"
#include "../h/types.h"

#include "../e/pcb.e"
#include "../e/asl.e"
#include "initial.c"

void invokeScheduler() {


    currentProcess = removeProcQ(&(readyQueue));
}





