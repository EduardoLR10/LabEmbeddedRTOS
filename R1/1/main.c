#include <msp430.h>
#include <stdint.h>


/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~BIT0; // remove High-Z

    P1DIR |= BIT1;

    volatile uint32_t i;

    int x = 25000;
    int y = 50000;

    while(1){
        asm("MOV.W %0,%1" : "=m" (x) : "m" (i) );
        asm("MOV.W %0,%1" : "=m" (y) : "m" (i) );
        while(i--){
            P4OUT ^= BIT1;
        }
    }

    return 0;
}
