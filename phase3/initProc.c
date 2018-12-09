#include "../h/const.h"
#include "../h/types.h"

/* GLOBAL VARIABLES */
pteOS_t kSegOS;
pte_t kUseg3;
swapPool_t pool[100];
Tproc_t uProcesses[8];
int next;
int disk1Semaphore;
int disk0Semaphore;
int swapSemaphore;
int masterSemaphore;
int mutexSemaphores[MAXSEMALLOC];
/* END OF GLOBAL VARIABLES */

void test() {
	/* some variables for indexing */
	int i;
	int j;

	/* initalize the page table */
	for(i = 0; i < SWAPSIZE; i++) {

	}

	for(i = 0; i < KSEGOSPTESIZE; i++) {

	}

	for(i = 0; i < MAXSEMALLOC; i++) {

	}

	for(i = 0; i < MAXUPROC; i++) {

	}

	
}
