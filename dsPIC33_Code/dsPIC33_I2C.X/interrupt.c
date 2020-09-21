#include "xc.h"

void enableInterrupts(void)
{
    //Global interrupt enable
    INTCON2bits.GIE = 1;
    
    return;
}

void disableInterrupts(void)
{
    INTCON2bits.GIE = 0;
    
    return;
}

void initializeInterrupts(void) //*Need to add I2C slave init
{
    //Interrupt nesting
    INTCON1bits.NSTDIS = 0;
    
    //*Get rid of this?
//    //Timer1 - interrupt priority = 7, interrupt flag reset, interrupt enabled
//    IPC0bits.T1IP = 7;
//    IFS0bits.T1IF = 0;
//    IEC0bits.T1IE = 1;
    
    //Clear Interrupt Flag, Enable, and Priority
    IFS1bits.SI2C1IF = 0; //Check for this flag in interrupt 
    IEC1bits.SI2C1IE = 1;
    IPC4bits.SI2C1IP = 7; //Highest Priority
    
    return;
}
