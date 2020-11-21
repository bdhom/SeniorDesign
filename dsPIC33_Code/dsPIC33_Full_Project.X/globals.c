#include "xc.h"

void EnableInterrupts(void)
{
    //Global interrupt enable
    INTCON2bits.GIE = 1;
    
    //Interrupt nesting
    INTCON1bits.NSTDIS = 0;
}

void DisableInterrupts(void)
{
    INTCON2bits.GIE = 0;
}
