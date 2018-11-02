#ifndef INIT
#define INIT
#include "../h/types.h"
#include "../h/const.h"


    /* globals */
    /* the current process count */
    extern int processCount;
    /* the soft blocked count */
    extern int softBlockedCount;
    /* the current process */
    extern pcb_PTR currentProcess;
    /* the queue of ready processes */
    extern pcb_PTR readyQueue;
    /* semaphore list */
    extern int semdTable[MAXSEMALLOC];
    /* clock */
    extern cpu_t startTOD;
/* * */
    extern cpu_t currentTOD;

#endif
