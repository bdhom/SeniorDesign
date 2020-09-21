#include "xc.h"

void __attribute__((__interrupt__,no_auto_psv)) _T1Interrupt(void)
{
    /*If there were to be other interrupts, check the specific flag first before proceeding*/
    //Timer1 interrupt flag reset
    IFS0bits.T1IF = 0;
    
    //Toggle pin 3 (RA1)
    _LATA1 ^= 1;
}

void setupTimer1(void)
{
    //EQN: T = (1/F) * PR1 * pre-scaler
    //Start timer1 at 0
    TMR1 = 0x00;
    
    //Period: ~1.7s w/ 256 pre-scaler on control register
    PR1 = 0x6000;
    
    //Timer control register
    T1CON = 0x8030;
}
