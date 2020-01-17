#include <msp430.h>

#define MAX_TASKS 2

/**
 * main.c
 */

typedef struct task{
    short int* pStack;
    short int* pTask;
}task;

const short int stackStart = 0x2802;
const short int stackSize = 0x80;

task taskVector[2];

int currentIndex = 0;

short int* pStackScheduler = 0x2500;

void registerTask(void *pFunction){

    taskVector[currentIndex].pTask = pFunction;

    int msbPC = (((int)pFunction & 0x000F0000) >> 4);

    // Finding stack's address;
    short int* stackBaseAddress = (stackStart - (stackSize*currentIndex));

    stackBaseAddress--;

    *stackBaseAddress = pFunction;

    stackBaseAddress--;

    *stackBaseAddress = (msbPC| 0x008);

    // Pushing registers
    int i = 12;
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

void startRTOS(){

    // Initialize
    asm("MOV.W %0,SP" : "=m" (taskVector[0].pStack));

    currentIndex = -1;
    WDTCTL = (WDTPW | WDTSSEL__ACLK | WDTIS_4 | WDTTMSEL_1);
    SFRIE1 = WDTIE;

    while(1);
}


void main(void){

    basicConfig();

    registerTask(switchLEDGreen);
    registerTask(switchLEDRed);

    //taskVector[0].pStack += 13;

    startRTOS();

}

__attribute__ ((naked))
__attribute__ ((interrupt(WDT_VECTOR)))
void WDT_DISPATCHER(){

    // Saving Current Context
    if(currentIndex >= 0){
        asm("PUSHM.W #12,R15");
        asm("MOV.W SP,%0" : "=m" (taskVector[currentIndex].pStack) );
    }

    // Choosing next task
    currentIndex = (currentIndex + 1) % MAX_TASKS;

    // Restoring context
    asm("MOV.W %0,SP" : "=m" (taskVector[currentIndex].pStack));
    asm("POPM.W #12,R15");

    asm("RETI");

}
