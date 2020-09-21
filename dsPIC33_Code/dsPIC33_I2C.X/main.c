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

//*Move into I2C.h/.c after verifying interrupt works
void __attribute__((__interrupt__,no_auto_psv)) _I2CInterrupt()
{
    if(IFS1bits.SI2C1IF)
    {
        IFS1bits.SI2C1IF = 0;
        
        if (I2CReceivedRestart())
        {
            while(!I2CAddressReceived());
            I2CTransmitRegister(0x53);
            _LATA1 ^= 1;
        }
    }
}

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
