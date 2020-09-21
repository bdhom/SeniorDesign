/*
 * File:   interrupt.c
 * Author: bjdho
 *
 * Created on September 20, 2020, 10:48 PM
 */


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

void initializeInterrupts(void)
{
    //Interrupt nesting
    INTCON1bits.NSTDIS = 0;
    
    //Timer1 - interrupt priority = 7, interrupt flag reset, interrupt enabled
    IPC0bits.T1IP = 7;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    
    return;
}
