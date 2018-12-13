/*************************************************** initial.c ************************************************************
	Contains he bootcode for the Kaya Operating System. It contains the definitions for the gobal variables required for
    phase2, being the process count, the softblocked count, the current process (henceforth, jobs and processes will be 
    synonymous), and the ready queue - the queue of jobs who status is marked as ready. In main, there are a number of 
    boot opertions. First, the four new areas in low memory are populated to support context switching. The stack 
    pointer will be set, the program counter will be reflected appropriately and the status of VM will be off, 
    interrupts will be masked, and kernel mode is on. Process control blocks are allocated and the ASL is set. 

***************************************************** initial.c ************************************************************/

/* h files to include */
#include "../h/const.h"
#include "../h/types.h"
/* e files to include */
#include "../e/pcb.e"
#include "../e/asl.e"
#include "../e/initial.e"
#include "../e/interrupts.e"
#include "../e/exceptions.e"
#include "../e/scheduler.e"
#include "../e/p2test.e"
/* include the µmps2 library */
#include "/usr/local/include/umps2/umps/libumps.e"

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
    /* initalize global variables */
    readyQueue = mkEmptyProcQ();
    currentProcess = NULL;
    processCount = 0;
    softBlockedCount = 0;
    /* the device register */
    devregarea_PTR bus = (devregarea_PTR) RAMBASEADDR;
    /* set the top of the RAM to be the base plus the amount 
    of RAM available. This logical sum will equal (obviously) the
    size of available RAM */
    unsigned int RAMTOP = (bus->rambase) + (bus->ramsize);
    
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
    state->s_pc = (memaddr) syscallHandler; 
    /* fill the t9 register */
    state->s_t9 = (memaddr) syscallHandler; 
    /******************************************** PRGMTRAP AREA ****************************************/
    state = (state_PTR) PRGMTRAPNEWAREA;
    state->s_status = ALLOFF;   
    state->s_sp = RAMTOP;
    state->s_pc = (memaddr) programTrapHandler; 
    /* fill the t9 register */
    state->s_t9 = (memaddr)  programTrapHandler; 
    /******************************************** TBLMGMT AREA ****************************************/
    state = (state_PTR) TBLMGMTNEWAREA;
    /* privlaged ROM instruction */
    state->s_status = ALLOFF;   
    state->s_sp = RAMTOP;
    state->s_pc = (memaddr) translationLookasideBufferHandler; 
    /* fill the t9 register */
    state->s_t9 = (memaddr) translationLookasideBufferHandler;
    /******************************************** INTRUPT AREA ****************************************/
    state = (state_PTR) INTRUPTNEWAREA;
    state->s_status = ALLOFF;   
    state->s_sp = RAMTOP;
    state->s_pc = (memaddr) interruptHandler; 
    /* fill the t9 register */
    state->s_t9 = (memaddr) interruptHandler; 

    /* next, we address each semaphore in the ASL free list to have 
    an address of 0 */
    int i;    
    for(i = 0; i < MAXSEMALLOC; i++) {
        /* intialize every semaphore to have a starting address of 0 */
        semdTable[i] = 0;
    }

    /* now, we start up the underlying data structures to support the rest of the 
    operating system - being the process control blocks and the semaphore list */ 
    initPcbs();
    initASL();
    /* allocated a process - just like before, we must now allocate memory according`ly */
    currentProcess = allocPcb();
    currentProcess->p_state.s_sp = (RAMTOP - PAGESIZE);
    currentProcess->p_state.s_pc = (memaddr) test; /* TODO IMPLEMENT TEST CODE */
    currentProcess->p_state.s_t9 = (memaddr) test; /* TODO IMPLEMENT TEST CODE */
    currentProcess->p_state.s_status = (ALLOFF | INTERRUPTSON | IM | TE);
    /* increment the process count, since we have one fired up */
    processCount++;
    /* insert the newly allocated process into the ready queue */
    insertProcQ(&(readyQueue), currentProcess);
    /* its in the queue */
    currentProcess = NULL;
    /* load an interval */
    LDIT(INTERVAL);
    /* call the scheduler */
    invokeScheduler();
}