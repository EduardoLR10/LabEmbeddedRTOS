#ifndef KERNEL_H
#define KERNEL_H

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum PRIORITY{
    LOW, HIGH,
}PRIORITY;

typedef struct task{
    volatile uint16_t* pStack;
    volatile uint16_t* pTask;
    volatile uint16_t waitTime;
    volatile uint16_t index;
    volatile uint16_t quantum;
    volatile PRIORITY priority;
}task;

#define MAX_TASKS 3
#define N_FIFOS 2
#define IDLE_INDEX 2


void registerTask(void *pFunction, PRIORITY priority, uint16_t quantum);
void startRTOS();
void basicConfig();
void wait(uint16_t tickTime);

// Idle task
void idle();

// Functions related to fifos
void decrementWaitTime();
void putTaskToPriorityFifo(task* t);
void addToBlockedTasks();
void manageBlockedTasks();
void initializeFifo();
bool pickFromLowPriority();
bool pickFromHighPriority();

#endif // KERNEL_H
