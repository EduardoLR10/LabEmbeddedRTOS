#include <msp430.h>


/**
 * main.c
 */

typedef struct task{
    short int* pStack;
    short int* pTask;
}task;

const short int stackStart = 0x2802;
task taskVector[2];
int currentIndex = 0;
short int* pStackScheduler;

void registerTask(short int* pFunction){

    asm("MOV.W SP,R7");

    taskVector[currentIndex].pTask = pFunction;

    // Finding stack's address;
    volatile int stackBaseAddress = (stackStart - (0x80*currentIndex));

    // Move stack pointer to the beginning of each task's stack
    asm("MOV.W %0,SP" : "=m" (stackBaseAddress));

    // Pushing task's address
    asm("PUSH.W %0" : "=m" (taskVector[currentIndex].pTask));

    // Pushing SR
    asm("PUSH.W #8");

    // Pushing registers
    asm("PUSHM.W #12,R15");

    // Saving stack pointer in task's stack pointer
    asm("MOV.W SP,%0" : "=m" (taskVector[currentIndex].pStack) );

    currentIndex++;

    asm("MOV.W R7,SP");
}
