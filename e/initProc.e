#include "../h/const.h"
#include "../h/types.h"
#ifndef INITPROC
#define INITPROC
    pteOS_t kSegOS;
    pte_t kUseg3;
    swapPool_t pool[100];
    Tproc_t uProcesses[MAXUPROC];
    int next;
    int disk1Semaphore;
    int disk0Semaphore;
    int swapSemaphore;
    int masterSemaphore;
    int mutexSemaphores[MAXSEMALLOC];
#endif