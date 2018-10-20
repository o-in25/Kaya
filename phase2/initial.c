/*************************************************** initial.c ************************************************************
	inital.c is the bootcode for the Kaya Operating System. It contains the definitions for the gobal variables required for
    phase2, being the process cout, the softblocked count, the current process, and the ready queue - the queue of jobs who
    status is marked as ready. In main, there are a number of boot opertions. First, the four new areas in low memory are 
    populated to support context switching. The stack pointer will be set, the program counter will be reflected appropriately
    and the status of VM will be off, interrupts will be masked, and kernel mode is on. Process control blocks are allocated
    and the ASL is set. 

***************************************************** initial.c ************************************************************/

/* h files to include */
#include "../h/const.h"
#include "../h/types.h"
/* e files to include */
#include "../e/pcb.e"
#include "../e/asl.e"

/* globals */
/* the current process count */
int processCount;
/* the soft blocked count */
int softBlockedCount;
/* the current process */
pcb_PTR currentProcess;
/* the queue of ready processes */
pcb_PTR readyQueue;

/* 
* Function: the boot squence for the OS; it will initalize process control blocks and 
* will initialize the active semaphore list. It will also populate low areas of memory
* as well as setting the ground work for context switching. Finally, it will call the scheduler
* which will delegate the rest of the OS, i.e. main will return 1
*/
int main() {
 
    state_PTR state;

    /* first, the syscakk area is populated, the stack pointer is set
    and teh t9 register is filled */
    state = (state_PTR) SYSCALLNEWAREA;
    state->s_status = ALLOFF;

    /* the device register */
    devregarea_PTR bus = (devregarea_PTR) RAMBASEADDR;
    unsigned int ramTop = (bus->rambase) + (bus->ramsize);

}

