#include "../h/const.h"
#include "../h/types.h"
#ifndef EXCEPT
#define EXCEPT
    extern void syscallHandler();
    extern void programTrapHandler();
    extern void tableHandler();
#endif