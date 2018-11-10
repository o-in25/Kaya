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
    /* was someone just running? */
    /* this means a process was running and was then blocked 
	 * or returned to readyQ for some reason */
    if (currProc != NULL)
    {
        /* save how much time current process used on CPU */
        /* subtract current time from global start time to get this ^ */
        STCK(currentTOD);
        currentProcess->p_time = (currentProcess->p_time) + (currentTOD - startTOD);
    }

    if (!emptyProcQ(readyQueue))
    {
        /* start next process in queue */
        currentProcess = removeProcQ(&readyQueue);
        /* get start time */
        STCK(startTOD);
        /* load QUANTUM into process timer */
        setTIMER(QUANTUM);

        contextSwitch(&(currentProcess->p_state));
    }
    else
    {
        /* nothing was in readyQueue */
        currentProcess = NULL; /* no running process */

        /* finished all processes properly */
        if (processCount == 0)
        {
            HALT();
        }

        /* deadlock */
        if (processCount > 0 && softBlockedCount == 0)
        {
            PANIC();
        }

        /* now it's just a waiting game */
        if (processCount > 0 && softBlockedCount > 0)
        {
            setSTATUS(getSTATUS() | ALLOFF | INTERRUPTSON | IEc | IM);
            WAIT(); /* run silent run deep */
        }
    }
}
