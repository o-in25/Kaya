#include "../h/const.h"
#include "../h/types.h"



void uSyscallHandler() {
    state_PTR state = NULL;
    delegateUSyscall(state);
}

static void delegateUSyscall(state_PTR state) {
    switch(state->s_a0) {

    }
}


static void readFromTerminal() {

}

static void writeToTerminal() {

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

static void writeToPrinter() {

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
        SYSCALL(PASSEREN, (int)semaphore, EMPTY, EMPTY);
    } else {
        /* no - we are releasing it */
        SYSCALL(VERHOGEN, (int)semaphore, EMPTY, EMPTY);
    }
} 

/* read in the uproc's .data and .text from the tape */
void diskOperation(int diskInformation[], int *semaphore, device_PTR diskDevice) {
    /* initialize the disk with the disk number */
    diskDevice = (device_PTR) diskDevice + (diskInformation[DISKNUM] * DEVREGSIZE);
    /* save the status before we turn everything off */
    int oldStatus = getSTATUS();
    /* gain control */
    mutex(TRUE, semaphore);
    /* turn off interrupts */
    setSTATUS(ALLOFF);
    /* the command for the disk operation to find the specified cylinder, per 5.3 of pops */
    diskDevice->d_command = ((diskInformation[CYLINDER] << COMMANDMASK) | SEEKCYL);
    int status = SYSCALL(WAITIO, DISKINT, diskInformation[DISKNUM], EMPTY);
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
    diskDevice->d_command = ((diskInformation[HEAD] << COMMANDMASK) | diskInformation[SECTOR]) | diskInformation[READWRITE];
    /* wait for the I/O while we have mutex */
    status = SYSCALL(WAITIO, DISKINT, diskInformation[DISKNUM], EMPTY);
    setSTATUS(oldStatus);
    /* are we ready? */
    if (status != READY) {
        SYSCALL(TERMINATEPROCESS, EMPTY, EMPTY, EMPTY);
    }
    /* release control */
    mutex(FALSE, semaphore);
}

