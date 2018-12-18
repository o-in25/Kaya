#include "../h/const.h"
#include "../h/types.h"
#include "../e/sysSupport.e"
/* include the µmps2 library */
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


int debugger(int* i) {
	i = 0;
	i = 42;
}


/* will invalidate a page table entry given a frame number */
void invalidateEntry(int frameNumber) {
    pool[frameNumber].pageTableEntry->entryLO = ALLOFF | DIRTY;
    pool[frameNumber].ASID = -1;
    pool[frameNumber].pageNumber = 0;
    pool[frameNumber].segmentNumber = 0;
    /* were done */
    pool[frameNumber].pageTableEntry = NULL;
    /* deal with the TLB cache consistency */
    /* by clearing the TLB */
    TLBCLR();
}

/* gets the ball rolling */
void initUProc() {

	int asid = extractASID();
	int asidIndex = asid - 1;
	/* set up the disk */
	device_PTR diskDevice = (device_PTR) DISKDEV;
	/* set up the tape */
	device_PTR tapeDevice = (device_PTR) TAPEDEV + ((asidIndex) * DEVREGSIZE);
	/* set up a memory buffer */
	int memoryBuffer = BUFFER + (asidIndex * PAGESIZE);

	/* set up the exception state vectors for the sys-5 pass up 
	or die helper method */
	initializeExceptionsStateVector();
	int pageNumber = 0;
	/* read until we reach the end of line character or the 
	end of tape marker */
	int diskInformation[DISKPARAMS];
	while((tapeDevice->d_data1 != EOF) && (tapeDevice->d_data1 != EOT)) {
		/* while there are things to do... */
		tapeDevice->d_data0 = memoryBuffer;
		tapeDevice->d_command = READBLK;
		/* set up the parameters for a disk 
		operation */
		diskInformation[SECTOR] = asidIndex;
		diskInformation[CYLINDER] = pageNumber;
		diskInformation[HEAD] = EMPTY;
		diskInformation[DISKNUM] = EMPTY;
		diskInformation[PAGELOCATION] = memoryBuffer;
		diskInformation[READWRITE] = WRITEBLK;
		/* perform a disk I/O now that we have all of the 
		information we need */
		diskOperation(diskInformation, &(disk0Semaphore), diskDevice);
		/* keep track of the pages */
		pageNumber++;
	}

	/* prepare a new processor state */
	state_PTR processorState = prepareProcessorState(FALSE, 0);
	/* perform a context switch for the prepared state */
	contextSwitch(processorState);
}

/* prepare a new processor state */
static state_PTR prepareProcessorState(int flag, int index) {
	/* preparing a processor state appropriate for the 
	execution of uprocs */
	state_PTR processorState;
	/* is new processor state for variable 
	initialization or for reading from a tape? */
	if(flag) {
		/* the new processor state dictates that interupts are enabled,
		user mode is on, status.te is 1 and statis vmc = 1 */
		processorState->s_status = ALLOFF | INTERRUPTSON | IM | TE | VMc | KUo;
		/* set the text area masks */
		processorState->s_pc = (memaddr) TEXTAREASEGMENTMASK;
		processorState->s_t9 = (memaddr) TEXTAREASEGMENTMASK;
		processorState->s_sp = TEXTAREASEGMENTMASK;
		/* set the asid */
		processorState->s_asid = getENTRYHI();
	} else {
		/* this is for variable initialization i.e. step 1 */
		processorState->s_asid = (index << ASIDMASK);
		processorState->s_status = ALLOFF | IEc | IM | TE;
		/* the init handler */
		processorState->s_t9 = (memaddr) initUProc;
		/* TODO: set up pc */
		processorState->s_sp = (memaddr) initUProc;
	}
	return processorState;
}


/* wrapper function for our phase 3 */
void test() {
	/* here we are */
	debugger(420);
	/* some variables for indexing */
	int i;
	int j;

	/* initalize the swap pool */
	for(i = 0; i < SWAPSIZE; i++) {
		pool[i].pageTableEntry = NULL;
		pool[i].segmentNumber = 0;
		pool[i].pageNumber = 0;
		/* -1 signifies */
		pool[i].ASID = -1;
	}
	debugger(2);
	/* initialize the semaphores */
	for(i = 0; i < MAXSEMALLOC; i++){
		mutexSemaphores[i] = 1;
	}
	debugger(3);

	/* initalize the page table for the kUsegOS */
	for (i = 0; i < KUSEGPTESIZE; i++) {
		/* occupy the EntryHI CP0 register */
		kSegOS.pteTable[i].entryHI = (PADDRBASE + i) << VPNMASK;
		/* occupy the EntryLO CP0 register */
		kSegOS.pteTable[i].entryLO = ((PADDRBASE + i) << VPNMASK) | VALID | DIRTY | GLOBAL;
	} 

	debugger(4);
	/* initialize the header */
	kSegOS.header = MAGICNO << PGTBLHEADERWORD | KSEGOSPTESIZE;
	for(i = 1; i < MAXUPROC + 1; i++) {
		/* get the ith uProc */
		/* Tproc_t userProc = uProcesses[i - 1] */
		/* initialize the header */
		uProcesses[i - 1].Tp_pte.header = ((MAGICNO << PGTBLHEADERWORD) | KUSEGPTESIZE);
		debugger(7);
		/* set up the page table entry */
		for(j = 0; j < KUSEGPTESIZE; j++) {
			/* TODO: set up entryHI */
			uProcesses[i - 1].Tp_pte.pteTable[j].entryHI = (BASEADDR + j) >> VPNMASK | (i << ASIDMASK);
			uProcesses[i - 1].Tp_pte.pteTable[j].entryLO = ALLOFF | DIRTY;
		}
		/* get the address of ith entry the segment table */
		segt_PTR segmentTable = (segt_PTR) SEGSTART + (i * SEGWIDTH);
		/* point to the kSegOS segment */
		segmentTable->kUseg2 = (&(uProcesses[i - 1].Tp_pte));
		segmentTable->kSegOS = (&(kSegOS));
		/* prepare the processor state */
		uProcesses[i - 1].Tp_pte.pteTable[KUSEGPTESIZE-1].entryHI = (BSDGMT  << VPNMASK) | (i << ASIDMASK);
		debugger(8);
		/* add a new processor state, per the student guide */
		state_PTR processorState = prepareProcessorState(FALSE, i);
		debugger(9);
		/* set the semaphore */
		uProcesses[i - 1].Tp_sem = 0;
		int status = SYSCALL(CREATEPROCESS, (int) &(processorState), EMPTY, EMPTY);
		if(status != SUCCESS) {
			SYSCALL(TERMINATEPROCESS, EMPTY, EMPTY, EMPTY);
		}
	}
	debugger(5);
	for(i = 0; i < MAXUPROC; i++) {
		SYSCALL(PASSEREN, (int) &(masterSemaphore), EMPTY, EMPTY);
	}
	debugger(6);
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
int extractASID() {
	/* extracts the asid from the entryHi cp0 register */
	return ((getENTRYHI() & ENTRYHIASID) >> ASIDMASK);
}




/* sets up pass up or die stuff */
static void initializeExceptionsStateVector() {
	/* need a new state */
	state_PTR state = NULL;
	int i;
	/* for each trap type */
	for(i = 0; i < TRAPTYPES; i++) {
		/* get the state by the asid */
		state_PTR state = &(uProcesses[extractASID() -1].Tnew_trap[i]);
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
		SYSCALL(SPECTRAPVEC, i, (int) &(uProcesses[extractASID() - 1].Told_trap[i]), (int) state);
	}
}

	
