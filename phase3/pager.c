#include "../h/const.h"
#include "../h/types.h"
/* e files */
#include "../e/exceptions.e"
#include "../e/initProc.e"
#include "../e/sysSupport.e"
/* include the µmps2 library */
#include "/usr/local/include/umps2/umps/libumps.e"

void progTrapHandler() {
    terminateUProcess();
}

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



/* just returns an increment on the last frame mod to create an incremental choice */
static int next() {
    static int lastFrame = 0;
    if (!lastFrame < SWAPSIZE) {
        lastFrame = 0;
    }
    lastFrame++;
    return lastFrame;
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

/* the pager for the TLB exception */
void pager() {
    /* acquire the mutex on the swapool metaphor */
    mutex(TRUE, &(swapSemaphore));
    /* get the current asid */
    int ASID = extractASID();
    /* why are we here */
    /*examine oldmem cause register */
    state_PTR state = &(uProcesses[extractASID() - 1].Told_trap[TLBTRAP]);
    /* the device register */
    devregarea_PTR bus = (devregarea_PTR)RAMBASEADDR;
    memaddr RAMTOP = bus->rambase + bus->ramsize;
    /* start of the first frame */
    memaddr swapPoolStart = ((bus->rambase + bus->ramsize) - (3 * PAGESIZE));
    /* get the cause of the fault */
    int cause = (state->s_cause & 0x3C) >> EXCMASK;
    /* who am I? */
    /* get current processid in ASID register */
    /* this is needed as the index into the phase3 global structure */
    int missingASID = ((getENTRYHI() & 0x3FFFF000) >> ASIDMASK);
    device_PTR diskDevice = (device_PTR)DISKDEV;
    /* get the cause */
    /* if TLB Invalid then SYS18 */
    /* 2 and 3 only valid TLB causes (pg 16 in yellow book) */
    if ((cause != TLBL) && (cause != TLBS)) {
        terminateUProcess();
    }
    /* which page is missing */
    /*oldMem ASID register has segment no and page no */
    /*not sure about 30, will revisit later */
    int segmentNumber = (state->s_asid >> SEGMENTMASK);
    /* i think that 0x3FFFF000 covers the asid but im not sure */
    int pageNumber = ((state->s_asid & 0x3FFFF000) >> VPNMASK);
    /* pick a frame to use */
    int frameNumber = next();
    swapPoolStart = swapPoolStart + (frameNumber * PAGESIZE);

    /* information for the disk */
    int diskInformation[DISKPARAMS];
    diskInformation[HEAD] = EMPTY;
    diskInformation[DISKNUM] = EMPTY;
    diskInformation[PAGELOCATION] = pageNumber;
    /* what type of operation is it? */
    diskInformation[READWRITE] = WRITEBLK;
    /* save out status for interrupts */
    int preservedStatus;
    /* if the frame is currently occupied */
    if(pool[frameNumber].ASID != -1) {
        preservedStatus = getSTATUS();
        setSTATUS(ALLOFF);
        /* turn the valid bit off in the page table of the current frames occupent */
        int squatterASID = pool[frameNumber].ASID - 1;
        int squatterPageNum = pool[frameNumber].pageNumber;
        if(pageNumber >= KUSEGPTESIZE) {
            pageNumber = KUSEGPTESIZE - 1;
        }
        
        /* write current frames contents on the backing store */
        invalidateEntry(frameNumber);
        /* get the information ready for the disk */
        diskInformation[SECTOR] = squatterASID;
        diskInformation[CYLINDER] = squatterPageNum;
        /* reenable the enterrupts */
        setSTATUS(preservedStatus);
        /* perform a disk operation */
        diskOperation(diskInformation, (&(disk0Semaphore)), diskDevice);
        
    }
    /* reset the disk information */
    diskInformation[SECTOR] = ASID - 1;
    diskInformation[CYLINDER] = pageNumber;
    diskInformation[READWRITE] = READBLK;
    /* read missing page into selected frame */
    diskOperation(pageNumber, (&(disk0Semaphore)), diskDevice);


    /*update the swapool data structure */
    pool[frameNumber].ASID = missingASID;
    pool[frameNumber].segmentNumber = segmentNumber;
    pool[frameNumber].pageNumber = pageNumber;
    /* update missing pages page table entry: frame and valid bit */
    if (segmentNumber == 3) {
        pool[frameNumber].pageTableEntry = &(kUseg3.pteTable[pageNumber]);
        pool[frameNumber].pageTableEntry->entryLO = swapPoolStart | VALID | DIRTY | GLOBAL;
    } else {
        pool[frameNumber].pageTableEntry = &(uProcesses[missingASID].Tp_pte.pteTable[pageNumber]);
        pool[frameNumber].pageTableEntry->entryLO = (swapPoolStart & LOCAL) | VALID | DIRTY | GLOBAL;
    }
    /* deal with the cache consitency */
    TLBCLR();
    
    /*release mutex and return control to process */
    mutex(FALSE, (&(swapSemaphore)));
    
    contextSwitch(state);
}





