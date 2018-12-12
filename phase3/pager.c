
#include "../h/const.h"
#include "../h/types.h"
#include "../e/initProc.e"
void test() {
    
}

int lastFrame;

void progTrapHandler (){
    /* SYS18 */
}

/* just returns an increment on the last frame mod to simulate an incremental choice */
int nextFrame (){
    if (!lastFrame){
        lastFrame = 0;
    }
    lastFrame = lastFrame + 1;
    return (lastFrame % (8*2));
}

void TLBhandler (){
    devregarea_PTR devReg = (devregarea_PTR)RAMBASEADDR;
    memaddr RAMTOP = devReg->rambase + devReg->ramsize;
    memaddr swappoolstart = RAMTOP - (2 * PAGESIZE) - (SWAPSIZE * PAGESIZE);
    
    /* who am I? */
    /* get current processid in ASID register */
    /* this is needed as the index into the phase3 global structure */
    int missing_ASID = ((getENTRYHI() & 0) >> ASIDMASK);
    
    /* why are we here */
    /*examine oldmem cause register */
    state_PTR oldState = (state_PTR) & (uProcesses[missing_ASID - 1] /*however we do old trap area for tlb */);
    int cause = (oldState->s_cause & 0 /* bits 2-6 */) >> 2;
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
    /* check the KUseg3 page table enty's valid bit */
    if (/*determine if it is from kusegos and if it is now valid, if both are true then you execute the following */){
        /* if no longer missing, release mutex and return control */
        /* LDST oldMem */
        SYSCALL (VERHOGEN, (int) &swapSemaphore, 0, 0);
        LDST (oldState);
    }
    
    /* pick a frame to use */
    int frame = nextFrame ();
    memaddr address = SWAPPOOLSTART + (frameNumber * PAGESIZE);
    
    /* if the frame is currently occupied */
    if (pool[frame].ASID != -1){
        
        /* disable interupts */
        
        /* turn the valid bit off in the page table of the current frames occupent */
        pool[frame].pageTableEntry->entryLO = pool[frame].pageTableEntry->entryLO & INVALID;
        /* deal with the TLB cache consistency */
        /* by clearing the TLB */
        TLBCLR ();
        /* enable interupts */
    }
    /* write current frames contents on the backing store */
    /* read missing page into selected frame */
    /*update the swapool data structure */
    /* update missing pages page table entry: frame and valid bit */
    /* deal with the cache consitency */
    
    /*release mutex and return control to process */
}





