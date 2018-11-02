#include "../h/const.h"
#include "../h/types.h"
#include "../e/initial.e"
#include "../e/pcb.e"
#include "../e/asl.e"
#include "/usr/local/include/umps2/umps/libumps.e"

cpu_t currentTOD;
cpu_t startedTOD;

extern void invokeScheduler() {
    pcb_PTR currentProcess = removeProcQ(&(readyQueue));
    if((emptyProcQ(&(readyQueue)))) {
        if(processCount == 0) { /* case 1 */
                /* our work here is done. there are no jobs in the ready queue
                and we have no processes running */
                HALT();
        }
        if(processCount > 0) {
            /* no current process, since the we have no process counts */
            currentProcess = NULL;
            /* now, we have 2 subcases. there isn't anything on the ready queue, but we have at least one active process
            so why is this? we are either softblocked and waiting on I/O - in which case all is good, we just wait it out.
            but if we are not waiting on I/O there's nothing on the ready queue, AND we have a processes lingering,
            we panic */
            if(softBlockedCount == 0) {
                /* all is good - waiting on I/O to finish up */
                PANIC();
            } else if(softBlockedCount > 0) {
                /* kernel panic. we have nothing on the ready queue, we have a process lingering - but it's not
                I/O - time to panic */
                setSTATUS(getSTATUS() | ALLOFF | INTERRUPTSON | IEc | IM);
                WAIT();
            }
        }
        currentProcess = removeProcQ(&(readyQueue));
        currentProcess->p_time += currentTOD - startTOD;
        setTIMER(QUANTUM);
        /* context switch */
        debugA(3434);
        LDST(&(currentProcess->p_state));
    }
}
