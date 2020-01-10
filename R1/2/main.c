#include <msp430.h> 


/**
 * main.c
 */
int main(void){
    PM5CTL0 &= ~BIT0;
    __enable_interrupt();
	WDTCTL = (WDTPW | WDTSSEL__ACLK | WDTIS_4 | WDTTMSEL_1);
	SFRIE1 = WDTIE;
	P1DIR |= BIT1;
	
	return 0;
}

__attribute__ ((interrupt(WDT_VECTOR)))
void WDT_ISR(){
    P1OUT ^= BIT1;
}
