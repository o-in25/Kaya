#include "../h/const.h"
#include "../h/types.h"

#include "../e/pcb.e"
#include "../e/asl.e"
#include "initial.c"

void schedule() {

    

    pcb_PTR currentPcb = mkEmptyProcQ();
    if(procCount < 1) {
        HALT();
    }
    
}