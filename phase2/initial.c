#include "../h/const.h"
#include "../h/types.h"

#include "../e/pcb.e"
#include "../e/asl.e"

int procCount;
int softBlkCount;
pcb_PTR currentProc;
pcb_PTR readyProcQ;


int main() {
    initPcbs();
    initASL();

    readyProcQ = mkEmptyProcQ();
    
    currentProc = readyProcQ;

}

