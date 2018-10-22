#include "../h/const.h"
#include "../h/types.h"

 void syscallHandler() {
    int callNumber = 0; /* TODO: properly assign the number and handle case  */
    switch(callNumber) {
    case WAITFORIODEVICE:
        return NULL;
    case WAITFORCLOCK:
        return NULL;
    case GETCPUTIME:
        return NULL;
    case SPECIFYEXCEPTIONSTATEVECTOR:
        return NULL;
    case PASSEREN:
        return NULL;
    case VERHOGEN:
        return NULL;
    case TERMINATEPROCESS:
        return NULL;
    case CREATEPROCESS:
        return NULL;
    }
 }


 void programTrapHandler() {

 }

 void tableHandler() {

 }

 