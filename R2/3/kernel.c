#include "kernel.h"

#define MAX_TASKS 3
#define IDLE_INDEX 2

const short int stackStart = 0x2802;
const short int stackSize = 0x80;
task taskVector[MAX_TASKS];
int currentIndex = 0;

uint16_t* pStackScheduler = 0x2500;

queue highPriorityQueue;
queue lowPriorityQueue;
queue sleepyTasksQueue;

void manageTasksToQueues(){
    addTaskToQueue(&highPriorityQueue, taskVector[0]);
    addTaskToQueue(&lowPriorityQueue, taskVector[1]);
    addTaskToQueue(&lowPriorityQueue, taskVector[2]);
}

// Compiler optimizes operations (WTF), removing operation add 24 to pStack
#pragma GCC optimize ("0")
void startRTOS(){

    registerTask(idle);

    manageTasksToQueues();

    WDTCTL = (WDTPW | WDTSSEL__ACLK | WDTIS_7 | WDTTMSEL_1 | WDTCNTCL);
    SFRIE1 = WDTIE;

    currentIndex = 0;

    // Move first's stack pointer to initial position
    taskVector[0].pStack += 24;

    volatile uint16_t aux = taskVector[0].pStack + 2;

    // Initialize stack pointer to first task
    asm("MOVX.W %0,SP" : "=m" (aux));
    asm("MOVX.A #8,SR");
    asm("MOVX.A %0,PC" : "=m" (taskVector[0].pTask));

    return;
}

void initializeQueues(){
    createQueue(&highPriorityQueue);
    createQueue(&lowPriorityQueue);
    createQueue(&sleepyTasksQueue);
}

void basicConfig(){

    WDTCTL = WDTPW | WDTHOLD;

    PM5CTL0 &= ~BIT0;

    __enable_interrupt();

}

void registerTask(void *pFunction){

    // Initialize waitTime
    taskVector[currentIndex].waitTime = 0;

    // Put index inside task
    taskVector[currentIndex].index = currentIndex;

    // Save address to function
    taskVector[currentIndex].pTask = pFunction;

    // Manipulate fuction's address, saving most significant bits
    int msbPC = ((int)pFunction >> 4);

    // Finding stack's address;
    uint16_t* stackBaseAddress = (stackStart - (stackSize*currentIndex));

    // Move pointer
    stackBaseAddress--;

    // Save start of function in it's stack
    *stackBaseAddress = pFunction;

    // Move pointer
    stackBaseAddress--;

    // Save SR value in function's stack
    *stackBaseAddress = ((msbPC & 0x0000F000) | 0x008);

    // Pushing registers with zero
    int i = 24;
    while(i){
        *(--stackBaseAddress) = 0;
        i--;
    }

    // Saving stack pointer in task's stack pointer
    taskVector[currentIndex].pStack = stackBaseAddress;

    currentIndex++;
}

void createQueue(queue* q){
    q->qTaskVector = (task**) malloc(sizeof(task*) * MAX_TASKS);
    q->size = 0;
    q->head = 0;
    q->tail = -1;
}

void addTaskToQueue(queue* q, task t){
    q->size++;
    if(q->tail > (MAX_TASKS - 1)){
        q->tail = 0;
    }else{
        q->tail++;
    }
    q->qTaskVector[q->tail] = &t;
}

void removeTaskFromQueue(queue* q){
    q->size--;
    if(q->head > (MAX_TASKS - 1)){
        q->head = 0;
    }else{
        q->head++;
    }
}

void decrementWaitTime(){

    // Decrement waitTime of every wait task
    for(int taskIndex = 0; taskIndex < MAX_TASKS; taskIndex++){
        if(taskVector[taskIndex].waitTime != 0){
            taskVector[taskIndex].waitTime--;
        }
    }
}

bool checkAvailableWaitTimes(queue* q){
    for(int i = 0; i < q->size; i++){
        if(q->qTaskVector[i]->waitTime == 0){
            return true;
        }
    }
    return false;
}

bool pickFromHighPriority(){
    bool hasAvailableTasks = checkAvailableWaitTimes(&highPriorityQueue);
    if(highPriorityQueue.size == 0 || !hasAvailableTasks){
        return false;
    }
    uint16_t position = highPriorityQueue.head;
    currentIndex = highPriorityQueue.qTaskVector[position]->index;
    return true;
}

bool pickFromLowPriority(){
    bool hasAvailableTasks = checkAvailableWaitTimes(&lowPriorityQueue);
    if(lowPriorityQueue.size == 0 || !hasAvailableTasks){
        return false;
    }
    uint16_t position = lowPriorityQueue.head;
    currentIndex = lowPriorityQueue.qTaskVector[position]->index;
    return true;
}

void executeScheduler(){

    decrementWaitTime();

    if(pickFromHighPriority()){
        return;
    }else{
        if(pickFromLowPriority()){
            return;
        }else{
            currentIndex = IDLE_INDEX;
            return;
        }
    }

    // Choosing next task, based on waitTime, giving priority to tasks with waitTime = 0
//    do{
//        currentIndex++;
//    }while(taskVector[currentIndex].waitTime != 0 && currentIndex < MAX_TASKS);
//    if(currentIndex == MAX_TASKS){
//        currentIndex = 0;
//    }
}

void idle(){
    while(1);
}

__attribute__ ((naked))
__attribute__ ((interrupt(WDT_VECTOR)))
void WDT_DISPATCHER(){

    // Saving Current Context
    asm("PUSHM.A #12,R15");
    asm("MOVX.A SP,%0" : "=m" (taskVector[currentIndex].pStack) );

    // Move SP to scheduler's stack
    asm("MOVX.W %0,SP" : "=m" (pStackScheduler));

    executeScheduler();

    // Restoring context
    asm("MOVX.A %0,SP" : "=m" (taskVector[currentIndex].pStack));
    asm("POPM.A #12,R15");

    asm("RETI");

}

void wait(uint16_t tickCount){
    taskVector[currentIndex].waitTime = tickCount;
    while(taskVector[currentIndex].waitTime);
}
