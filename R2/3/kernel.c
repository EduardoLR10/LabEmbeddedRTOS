#include "kernel.h"
#include "fifo.h"

const short int stackStart = 0x2802;
const short int stackSize = 0x80;
task taskVector[MAX_TASKS];
int currentIndex = 0;

fifo fifos[2];
fifo blockedFifo;
PRIORITY currentFifo;
bool starvationCase = false;

uint16_t* pStackScheduler = 0x2500;

// Compiler optimizes operations (WTF), removing operation add 24 to pStack
#pragma GCC optimize ("0")
void startRTOS(){

    registerTask(idle, LOW, 0);

    WDTCTL = (WDTPW | WDTSSEL__ACLK | WDTIS_6 | WDTTMSEL_1 | WDTCNTCL);
    SFRIE1 = WDTIE;

    currentIndex = 0;

    // Move first's stack pointer to initial position
    fifos[HIGH].taskVector[0].pStack += 24;
    currentFifo = HIGH;

    volatile uint16_t aux = fifos[HIGH].taskVector[0].pStack + 2;

    // Initialize stack pointer to first task
    asm("MOVX.W %0,SP" : "=m" (aux));
    asm("MOVX.A #8,SR");
    asm("MOVX.A %0,PC" : "=m" (fifos[HIGH].taskVector[0].pTask));

    return;
}

void basicConfig(){

    WDTCTL = WDTPW | WDTHOLD;

    PM5CTL0 &= ~BIT0;

    __enable_interrupt();

}

void initializeFifos(){
    for(int i = 0; i < N_FIFOS; i++){
        createFifo(&fifos[i]);
    }
    createFifo(&blockedFifo);
}

bool pickFromHighPriority(){
    if(fifos[HIGH].size == 0){
        return false;
    }
    currentIndex = fifos[HIGH].head;
    return true;
}

bool pickFromLowPriority(){
    if(fifos[LOW].size == 0){
        return false;
    }
    currentIndex = fifos[LOW].head;

    if(starvationCase){
        currentIndex = (currentIndex + 1) % MAX_TASKS;
        starvationCase = false;
    }

    return true;
}

void registerTask(void *pFunction, PRIORITY priority, uint16_t quantum){

    task t;

    // Add priority to task
    t.priority = priority;

    // Add quantum to task
    t.quantum = quantum;

    // Initialize waitTime
    t.waitTime = 0;

    // Put index inside task
    t.index = currentIndex;

    // Save address to function
    t.pTask = pFunction;

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
    t.pStack = stackBaseAddress;

    putTaskToPriorityFifo(&t);

    currentIndex++;
}

void decrementWaitTime(){

    // Decrement waitTime of every wait task in fifo
    for(int taskIndex = 0; taskIndex < MAX_TASKS; taskIndex++){
        if(blockedFifo.taskVector[taskIndex].waitTime > 0){
            blockedFifo.taskVector[taskIndex].waitTime--;
        }
    }

}

void putTaskToPriorityFifo(task* t){
    for(int i = 0; i < N_FIFOS; i++){
        if(t->priority == i){
            putTaskToFifo(&fifos[i], *t);
        }
    }
}

void addToBlockedTasks(){

    for(int fifoIndex = 0; fifoIndex < N_FIFOS; fifoIndex++){
        for(int taskIndex = 0; taskIndex < fifos[fifoIndex].size; taskIndex++){
            task candidateTask = getTaskFromFifo(&fifos[fifoIndex]);
            if(candidateTask.waitTime != 0){
                putTaskToFifo(&blockedFifo, candidateTask);
            }else{
                putTaskToPriorityFifo(&candidateTask);
            }
        }
    }

}

void manageBlockedTasks(){

    for(int taskIndex = blockedFifo.head; taskIndex != blockedFifo.tail; (taskIndex = (taskIndex + 1) % MAX_TASKS)){
        if(blockedFifo.taskVector[taskIndex].waitTime == 0){
            task releasedTask = getTaskFromFifo(&blockedFifo);

            // IDLE CASE
            if(fifos[LOW].size == 1 && releasedTask.priority == LOW){
                starvationCase = true;
            }

            putTaskToPriorityFifo(&releasedTask);
        }
    }
}

void executeScheduler(){

    decrementWaitTime();
    
    addToBlockedTasks();

    manageBlockedTasks();

    if(pickFromHighPriority()){
        currentFifo = HIGH;
        return;
    }else{
        currentFifo = LOW;
        pickFromLowPriority();
        return;
    }

    // Choosing next task, based on waitTime, giving priority to tasks with waitTime = 0
//    do{
//        currentIndex++;
//    }while(taskVector[currentIndex].waitTime != 0 && currentIndex < MAX_TASKS);
//    if(currentIndex == MAX_TASKS){
//        currentIndex = 0;
//    }
}

void wait(uint16_t tickCount){
    fifos[currentFifo].taskVector[currentIndex].waitTime = tickCount;
    while(fifos[currentFifo].taskVector[currentIndex].waitTime);
}

__attribute__ ((naked))
__attribute__ ((interrupt(WDT_VECTOR)))
void WDT_DISPATCHER(){

    // Saving Current Context
    asm("PUSHM.A #12,R15");
    asm("MOVX.A SP,%0" : "=m" (fifos[currentFifo].taskVector[currentIndex].pStack) );

    // Move SP to scheduler's stack
    asm("MOVX.W %0,SP" : "=m" (pStackScheduler));

    executeScheduler();

    // Restoring context
    asm("MOVX.A %0,SP" : "=m" (fifos[currentFifo].taskVector[currentIndex].pStack));
    asm("POPM.A #12,R15");

    asm("RETI");

}
