#include "../h/const.h"
#include "../h/types.h"
#include "../e/pager.e"


/* Page table for kUseg3 */
/* 8 kUseg2 page tables */
/* kSegOS page table */

/* master semaphore */
int* masterSemaphore;

/*device semaphores */
int mutexSemaphores[MAXSEMALLOC];

/* framepool data structure */
frame swappool[MAXPROCESS*2];

/* user process data structure */
user userProcess[MAXPROCESS];

/*initialization for elements to be put into where we start */
for (int i = 0; i < MAXSEMALLOC; i++){
	semdTable[i] = 1;
}
*masterSemaphore = 0;
/*need to initialize the semaphores for userProcess to 0 */ 


void test() {
    
}
