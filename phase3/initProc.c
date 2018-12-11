#include "../h/const.h"
#include "../h/types.h"
#include "/usr/local/include/umps2/umps/libumps.e"

/* GLOBAL VARIABLES */
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
/* END OF GLOBAL VARIABLES */

/* wrapper function for our phase 3 */
void test() {
	/* some variables for indexing */
	int i;
	int j;

	/* initalize the page table */
	for(i = 0; i < SWAPSIZE; i++) {
		pool[i].pageTableEntry = NULL;
		pool[i].segmentNumber = 0;
		pool[i].pageNumber = 0;
		/* -1 signifies */
		pool[i].ASID = -1;
	}

	/* initialize the semaphores */
	for (i = 0; i < MAXSEMALLOC; i++){
		mutexSemaphores[i] = 1;
	}
	/* initalize the page table entries for the kUsegOS */
	for (i = 0; i < KUSEGPTESIZE; i++) {
		/* occupy the EntryHI CP0 register */
		kSegOS.pteTable[i].entryHI = (PADDRBASE + i) << VPN;
		/* occupy the EntryLO CP0 register */
		kSegOS.pteTable[i].entryLO = ((PADDRBASE + i) << VPN) | VALID | DIRTY | GLOBAL;
	} 

}

static void extractASID() {
	/* extracts the asid from the entryHi cp0 register */
	return getENTRYHI() >> 6;
}

static void initProc() {

}

static void exceptionsStateVector() {
	int i;
	/* initialize an new state */
	state_PTR state = NULL;
	/* for each trap type */
	for(i = 0; i < TRAPTYPES; i++) {
		state_PTR state = &(uProcesses[asid].Tnew_trap[i]);
		state->s_asid = getENTRYHI();

		if(i == TLBTRAP){

		} else if(i == PROGTRAP) {

		} else if(i == TLBTRAP) {

		}

	}
}

	

	
