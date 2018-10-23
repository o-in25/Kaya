#include "../h/const.h"
#include "../h/types.h"

#include "../e/pcb.e"
#include "../e/asl.e"
#include "initial.c"

void invokeScheduler() {
    pcb_PTR currentProcess = removeProcQ(&(readyQueue));
    /*  */
    if(!(emptyProcQ(&(readyQueue)))) {
        /* TODO: load the state */
        LDST(&(currentProcess->p_state));
    } else if(currentProcess == NULL) {
        
        /* there is nothing on the ready queue to be scheduled. we must check for special cases first, what if there are no 
        processes left in the system - that is, the process count is less than 1? If this is the case, we invoke 
        the privilaged ROM command HALT. secondly, what if there are running processes in the system, but the ready 
        queue is empty? there are possible cases for this, despite seeming unintuitive. first, if there is the possibility 
        that a process is waiting on I/O to complete. If this is the case, the softblocked out will be > 0. Since these 
        processes are gaurenteed to cause an interrupt, we simply have to wait for them to finish. appropriately, this is done
        with the privaleged ROM command WAIT. finally, if neither of these are the case there is an error. here, there are no 
        jobs in the ready queue - but none of them are waiting for I/O. If this is the case, the ROM command PANIC is invoked  
        and we close up shop */
        /* case 1: the ready queue is empty, but there are soft blocked processes waiting to return */
        if(processCount == 0) { /* case 1 */
            /* our work here is done. there are no jobs in the ready queue
            and we have no processes running */
            HALT();
        } else if(processCount > 0) { /* case 2 */
            /* no current process, since the we have no process counts */
            currentProcess = NULL;
            /* now, we have 2 subcases. there isn't anything on the ready queue, but we have at least one active process
            so why is this? we are either softblocked and waiting on I/O - in which case all is good, we just wait it out.
            but if we are not waiting on I/O there's nothing on the ready queue, AND we have a processes lingering,
            we panic */
            if(softBlockedCount > 0) {
                /* all is good - waiting on I/O to finish up */
                WAIT();
            } else if(softBlockedCount == 0) {
                /* kernel panic. we have nothing on the ready queue, we have a process lingering - but it's not
                I/O - time to panic */
                setSTATUS(getSTATUS() | IEc | IM);
                PANIC();
            }
        } 
    } else {
        /* TODO timer */
    }
}
