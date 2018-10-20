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
/* semaphore list */
int semdTable[MAXSEMALLOC];

/* 
* Function: the boot squence for the OS; it will initalize process control blocks and 
* will initialize the active semaphore list. It will also populate low areas of memory
* as well as setting the ground work for context switching. Finally, it will call the scheduler
* which will delegate the rest of the OS, i.e. main will return 1
*/
int main() {
 
    /* the device register */
    devregarea_PTR bus = (devregarea_PTR) RAMBASEADDR;
    /* set the top of the RAM to be the base plus the amount 
    of RAM available. This logical sum will equal (obviously) the
    size of available RAM */
    const unsigned int RAMTOP = (bus->rambase) + (bus->ramsize);
    
    /* now, we need a state (pointer) that will be used to allocate
    the areas of memory; in is encapsulated in the function such that no
    external functions can manipulate the state unintentionally */
    state_PTR state;

    /* then, the areas of low memory are populated, the stack pointer is set
    and the t9 register is filled in each respective location */
    /******************************************** SYSCALL AREA ****************************************/
    state = (state_PTR) SYSCALLNEWAREA;
    state->s_status = ALLOFF;   
    state->s_sp = RAMTOP;
    state->s_pc = (memaddr) NULL; /* TODO: build syscall handler */
    /* fill the t9 register */
    state->s_t9 = NULL; /* TODO: build syscall handler */
    /******************************************** PRGMTRAP AREA ****************************************/
    state = (state_PTR) PRGMTRAPNEWAREA;
    state->s_status = ALLOFF;   
    state->s_sp = RAMTOP;
    state->s_pc = (memaddr) NULL; /* TODO: build program trap handler */
    /* fill the t9 register */
    state->s_t9 = NULL; /* TODO: build program trap handler */
    /******************************************** TBLMGMT AREA ****************************************/
    state = (state_PTR) TBLMGMTNEWAREA;
    state->s_status = ALLOFF;   
    state->s_sp = RAMTOP;
    state->s_pc = (memaddr) NULL; /* TODO: build table management handler */
    /* fill the t9 register */
    state->s_t9 = NULL; /* TODO: build table management handler */
    /******************************************** INTRUPT AREA ****************************************/
    state = (state_PTR) INTRUPTNEWAREA;
    state->s_status = ALLOFF;   
    state->s_sp = RAMTOP;
    state->s_pc = (memaddr) NULL; /* TODO: build interrupt handler */
    /* fill the t9 register */
    state->s_t9 = NULL; /* TODO: build interrupt handler */


    /* next, we address each semaphore in the ASL free list to have 
    an address of 0 */
    int i;    
    for(i = 0; i < MAXSEMALLOC; i++) {
        /* intialize every semaphore to have a starting address of 
        0 */
        semdTable[i] = 0;
    }

    initPcbs();
    initASL();
}

