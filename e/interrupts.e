#include "../h/const.h"
#include "../h/types.h"
#ifndef INT
#define INT
    extern void copyState(state_PTR from, state_PTR to);
    extern void interruptHandler();
#endif