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

void interruptHandler() {
    /* the old interrupt */
    state_PTR oldInterupt = (state_PTR) INTRUPTOLDAREA;
    const unsigned int cause = oldInterupt->s_cause;
    int deviceNumber;
    int lineNumber;

    if((cause & LINEONE) != 0) {
        
    } else if((cause & LINETWO) != 0) {

    } else if((cause & LINETHREE) != 0) {

    } else if((cause & LINEFOUR) != 0) {

    } else if((cause & LINEFIVE) != 0) {

    } else if((cause & LINESIX) != 0) {

    } else if((cause & LINESEVEN) != 0) {

    } else {
        
    }

}