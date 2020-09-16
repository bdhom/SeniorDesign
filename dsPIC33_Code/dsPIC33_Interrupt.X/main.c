/*
 * File:   main.c
 * Author: bjdho
 *
 * Created on September 16, 2020, 11:25 AM
 */

#define _XTAL_FREQ  3685000UL
#define FCY (_XTAL_FREQ/2)

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

void __attribute__((__interrupt__,no_auto_psv)) _T1Interrupt(void)
{
    /*If there were to be other interrupts, check the specific flag first before proceeding*/
    //Timer1 interrupt flag reset
    IFS0bits.T1IF = 0;
    
    //Toggle pin 3
    _LATA1 ^= 1;
}

void setupTimer1(void)
{
    //Start timer1 at 0
    TMR1 = 0x00;
    
    //Period: ~1.7s w/ 256 pre-scaler on control register
    PR1 = 0x6000;
    
    //Timer control register
    T1CON = 0x8030;
}

int main(void) 
{
    enableInterrupts();
    initializeInterrupts();
    setupTimer1();
    
    TRISA = 0x0015;
    TRISB = 0xFFFF;
    
    while(1)
    {
        
    }
    return 1;
}
