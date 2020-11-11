/*
 * File:   main.c
 * Author: bjdho
 *
 * Created on September 12, 2020, 10:19 PM
 */

#define _XTAL_FREQ 4000000
#define FCY (_XTAL_FREQ/2)

#include "config.h"
#include "xc.h"
#include "pins.h"
#include "interrupt.h"
#include "I2C.h"

int main(void)
{
    enableInterrupts();
    initializeInterrupts();
    setupPins();
    I2CSlaveInit();
    
    while(1)
    {
        
    }
    
    return 1;
}
