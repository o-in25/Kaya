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
        currentProcess = removeProcQ(&(readyQueue));
        if(currentTOD < 0) {
            currentTOD = 0;
        } else if(currentTOD < QUANTUM) {
            setTIMER(currentTOD);
        } else {
            setTIMER(QUANTUM);
        }
        contextSwitch(&(currentProcess->p_state));
    }
}
