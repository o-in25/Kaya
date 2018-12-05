#include "../h/const.h"
#include "../h/types.h"
#include "../e/pager.e"

/* The master semaphore */
int masterSemaphore = 0;
/* One semaphore for each device */
int mutexSemaphores[MAXSEMALLOC];




void test() {
    
}