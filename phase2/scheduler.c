#include "../h/const.h"
#include "../h/types.h"
#include "../e/initial.e"
#include "../e/pcb.e"
#include "../e/asl.e"
#include "/usr/local/include/umps2/umps/libumps.e"

/* clock timer */
cpu_t startTOD;
cpu_t currentTOD;
extern void invokeScheduler() {
    if(emptyProcQ(readyQueue)) {
        currentProcess = NULL;
        if(processCount == 0) { /* case 1 */
                HALT();
        }
        if(processCount > 0) {
            if(softBlockedCount == 0) {
                PANIC();
            } else if(softBlockedCount > 0) {
                setSTATUS(getSTATUS() | ALLOFF | INTERRUPTSON | IEc | IM);
                WAIT();
            }
        }
    } else {
        if (currentProcess != NULL) {
            STCK(currentTOD);
            currentProcess->p_time = currentProcess->p_time + (currentTOD - startTOD);
        }
        currentProcess = removeProcQ(&(readyQueue));
        STCK(startTOD);
        setTIMER(QUANTUM);
        contextSwitch(&(currentProcess->p_state));
    }
}
