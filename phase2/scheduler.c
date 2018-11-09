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
        debugger(8003);
        currentProcess = NULL;
        if(processCount == 0) { /* case 1 */
                /* our work here is done. there are no jobs in the ready queue
                and we have no processes running */
                debugger(8004);
                HALT();
        }
        if(processCount > 0) {
            /* no current process, since the we have no process counts */
            /* now, we have 2 subcases. there isn't anything on the ready queue, but we have at least one active process
            so why is this? we are either softblocked and waiting on I/O - in which case all is good, we just wait it out.
            but if we are not waiting on I/O there's nothing on the ready queue, AND we have a processes lingering,
            we panic */
            if(softBlockedCount == 0) {
                /* all is good - waiting on I/O to finish up */
                debugger(8005);
                PANIC();
            } else if(softBlockedCount > 0) {
                /* kernel panic. we have nothing on the ready queue, we have a process lingering - but it's not
                I/O - time to panic */
                setSTATUS(getSTATUS() | ALLOFF | INTERRUPTSON | IEc | IM);
                debugger(8006);
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
        /* DEBUG NOTES: got to here before printing p */
        debugger(8004);
        contextSwitch(&(currentProcess->p_state));
    }
}
