#ifndef KERNEL_H
#define KERNEL_H

#include <msp430.h>
#include <stdint.h>


/**
 * main.c
 */

typedef struct queue{
    int size;
    volatile uint16_t* pTasksVector;
    int head;
    int tail;
}queue;

typedef struct task{
    volatile uint16_t* pStack;
    volatile uint16_t* pTask;
    volatile uint16_t waitTime;
}task;

void registerTask(void* pFunction);
void startRTOS();
void basicConfig();
void wait(uint16_t tickTime);
void idle();

#endif // KERNEL_H
