void test() {
    
}


void TLBhandler (){
    
    memaddr address;
    devregarea_t* devReg = (devregarea_t*) RAMBASEADDR;
    memaddr RAMTOP = devReg->rambase + devReg->ramsize;
    memaddr swappoolstart = RAMTOP - (2 * PAGESIZE) - (SWAPSIZE * PAGESIZE);
    
    /* who am I? */
    /* get current processid in ASID register */
    /* this is needed as the index into the phase3 global structure */
    int missing_ASID = ((getENTRYHI() & /*whereever the bits are for asid */) >> ASIDMASK)
    
    /* why are we here */
    /*examine oldmem cause register */
    state_t* oldState = (state_t*) &(uProcesses[missing_ASID - 1]./*however we do olt trap area for tlb */)
    int cause = (oldState->s_cause & /* bits 2-6 */) >> 2;
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
    int pageNumber =((oldstate->s_asid & 0x3FFFF000) >> 12);
    /* acquire the mutex on the swapool metaphor */
    SYSCALL (PASSEREN, (int) &swapSemaphore, 0, 0);
    
    /* if missing page (mp) is from KUseg3, check if the page is still missing */
    /* check the KUseg3 page table enty's valid bit */
    
    /* if no longer missing, release mutex and return control */
    /* LDST oldMem */
    
    /* pick a frame to use */
    /* if the frame is currently occupied */
    /* turn the valid bit off in the page table of the current frames occupent */
    /* deal with the TLB cache consistency */
    /* write current frames contents on the backing store */
    /* read missing page into selected frame */
    /*update the swapool data structure */
    /* update missing pages page table entry: frame and valid bit */
    /* deal with the cache consitency */
    
    /*release mutex and return control to process */
}





