/*
 * File:   main.c
 * Author: bjdho
 *
 * Created on September 16, 2020, 11:25 AM
 */

#define _XTAL_FREQ  3685000UL
#define FCY (_XTAL_FREQ/2)

#include "config.h"
#include "xc.h"
#include "interrupt.h"
#include "timer.h"

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
