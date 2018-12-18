#include "../h/const.h"
#include "../h/types.h"
#ifndef SYSSUPPORT
#define SYSSUPPORT
    void diskOperation(int diskInformation[], int *semaphore, device_PTR diskDevice);
    void mutex(int flag, int *semaphore);
    void terminateUProcess();
    void enableInterrupts();
    void disableInterrupts();
#endif
