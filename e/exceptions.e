#include "../h/const.h"
#include "../h/types.h"
#ifndef EXCEPTIONS
#define EXCEPTIONS
    extern void syscallHandler();
    extern void programTrapHandler();
    extern void translationLookasideBufferHandler();
    extern void contextSwitch(state_PTR s);
    extern void copyState(state_PTR from, state_PTR to)
#endif