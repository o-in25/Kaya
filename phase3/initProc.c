#include "../h/const.h"
#include "../h/types.h"
#include "../e/pager.e"

/* GLOBAL VARIABLES */

/* Page table for kUseg3 */
pt_t kUseg3;

/* 8 kUseg2 page tables */
pt_t kUseg2[MAXPROCCESS];

/* kSegOS page table */
ptOS_t kSegOS;

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
	for (int i = 0; i < MAXSEMALLOC; i++) {
        	mutexSemaphores[i] = 1;
	}
	masterSemaphore = 0;
	/*need to initialize the semaphores for userProcess to 0 */
	/* initialization for kSegOS */
	for (int i = 0; i < 64; i++){
        	/*set entryHi to 0x20000+i*/
		kSegOS.entries[i].entryHI = (0x20000+i) << 12;

        	/*set entryLO to 0x20000+i w/Dirt, Global, Valid */
		kSegOS.entries[i].entryLO = ((0x20000 + i) << 12) | DIRTY | VALID | GLOBAL;
	}

	/* initialization for KUseg3 */
	for (int i = 0; i < 32; i++){
        	/* set entryHI = 0xC0000+i */
		kUseg3.entries[i].entryHI = (0xC0000+i) << 12;
        	/* set entryLO = Dirty, Global */
		kUseg3.entries[i].entryLO = DIRTY | GLOBAL;
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
		userProcess[i].ASID = i;
		for (int x = 0; x < 32; x++){
                	/* set entryHI = 0x80000+i */
			kUseg2[i].entries[x].entryHI = ((0x80000+x) << 12) | (i << 6);
                	/* set entryLO = Dirty */
			kUseg2[i].entries[x].entryLO = DIRTY;
			/* set the KSegOS pointer */
			
			/* set KUseg2 pointer */
			/* set KUseg3 pointer */
        	}
		/*fix the last entry's entryHI to 0xBFFFF w/ASID */
		kUseg2[i].entries[31].entryHI = (0xBFFFF << 12) | (i << 6);
        segTable = (st_PTR) (0x20000500 + (12*i));
        segTable->kSegOS = &kSegOS;
        segTable->kUseg2 = &kUseg2;
        segTable->kUseg3 = &kUseg3;
        	/* SYS 1*/
        SYSCALL (CREATEPROCESS, /* nore sure about state yet */, 0, 0);
	}
	
	
	
	for (int i = 0; i < MAXPROC; i++){
        SYSCALL (PASSERAN, (int) &masterSemaphore, 0, 0);
	}
	/* end with SYS2 */
    SYSCALL (TERMINATEPROCESS, 0, 0, 0);
}
