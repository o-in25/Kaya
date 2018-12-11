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

static void initUProcs() {
	int i;
	int j;
	
}

/* prepare a new processor state */
static state_PTR prepareProcessorState(int flag, int index) {
	/* preparing a processor state appropriate for the 
	execution of uprocs */
	state_PTR processorState;
	/* is this being prepared in the segment table? */
	if(flag) {
		/* the new processor state dictates that interupts are enabled,
		user mode is on, status.te is 1 and statis vmc = 1 */
		processState->s_status = ALLOFF | INTERRUPTSON | IM | TE | VMc | KUo;
		/* set the text area masks */
		processorState->s_pc = (memaddr) TEXTAREASEGMENTMASK;
		processorState->s_t9 = (memaddr) TEXTAREASEGMENTMASK;
		processorState->s_sp = TEXTAREASEGMENTMASK;
		/* set the asid */
		processorState->s_asid = getENTRYHI();
	} else {
		processorState->s_asid = (index << ASIDMASK);
		processorState->s_status = ALLOFF | IEc | IM | TE;
		/* TODO: set up the handler */
		processorState->s_t9 = NULL;
		processorState->s_status = NULL;

	}
	return processorState;
}

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
	for(i = 0; i < MAXSEMALLOC; i++){
		mutexSemaphores[i] = 1;
	}
	/* initalize the page table entries for the kUsegOS */
	for (i = 0; i < KUSEGPTESIZE; i++) {
		/* occupy the EntryHI CP0 register */
		kSegOS.pteTable[i].entryHI = (PADDRBASE + i) << VPN;
		/* occupy the EntryLO CP0 register */
		kSegOS.pteTable[i].entryLO = ((PADDRBASE + i) << VPN) | VALID | DIRTY | GLOBAL;
	} 
	/* initialize the header */
	kSegOS.header = MAGICNO << PGTBLHEADERWORD | KSEGOSPTESIZE;
	for(i = 0; i < MAXUPROC; i++) {
		/* get the ith uProc */
		Tproc_PTR userProc = &(uProcesses[i - 1]);
		/* initialize the header */
		userProc->Tp_pte.header = (MAGICNO << PGTBLHEADERWORD | KUSEGPTESIZE);

		/* set up the page table entry */
		for(j = 0; j < KUSEGPTESIZE; j++) {
			/* TODO: set up entryHI */
			userProc->Tp_pte.pteTable[j].entryHI = NULL;
			userProc->Tp_pte.pteTable[j].entryLO = ALLOFF | DIRTY;
		}
		/* get the address of ith entry the segment table */
		segt_PTR segmentTable = (segt_PTR) SEGSTART + (i * SEGWIDTH);
		/* point to the kSegOS segment */
		segmentTable->kSegOS = &kSegOS;
		segmentTable->kUseg2 = &(userProc->Tp_pte);
		/* prepare the processor state */
		state_PTR processorState = prepareProcessorState(TRUE, i);
		userProc->Tp_sem = 0;

		if(SYSCALL(CREATEPROCESS, (int) processorState, EMPTY, EMPTY) != SUCCESS) {
			SYSCALL(TERMINATEPROCESS, EMPTY, EMPTY, EMPTY);
		}
		if(i < MAXUPROC + 1) {
			SYSCALL(PASSERN, (int) &masterSemaphore, EMPTY, EMPTY);
		}
	}
	/* end the process */
	SYSCALL(TERMINATEPROCESS, EMPTY, EMPTY, EMPTY);
}

/* 
* Function: extract ASID
* Extracts the entryLO register. The register will have the bits 0-5 to be unused,
* and thus a bitwise shift to the 6th bit is where we begin. Then,
* since the ASID is bits 6-11, the ENTRYHIASID constant will determine
* which buts signify the ASID
*/
static void extractASID() {
	/* extracts the asid from the entryHi cp0 register */
	return ((getENTRYHI() & ENTRYHIASID) >> ASIDMASK);
}

static void initUProc() {

	int asid = extractASID();
	device_PTR tapeDevice;
	device_PTR diskDevice; 

	while((tapeDevice->d_data1 != EOF)) {
		diskOperation();
	}

	/* set up the exception state vectors for the sys-5 pass up 
	or die helper method */
	initializeExceptionsStateVector();
	/* prepare a new processor state */
	state_PTR processorState = prepareProcessorState(TRUE, 0);
	/* perform a context switch for the prepared state */
	contextSwitch(processorState);
}


static void diskOperation() {
	/* TODO: build this */
}


/* sets up pass up or die stuff */
static void initializeExceptionsStateVector() {
	state_PTR state;
	int i;
	/* initialize an new state */
	state_PTR state = NULL;
	/* for each trap type */
	for(i = 0; i < TRAPTYPES; i++) {
		/* get the state by the asid */
		state_PTR state = &(uProcesses[extractASID()].Tnew_trap[i]);
		if(i == TLBTRAP){
			/* TODO: set up the proper exception handler */
			state->s_t9 = NULL;
			state->s_pc = NULL;
		} else if(i == PROGTRAP) {
			/* TODO: set up the proper exception handler */
			state->s_t9 = NULL;
			state->s_pc = NULL;
		} else if(i == TLBTRAP) {
			/* TODO: set up the proper exception handler */
			state->s_t9 = NULL;
			state->s_pc = NULL;
		}
	}
	/* perform a sys5 - specify state exceptions vector */
	/* SYSCALL() */
}



	
