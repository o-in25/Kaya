/*************************************************** interrupts.c ************************************************************
	Interrupts.c is the I/O handler. In the I/O handler, the following steps are taken: first, the
    interrupt line that is on is determined (for convience and organization, the devices 3-7 are stored
    as constants in constants.h). Next, with the line number, the device instance must be determined, 
    and the address of that device's regsiter must be acquired.  

***************************************************** interrupts.c ************************************************************/

/* h files to include */
#include "../h/const.h"
#include "../h/types.h"
/* e files to include */
#include "../e/pcb.e"
#include "../e/asl.e"

/************************************************************************************************************************/
/******************************************** HELPER FUNCTIONS  *********************************************************/
/************************************************************************************************************************/

/*
* Function: Get Device Number 
* The interrupting devices bit map IDBM is a read-only 5 word 
* area that indicates which devices have an interrupt pending.
* When bit i in word j is equal to 1, the device associated with
* that corresponding bit has an interrupt pending. Since the word
* will be supplied prior to this function call, this will simply 
* calculate the device number  
*/
static void getDeviceNumber(int lineNumber) {
    /* get the address of the device bit map. Per secion 5.2.4 of pops, the 
    physical address of the bit map is 0x1000003C. When bit i is in word j is 
    set to one then device i attached to interrupt line j + 3 */
    const unsigned int* deviceBitMap = (memaddr) DEVREGLEN + (lineNumber - NOSEM) * WORDLEN;
    /* start at the first device */
    unsigned int candidate = STARTDEVICE;
    int i = 0;
    /* search each 8 bits */
    for(i; i < STARTDEVICE; i++) {
        /* if the bit i in word j is set to 1, then
        the device attached to interrupt j + 3 has a pending 
        interrupt */
        if((candidate & (*deviceBitMap) == candidate)) {
            /* since this index is equal to 1, we found it */
            return i;
        } else {
            /* bitwise shift right and go to the next one */
            candidate = candidate << 1;
        }
    }

}

static void getLineNumber(int cause) {
int lineNumbers[LINECOUNT - 2] = {
        LINETHREE,
        LINEFOUR,
        LINEFIVE,
        LINESIX,
        LINESEVEN
    };
    int i; 
    for(i = 2; i < LINECOUNT; i++) {
        if(cause & lineNumbers[i] != 0) {
            /* found the line number */
                return lineNumbers[i];
        }
    }
}


void interruptHandler() {
    /* the old interrupt */
    state_PTR oldInterupt = (state_PTR) INTRUPTOLDAREA;
    const unsigned int cause = oldInterupt->s_cause;
    int deviceNumber;
    int lineNumber;
    if((cause & LINEONE) != 0) {
        /* skip for now */
    } else if((cause & LINETWO) != 0) {
        /* skip for now */
    } else {
        lineNumber = findLineNumber(cause);
    }
    /* line number found, now find the corresponding device 
    number */


}