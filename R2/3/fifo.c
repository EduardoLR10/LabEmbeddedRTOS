#include "fifo.h"

void createFifo(fifo* q){
    q->size = 0;
    q->head = 0;
    q->tail = 0;
}

void putTaskToFifo(fifo* q, task t){
    q->size++;
    
    if(q->size != 1)
        q->tail = (q->tail + 1) % (MAX_TASKS);

    q->taskVector[q->tail] = t;
}

task getTaskFromFifo(fifo* q){
    q->size--;
    
    task t = q->taskVector[q->head];

    if(q->size != 0)
        q->head = (q->head + 1) % (MAX_TASKS);

    return t;
}
