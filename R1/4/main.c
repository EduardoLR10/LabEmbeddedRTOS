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

void switchLEDGreen(){
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
    int i;
    while(1){
        i = 500000;
        while(i){
            i--;
        }
        P1OUT ^= BIT0;
    }
}


void main(void){

    WDTCTL = WDTPW | WDTHOLD;

    PM5CTL0 &= ~BIT0;

    __enable_interrupt();

    P1DIR |= BIT1;
    P1DIR |= BIT0;

    registerTask(switchLEDGreen);
    registerTask(switchLEDRed);

    currentIndex = -1;

    // Initialize
    asm("MOV.W %0,SP" : "=m" (taskVector[0].pStack));

    WDTCTL = (WDTPW | WDTSSEL__ACLK | WDTIS_4 | WDTTMSEL_1);
    SFRIE1 = WDTIE;

    while(1);

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
    if(currentIndex == 1){
        currentIndex = 0;
    }else{
        currentIndex++;
    }

    // Restoring context
    asm("MOV.W %0,SP" : "=m" (taskVector[currentIndex].pStack));
    asm("POPM.W #12,R15");

    asm("RETI");

}
