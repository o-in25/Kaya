/* h files */
#include "../h/const.h"
#include "../h/types.h"
/* e files */
#include "../e/initProc.e"
#include "../e/exceptions.e"
#include "../e/initProc.e"

void uSyscallHandler() {
    state_PTR state = (&((uProcesses[getASID()-1]).Told_trap[SYSTRAP]));
    delegateUSyscall(state);
}

static void delegateUSyscall(state_PTR state) {
    switch(state->s_a0) {
        case READ_FROM_TERMINAL: 
            readFromTerminal(state);
            break;
        case WRITE_TO_TERMINAL:
            writeToTerminal(state);
            break;
        case V_VIRTUAL_SEMAPHORE:
            vVerhogen();
            break;
        case P_VIRTUAL_SEMAPHORE:
            vPasseren();
            break;
        case DELAY:
            delay();
            break;
        case DISK_PUT:
            diskPut(state);
            break;
        case DISK_GET:
            diskGet(state);
            break;
        case WRITE_TO_PRINTER:
            writeToPrinter(state);
            break;
        case GET_TOD:
            getTOD(state);
            break;
        case TERMINATE:
            terminateUProcess();
            break;
    }
}

static void readFromTerminal(state_PTR state) {
    /*int asidIndex = (getSTATUS() - 1);
    device_PTR printerDevice = (device_PTR) PRINTERDEV + (asidIndex * DEVREGSIZE); */
    
    char* address = state->s_a1;
    int ASID = ((getENTRYHI() & 0x19DEBB4FC0) >> ASIDMASK);
    /* call dibs */
    SYSCALL(PASSEREN, (int)&mutexSemaphores[32 + (ASID -1)], 0, 0);
    int done = FALSE;
    unsigned int status;
    /* find that pesky terminal */
    state_PTR oldState = (state_PTR) &uProcesses[ASID - 1].Told_trap[2];
    devregarea_PTR devReg = (devregarea_PTR) RAMBASEADDR;
    int deviceNumber = 32 + (ASID - 1);
    device_PTR terminal = &(devReg->devreg[deviceNumber]);
    int total = 0;
    
    /* loop for reading */
    while(!done) {
        /* tell the machine what to do and then tell it to do it */
        disableInterrupts ();
        /*tell the machine to read from the terminal */
        terminal->t_recv_command = 2;
        /*then tell it to do it */
        status = SYSCALL(WAITIO, TERMINT, (ASID -1), 1);
        enableInterrupts ();
        
        /* check if we are done */
        if (((status & 0x0FF00) >> 8) == (0xA)){
            done = TRUE;
        } else {
            /* set the character */
            *address = ((status &0xFF00) >> 8);
            total++;
        }
        
        /* did it work? */
        if ((status & 0xFF) != 5){
            PANIC();
        }
        address++;
    }
    
    /* put the ammount that were  written into v0 */
    state->s_v0 = total;
    
    /* RELEASE THE KRAKEN...by which i mean release the mutex */
    SYSCALL (VERHOGEN, (int)&mutexSemaphores[32 + (ASID - 1)], 0, 0);
}

static void writeToTerminal(state_PTR state) {
    char *address = state->s_a1;
    char *length = state->s_a2;
    int ASID = ((getENTRYHI() & 0x00000FC0) >> ASIDMASK);
    int deviceNumber = 32 + (ASID - 1);
    devregarea_PTR devReg = (devregarea_PTR) RAMBASEADDR;
    device_PTR terminal = &(devReg->devreg[deviceNumber]);
    
    /* call dibs */
    SYSCALL (PASSEREN, (int)&mutexSemaphores[40 + (ASID - 1)], 0, 0);
    
    unsigned int status;
    /* loop to write the string */
    int i = 0;
    while (i < length){
        
        disableInterrupts();
        /* set the command and call a wait for io */
        terminal->t_transm_command = 2 | (((unsigned int) *address) << 8);
        status = SYSCALL(WAITIO, 7, (ASID - 1), 0);
        enableInterrupts();
        
        /* check for error */
        if ((status & 0xFF) != 5){
            PANIC();
        }
        
        /* set up for the next character */
        address++;
        i++;
    }
    
    /* return the mutex */
    SYSCALL (VERHOGEN, (int)mutexSemaphores[40 + (ASID - 1)], 0, 0);
}

static void vVerhogen() {
    /* dont think we need this one for now */
}

static void vPasseren() {
    /* dont think we need this one for now */
}

static void delay() {
    /* dont think we need this one for now */
}

static void diskPut(state_PTR state) {
    messWithDisk(state, 4)
}

static void diskGet(state_PTR state) {
    messWithDisk (state, 3)
}

/* I noticed the two disk operations were super similair so I combined them into one */
/* since write and read commands are set to 3 and 4, those are used here as well to */
/* which command is desired */
static void diskStuff (state_PTR state, int readOrWrite){
    int *address = (int*) state->s_a1;
    int disk = state->s_a2;
    int sectNumber = state->s_a3;
    ASID = getASID();
    int *buffer = (int*)(0x2001E000 + (disk * 4096));
    state_PTR oldState = (state_PTR) &uProcs[ASID-1].Told_trap[2];
    devregarea_PTR devReg = (devregarea_PTR) RAMBASEADDR;
    device_PTR device = &(devReg->devreg[disk]);
    
    /* disk0 and ksegos are off limits */
    if (disk <= 0 || (memaddr) address < 0x80000000) {
        /* its a trap */
        terminateUProcess ();
    }
    
    /* need the head, sector and cylinder for the data */
    int head = sectNumber % 2;
    sectNumber = sectNumber/2;
    int sectorNumber = sectnumber % 8; /* sectNumber and sectorNumber can be easy to mix up but the way to think about it is that the shorter one is the one we dont use for real */
    sectNumber = sectNumber/8;
    int cylinder = sectNo;
    
    /* call dibs */
    SYSCALL(PASSEREN, (int)&mutexSemaphores[disk], 0, 0);
    
    if (readWrite == 4 /*command for write */){
        for (int i = 0; i < 1024; i++){
            /*copy it and... */
            *address = *buffer;
            
            /* move on! */
            buffer++;
            source++;
            i++;
        }
    }
    
    /*perform an atomic operation and seek to correct the cylinder */
    disableInterrupts();
    
    device->d_command = ((cylinder << 8) | 2);
    int status = SYSCALL(WAITIO, 3, disk, 0);
    
    enableInterrupts();
    
    /* if successful */
    if (status == 1){
        disableInterrupts();
        
        /* set up the data location and read/write command */
        device->d_data0 = (memaddr) buffer;
        device->d_command = (head << 16) | (sectorNumber << 8) | readOrWrite;
        
        status = SYSCALL(WAITIO, 3, disk, 0);
        enableInterrupts();
    }
    
    if (readOrWrite == 3/*read command*/){
        for (int i = 0; i < 1024; i++){
            /*copy it and... */
            *address = *buffer;
            
            /* move on! */
            buffer++;
            source++;
            i++;
        }
    }
    
    /* store the result in v0 */
    state->s_v0 = status;
    
    /* drop mic on stage */
    SYSCALL(VERHOGEN, (int)&mutexSemaphores[disk], 0, 0);
    
}

static void writeToPrinter(state_PTR state) {
    char* nextChar = (char*) state->s_a1;
    int stringLength = (int) state->s_a2;
    int asidIndex = getASID() - 1;
    /* get the device */
    device_PTR printerDevice = (device_PTR) PRINTERDEV + (asidIndex * DEVREGSIZE);
    int i = 0;
    int status;
    while(i < stringLength && i > 0) {
        printerDevice->d_command = PRINTCHR;
        printerDevice->d_data0 = nextChar[i];
        status = SYSCALL(WAITIO, PRNTINT, asidIndex, EMPTY);
        if(status == READY) {
            i++;
            continue;
        }
        /* the device is not ready, return the negative value */
        i = i - (2 * i);
    }
    state->s_v0 = i;
    contextSwitch(state);

}

static void getTOD(state_PTR state) {
    /* seems too easy, please double check my work */
    cpu_t TOD;
    STCK (TOD);
    state->s_v0 = TOD;
    LDST (state);
}

static void terminateUProcess() {
    int ASID = ((getENTRYHI() & 0x00000FC0) >> ASIDMASK);
    
    /* call dibs */
    SYSCALL(PASSEREN, (int)&swapSemaphore, 0, 0);
    
    disableInterrupts();
    
    /* set page table and the swap pool entries to invalid */
    int touched = FALSE; /* used to know if we have to clear the tlb or not */
    for (int i = 0; i < 16; i++){
        if(pool[i].ASID == ASID){
            pool[i].pageTableEntry->entryLO = (pool[i].pageTableEntry->entryLO | INVALID);
            pool[i].ASID = -1;
            touched = TRUE;
        }
    }
    if (touched) { /* we handle any problems with the tlb in the most elegant of ways */
        TLBCLR();
    }
    enableInterrupts ();
    
    /* we no longer need the semaphore */
    SYSCALL(VERHOGEN, (int)&swapSemaphore, 0, 0);
    SYSCALL(VERHOGEN, (int)&masterSemaphore, 0, 0);
    
    /* and to finish it off we add a dash of genocide */
    SYSCALL (TERMINATEPROCESS, 0, 0, 0);
}



/* used to gain mutual exclusion on a passed in semaphore or 
release mutal exclusion so that a process can be done atomically. Here,
TRUE is to gain mutal exclusion and FALSE is to release it */
void mutex(int flag, int *semaphore) {
    /* are we gaining control? */
    if(flag) {
        SYSCALL(PASSEREN, (int) semaphore, 0, 0);
    } else {
        /* no - we are releasing it */
        SYSCALL(VERHOGEN, (int) semaphore, 0, 0);
    }
} 

/* read in the uproc's .data and .text from the tape */
void diskOperation(int* diskInformation, int *semaphore, device_PTR diskDevice) {
    /* initialize the disk with the disk number */
    diskDevice = (device_PTR) diskDevice + ((*(diskInformation + DISKNUM)) * DEVREGSIZE);
    /* save the status before we turn everything off */
    int oldStatus = getSTATUS();
    /* gain control */
    mutex(TRUE, semaphore);
    /* turn off interrupts */
    setSTATUS(ALLOFF);
    /* the command for the disk operation to find the specified cylinder, per 5.3 of pops */
    diskDevice->d_command = (( (*(diskInformation + CYLINDER)) << COMMANDMASK) | SEEKCYL);
    int status = SYSCALL(WAITIO, DISKINT, (*(diskInformation + DISKNUM)), EMPTY);
    /* return to how we were */
    setSTATUS(oldStatus);
    /* if we aren't ready, it's over */
    if(status != READY) {
        SYSCALL(TERMINATEPROCESS, EMPTY, EMPTY, EMPTY);
    }
    /* set the data we wish to write */
    diskDevice->d_data0 = diskInformation[PAGELOCATION];
    /* done, now we turn back off interrupts */
    oldStatus = getSTATUS();
    setSTATUS(ALLOFF);
    /* we are ready, time to write */
    /* write the disk */
    diskDevice->d_command = (((*(diskInformation + HEAD)) << COMMANDMASK) | (*(diskInformation + SECTOR))) | (*(diskInformation + READWRITE));
    /* wait for the I/O while we have mutex */
    status = SYSCALL(WAITIO, DISKINT, (*(diskInformation = DISKNUM)), EMPTY);
    setSTATUS(oldStatus);
    /* are we ready? */
    if (status != READY) {
        SYSCALL(TERMINATEPROCESS, EMPTY, EMPTY, EMPTY);
    }
    /* release control */
    mutex(FALSE, semaphore);
}

