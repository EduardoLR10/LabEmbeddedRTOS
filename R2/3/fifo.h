#ifndef FIFO_H
#define FIFO_H

#include "kernel.h"

typedef struct fifo{
    volatile int size;
    task taskVector[MAX_TASKS];
    volatile int head;
    volatile int tail;
}fifo;

void createFifo(fifo* q);
void putTaskToFifo(fifo* q, task t);
task getTaskFromFifo(fifo* q);

#endif // FIFO_H
