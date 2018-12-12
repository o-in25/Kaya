#include "../h/const.h"
#include "../h/types.h"

/* read in the uproc's .data and .text from the tape */
void diskOperation(int diskInformation[], int *semaphore, device_PTR diskDevice) {
    /* initialize the disk with the disk number */
    diskDevice = (device_PTR) diskDevice + (diskInformation[DISKNUM] * DEVREGSIZE);
    /* gain control */
    mutex(TRUE, semaphore);
    /* save the status before we turn everything off */
    int oldStatus = getSTATUS();
    /* turn off interrupts */
    setSTATUS(ALLOFF);
    diskDevice->d_command = ALLOFF;
    int status = SYSCALL(WAITIO, DISKINT, diskInformation[DISKNUM], EMPTY);
    /* return to how we were */
    setSTATUS(oldStatus);
    /* if we aren't ready, it's over */
    if(status != READY) {
        SYSCALL(TERMINATEPROCESS, EMPTY, EMPTY, EMPTY);
    }
    /* we are ready, time to write */
    diskDevice->d_data0 = diskInformation[PAGELOCATION];

    /* release control */
    mutex(FALSE, semaphore);
}

/* used to gain mutual exclusion on a passed in semaphore or 
release mutal exclusion so that a process can be done atomically. Here,
TRUE is to gain mutal exclusion and FALSE is to release it */
void mutex(int flag, int *semaphore) {
    /* are we gaining control? */
    if (flag) {
        SYSCALL(PASSEREN, (int)semaphore, EMPTY, EMPTY);
    } else {
        /* no - we are releasing it */
        SYSCALL(VERHOGEN, (int)semaphore, EMPTY, EMPTY);
    }
}