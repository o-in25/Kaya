/*************************************************** scheduler.c ********************************************************
	Manages the requests for new jobs in the Kaya OS and is resonsible for scheduling them. The scheduler will 
    check if there are any jobs in the ready queue. If there are no jobs in the ready queue, the schedule 
    will ten determine why. If there are no processes listed as running, then the system halts. Otherwise, if 
    there is a record of processes running (the process count is greater than zero), then it will check if 
    any of these jobs are simply waiting on IO to finish. If they are, the scheduler will simply invoke the ROM
    reserved WAIT instruction - and sets up the bit masks for the next job. Otherwise, it enters a kernel panic.
    If there are jobs in the ready queue, however, the scheduler grab a job from the ready queue,
    will initialize their timer, set their quantum, and perform a context switch on that new job.

    This module contributes function definitions and a few sample fucntion implementations to the contributors put 
    forth by the Kaya OS project.

***************************************************** scheduler.c ******************************************************/

/* h files to include */
#include "../h/const.h"
#include "../h/types.h"
/* e files to include */
#include "../e/initial.e"
#include "../e/pcb.e"
#include "../e/asl.e"
/* include the µmps2 library */
#include "/usr/local/include/umps2/umps/libumps.e"

/* GLOBAL VARIABLES */
cpu_t startTOD;
/* clock timers */
cpu_t currentTOD;
/* END OF GLOBAL VARIABLES */

/************************************************************************************************************************/
/*************************************************** SCHEDULER  *********************************************************/
/************************************************************************************************************************/

/* 
* Function: Invoke Scheduler 
* In charge of assigning processes as well 
* the unit responsible for deadlock. If there are 
* no ready processes and there are no current processes,
* then the the system halts. If there are current processes, 
* but none are waiting on I/O the system will issue
* a kernel panic. Otherwise, the scheduler will wait as a means 
* of deadlock detection. Otherwise, jobs are scheduled using a 
* simple round-robbin algorithm.
*/
void invokeScheduler() {
    /* are there any ready jobs? */
    if(emptyProcQ(readyQueue)) {
        /* we have no running process */
        currentProcess = NULL;
        /* do we have any job to do? */
        if(processCount == 0) {
            /* nothing to do */
            HALT();
        }
        /* are we waiting on I/O? */
        if(processCount > 0) {
            /* do are we waiting for I/O? */
            if(softBlockedCount == 0) {
                /* not a job, not waiting for a job, 
                and are not a job itself - kernel panic */ 
                PANIC();
                /* are we waiting for I/O? */
            } else if(softBlockedCount > 0) {
                /* enable interrupts for the next job */
                setSTATUS(getSTATUS() | ALLOFF | INTERRUPTSON | IEc | IM);
                /* wait */
                WAIT();
            }
        }
    } else {
        /* simply ready the next job using round-robbin */
        if (currentProcess != NULL) {
            /* start the clock */
            STCK(currentTOD);
            currentProcess->p_time = currentProcess->p_time + (currentTOD - startTOD);
        }
        /* generate an interrupt when timer is up */
        if(currentTOD < QUANTUM) {
            /* our current job will be less than 
            our quantum, take the shorter */
            setTIMER(currentTOD);
        } else {
            /* set the quantum */
            setTIMER(QUANTUM);
        }
        /* grab a job */
        currentProcess = removeProcQ(&(readyQueue));
        STCK(startTOD);
        /* perform a context switch */
        contextSwitch(&(currentProcess->p_state));
    }
}
