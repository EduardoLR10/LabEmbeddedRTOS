#include "kernel.h"

void switchLEDGreen(){
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
    while(1){
        wait(250);
        P1OUT ^= BIT1;
    }
}

void switchLEDRed(){
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    while(1){
        wait(250);
        P1OUT ^= BIT0;
    }
}

int main(void){

    basicConfig();

    initializeQueues();

    registerTask(switchLEDGreen);
    registerTask(switchLEDRed);

    startRTOS();

}
