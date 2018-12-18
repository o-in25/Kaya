#include "../h/const.h"
#include "../h/types.h"
#ifndef PAGER
#define PAGER
    void invalidateEntry(int frameNumber);
    void enableInterrupts();
    void disableInterrupts();
#endif