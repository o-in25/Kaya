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
            diskPut();
            break;
        case DISK_GET:
            diskGet();
            break;
        case WRITE_TO_PRINTER:
            writeToPrinter(state);
            break;
        case GET_TOD:
            getTOD();
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
    while (!done) {
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
        address++
    }
    
    /* put the ammount that were  written into v0 */
    state->s_v0 = total;
    
    /* RELEASE THE KRAKEN...by which i mean release the mutex */
    SYSCALL (VERHOGEN, (int)&mutexSemaphores[32 + (ASID - 1)], 0, 0);
}

static void writeToTerminal(state_PTR state) {

}

static void vVerhogen() {

}

static void vPasseren() {

}

static void delay() {

}

static void diskPut() {

}

static void diskGet() {

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

static void getTOD() {

}

static void terminateUProcess() {

}


/* used to gain mutual exclusion on a passed in semaphore or 
release mutal exclusion so that a process can be done atomically. Here,
TRUE is to gain mutal exclusion and FALSE is to release it */
void mutex(int flag, int *semaphore) {
    /* are we gaining control? */
    if(flag) {
        SYSCALL(PASSEREN, (int) semaphore, EMPTY, EMPTY);
    } else {
        /* no - we are releasing it */
        SYSCALL(VERHOGEN, (int) semaphore, EMPTY, EMPTY);
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

