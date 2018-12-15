#include "../h/const.h"
#include "../h/types.h"

#include "../e/initProc.e"

/* disables interrupts on request */
void disableInterrupts() {
    int status = getSTATUS();
    status = (status & 0xFFFFFFFE);
    setSTATUS(status);
}

/* enables interrupts on request */
void enableInterrupts() {
    int status = getSTATUS();
    status = (status & 0x1);
    setSTATUS(status);
}

/* readBacking reads in data from the backing store using the givin cylinder, sector, head and address to write to */
void readBacking (int cylinder, int sector, int head, memaddr address){
    SYSCALL(PASSEREN, (int)&disk0Semaphore, 0, 0);
    
    devregarea_t* devReg = (devregarea_t *) RAMBASEADDR;
    device_t* disk = &(devReg->devreg[0]);
    unsigned int status;
    
    disableInterrupts();
    disk->d_command = (cylinder << 8) | 2;
    status = SYSCALL(WAITIO, DISKINT, BACKINGSTORE, 0);
    enableInterrupts();
    
    if (status == 1){
        disableInterrupts();
        disk->d_data0 = address;
        disk->d_command = (head << 16) | ((sector - 1) << 8) |  4;
        
        status = SYSCALL(WAITFORIO, DISKINT, BACKINGSTORE, 0);
        enableInterrupts();
    }
    
    SYSCALL(VERHOGEN, (int) &(disk0Semaphore), 0, 0);
}

/* write backing writes to the backing store given a cylinder, sector, head and memory address to write from */
void writeBacking (int cylinder, int sector, int head, memaddr address){
    SYSCALL(PASSEREN, (int)&disk0Semaphore, 0, 0);

    devregarea_PTR devReg = (devregarea_PTR) RAMBASEADDR;
    device_t* disk = &(devReg->devreg[0]);
    unsigned int status;
    
    disableInterrupts();
    disk->d_command = (cylinder << 8) | 2;
    status = SYSCALL (WAITIO, DISKINT, BACKINGSTORE, 0);
    enableInterrupts();
    
    if (status == 1){
        disableInterrupts();
        disk->d_data0 = address;
        disk->d_command = (head << 16) | ((sector - 1) << 8) | 3;
        
        status = SYSCALL (WAITFORIO, DISKINT, BACKINGSTORE, 0);
        enableInterrupts();
    }
    
    SYSCALL VERHOGEN, (int)&disk0Semaphore, 0, 0);
}

void pager() {
    
    
}


void test() {
    
}






int lastFrame;

void progTrapHandler () {
    /* SYS18 */
}


/* just returns an increment on the last frame mod to create an incremental choice */
int nextFrame() {
    if (!lastFrame){
        lastFrame = 0;
    }
    lastFrame = lastFrame + 1;
    return (lastFrame % (16));
}

void TLBhandler (){
    devregarea_PTR devReg = (devregarea_PTR)RAMBASEADDR;
    memaddr RAMTOP = devReg->rambase + devReg->ramsize;
    memaddr swappoolstart = RAMTOP - (2 * PAGESIZE) - (SWAPPOOLSIZE * PAGESIZE);
    
    /* who am I? */
    /* get current processid in ASID register */
    /* this is needed as the index into the phase3 global structure */
    int missing_ASID = ((getENTRYHI() & 0x3FFFF000) >> ASIDMASK);
    
    /* why are we here */
    /*examine oldmem cause register */
    state_PTR oldState = (state_PTR) &(uProcesses[missing_ASID - 1].Told_trap[2]);
    int cause = (oldState->s_cause & 0x3C) >> 2;
    /* if TLB Invalid then SYS18 */
    /* 2 and 3 only valid TLB causes (pg 16 in yellow book) */
    if ((cause != 2) && (cause != 3)){
        /* sys 18 */
    }
    /* which page is missing */
    /*oldMem ASID register has segment no and page no */
    /*not sure about 30, will revisit later */
    int segmentNumber = (oldState->s_asid >> 30);
    /* i think that 0x3FFFF000 covers the asid but im not sure */
    int pageNumber = ((oldState->s_asid & 0x3FFFF000) >> 12);
    /* acquire the mutex on the swapool metaphor */
    SYSCALL (PASSEREN, (int) &swapSemaphore, 0, 0);
    
    /* if missing page (mp) is from KUseg3, check if the page is still missing */
    /* check the KUseg3 page table entry's valid bit */
    if (segmentNumber == 3 && /*need to check if valid bit on */) != 0){
        /* if no longer missing, release mutex and return control */
        SYSCALL (VERHOGEN, (int) &swapSemaphore, 0, 0);
        LDST (oldState);
    }
    
    /* pick a frame to use */
    int frame = nextFrame ();
    memaddr address = SWAPPOOLSTART + (frame * PAGESIZE);
    
    /* if the frame is currently occupied */
    if (pool[frame].ASID != -1){
        
        disableInterrupts();
        /* turn the valid bit off in the page table of the current frames occupent */
        pool[frame].pageTableEntry->entryLO = pool[frame].pageTableEntry->entryLO & INVALID;
        /* deal with the TLB cache consistency */
        /* by clearing the TLB */
        TLBCLR ();
        enableInterrupts();
        
        int squatterASID = pool[frame].ASID;
        int squatterPageNum = pool[frame].pageNumber;
        /* write current frames contents on the backing store */
        writeBacking (squatterPageNum, squatterASID, 0, address);
        
    }
    
    /* read missing page into selected frame */
    readBacking(pageNumber, missing_ASID, 0, address);
    disableInterrupts();
    /*update the swapool data structure */
    pool[frame].ASID = missing_ASID;
    pool[frame].segmentNumber = segmentNumber;
    pool[frame].pageNumber = pageNumber;
    /* update missing pages page table entry: frame and valid bit */
    if (segmentNumber == 3) {
        pool[frame].pageTableEntry = &(KUseg3.pteTable[pageNumber]);
        pool[frame].pageTableEntry->entryLO = address | VALID | DIRTY |GLOBAL;
    } else {
        pool[frame].pageTableEntry = &(uProcesses[missing_ASID].tp_pte.pteTable[pageNumber]);
        pool[frame].pageTableEntry->entryLO = address | VALID | DIRTY |GLOBAL;
    }
    /* deal with the cache consitency */
    TLBCLR();
    enableInterrupts();
    
    /*release mutex and return control to process */
    SYSCALL(VERHOGEN, (int)&swapSemaphore, 0, 0);
    
    LDST(oldState);
}





