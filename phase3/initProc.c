#include "../h/const.h"
#include "../h/types.h"
#include "../e/pager.e"

/* GLOBAL VARIABLES */

/* Page table for kUseg3 */
/* 8 kUseg2 page tables */
/* kSegOS page table */

/* master semaphore */
int masterSemaphore;

/* swap pool semaphore */
int swapSem;

/*device semaphores */
int mutexSemaphores[MAXSEMALLOC];

/* framepool data structure */
frame swappool[MAXPROCESS*2];

/* user process data structure */
user userProcess[MAXPROCESS];

/* END OF GLOBAL VARIABLES */

void test() {

	/*initialization for elements to be put into where we start */
	for (int i = 0; i < MAXSEMALLOC; i++){
        	mutexSemaphores[i] = 1;
	}
	masterSemaphore = 0;
	/*need to initialize the semaphores for userProcess to 0 */
	/* initialization for kSegOS */
	for (int i = 0; i < 64; i++){
        	/*set entryHi to 0x20000+i*/
        	/*set entryLO to 0x20000+i w/Dirt, Global, Valid */
	}
	/* initialization for KUseg3 */
	for (int i = 0; i < 32; i++){
        	/* set entryHI = 0xC0000+i */
        	/* set entryLO = Dirty, Global */
	}
	/* initialization for swappool */
	for (int i = 0; i < MAXPROCESS*2; i++) {
        	swappool[i].ASID = -1;
	}
	/* swapPool semaphore */
	swapSem = 1;

	/* process initialization loop */
	for (int i = 1; i < MAXPROC+1; i++){
        	/*initialize stuff? */
        	/* SYS 1*/
	}

	for (int i = 0; i < MAXPROC; i++){
        	/*P(masterSemaphore) */
	}
	/* end with SYS2 */    
}
