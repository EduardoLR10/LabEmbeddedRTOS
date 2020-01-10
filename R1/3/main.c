#include <msp430.h> 


/**
 * main.c
 */

typedef struct task{
    short int* pStack;
    short int* pTask;
}task;

task taskVector[2];
int currentIndex = 0;
short int* pStackScheduler;

task registerTask(short int* pFunction){

    if(currentIndex == 0){
        taskVector[currentIndex].pStack = 0x2800;
    }else{
        taskVector[currentIndex].pStack = 0x2800 - (0x80*currentIndex);
    }

    taskVector[currentIndex].pTask = pFunction;
    currentIndex++;
}

void delay(int number){
    while(number){
        number--;
    }
}

void switchLEDGreen(){
    while(1){
        delay(50000);
        P1OUT ^= BIT1;
    }
}

void switchLEDRed(){
    while(1){
        delay(50000);
        P1OUT ^= BIT1;
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

    WDTCTL = (WDTPW | WDTSSEL__ACLK | WDTIS_4 | WDTTMSEL_1);
    SFRIE1 = WDTIE;
	
    asm("MOV.W %0,SP" : "=m" (taskVector[0].pStack));

    while(1);

}

__attribute__ ((interrupt(WDT_VECTOR)))
void WDT_DISPATCHER(){

    // Saving Current Context
    asm("PUSHM.W #12,R15");
    if(currentIndex >= 0){
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

}
