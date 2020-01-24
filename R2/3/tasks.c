#include "tasks.h"

void switchLEDGreen(){
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
    while(1){
        wait(25);
        P1OUT ^= BIT1;
    }
}

void switchLEDRed(){
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    while(1){
        wait(25);
        P1OUT ^= BIT0;
    }
}

void idle(){
    while(1);
}
