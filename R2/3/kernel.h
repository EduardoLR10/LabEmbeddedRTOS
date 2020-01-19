#ifndef KERNEL_H
#define KERNEL_H

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct task{
    volatile uint16_t* pStack;
    volatile uint16_t* pTask;
    volatile uint16_t waitTime;
    volatile int index;
}task;

typedef struct queue{
    volatile int size;
    volatile task** qTaskVector;
    volatile int head;
    volatile int tail;
}queue;

void manageTasksToQueues();
bool checkAvailableWaitTimes(queue* q);
void registerTask(void* pFunction);
void manageTasksToQueues();
void createQueue(queue* q);
void addTaskToQueue(queue* q, task t);
void removeTaskFromQueue(queue* q);
void initializeQueues();
void startRTOS();
void basicConfig();
void wait(uint16_t tickTime);
void idle();

#endif // KERNEL_H
