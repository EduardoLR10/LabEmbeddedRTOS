#include <msp430.h>
#include <stdint.h>

#define MAX_TASKS 2

/**
 * main.c
 */

typedef struct task{
    volatile uint16_t* pStack;
    volatile uint16_t* pTask;
}task;

const short int stackStart = 0x2802;
const short int stackSize = 0x80;

task taskVector[2];

int currentIndex = 0;

uint16_t* pStackScheduler = 0x2500;

void registerTask(void *pFunction){

    taskVector[currentIndex].pTask = pFunction;

    int msbPC = ((int)pFunction >> 4);

    // Finding stack's address;
    uint16_t* stackBaseAddress = (stackStart - (stackSize*currentIndex));

    stackBaseAddress--;

    *stackBaseAddress = pFunction;

    stackBaseAddress--;

    *stackBaseAddress = ((msbPC & 0x0000F000) | 0x008);

    // Pushing registers
    int i = 24;
    while(i){
        *(--stackBaseAddress) = 0;
        i--;
    }

    // Saving stack pointer in task's stack pointer
    taskVector[currentIndex].pStack = stackBaseAddress;

    currentIndex++;
}

void switchLEDGreen(){
    P1DIR |= BIT1;
    int i;
    while(1){
        i = 500000;
        while(i){
            i--;
        }
        P1OUT ^= BIT1;
    }
}

void switchLEDRed(){
    P1DIR |= BIT0;
    int i;
    while(1){
        i = 500000;
        while(i){
            i--;
        }
        P1OUT ^= BIT0;
    }
}

void basicConfig(){

    WDTCTL = WDTPW | WDTHOLD;

    PM5CTL0 &= ~BIT0;

    __enable_interrupt();

}

// Compiler optimizes operations (WTF), removing operation add 24 to pStack
#pragma GCC optimize ("0")
void startRTOS(){

    WDTCTL = (WDTPW | WDTSSEL__ACLK | WDTIS_4 | WDTTMSEL_1 | WDTCNTCL);
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



void main(void){

    basicConfig();

    registerTask(switchLEDGreen);
    registerTask(switchLEDRed);

    startRTOS();

}

__attribute__ ((naked))
__attribute__ ((interrupt(WDT_VECTOR)))
void WDT_DISPATCHER(){

    // Saving Current Context
    asm("PUSHM.A #12,R15");
    asm("MOVX.A SP,%0" : "=m" (taskVector[currentIndex].pStack) );

    // Move SP to scheduler's stack
    asm("MOVX.W %0,SP" : "=m" (pStackScheduler));

    // Choosing next task
    currentIndex = (currentIndex + 1) % MAX_TASKS;

    // Save scheduler's stack
    asm("MOVX.W SP,%0" : "=m" (pStackScheduler));

    // Restoring context
    asm("MOVX.A %0,SP" : "=m" (taskVector[currentIndex].pStack));
    asm("POPM.A #12,R15");

    asm("RETI");

}
