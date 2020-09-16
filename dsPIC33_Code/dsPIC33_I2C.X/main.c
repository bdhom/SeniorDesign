/*
 * File:   main.c
 * Author: bjdho
 *
 * Created on September 12, 2020, 10:19 PM
 */

#define _XTAL_FREQ 4000000
#define FCY (_XTAL_FREQ/2)

#include "xc.h"
#include <stdint.h>
#include <libpic30.h>

void Interrupt_Enable(void)
{
    __builtin_enable_interrupts();
}

void Setup_Pins(void)
{
    /****************************************************************************
     * Setting the Output Latch SFR(s)
     ***************************************************************************/
    LATA = 0x0002;
    LATB = 0x0000;

    /****************************************************************************
     * Setting the GPIO Direction SFR(s)
     ***************************************************************************/
    TRISA = 0x0015;
    TRISB = 0xFFFF;

    /****************************************************************************
     * Setting the Weak Pull Up and Weak Pull Down SFR(s)
     ***************************************************************************/
    CNPDA = 0x0000;
    CNPDB = 0x0000;
    CNPUA = 0x0000;
    CNPUB = 0x0000;

    /****************************************************************************
     * Setting the Open Drain SFR(s)
     ***************************************************************************/
    ODCA = 0x0000;
    ODCB = 0x0000;

    /****************************************************************************
     * Setting the Analog/Digital Configuration SFR(s)
     ***************************************************************************/
    ANSELA = 0x0015;
    ANSELB = 0x038C;
}

void I2C_Slave_Init(void)
{
    //Control registers
    I2C1CON1 = 0xC9C0;
    I2C1CON2 = 0x0070;
    I2C1STAT = 0X00;
    
    //Slave address and mask registers
    I2C1ADD = 0x07;
    I2C1MSK = 0x00;
    
    //Baud Rate Generator
    I2C1BRG = 0x01;
    
    //Clear Interrupt Flag, Enable, and Priority
    IFS1bits.SI2C1IF = 0; //Check for this flag in interrupt 
    IEC1bits.SI2C1IE = 1;
    IPC4bits.SI2C1IP = 7; //Highest Priority
}

void __attribute__ ((interrupt)) isr()
{
    if(IFS1bits.SI2C1IF)
    {
        IFS1bits.SI2C1IF = 0;
    }
    
    _LATA1 ^= 1;
    
    __delay_ms(1000);
}

int main(void)
{
    Interrupt_Enable();
    Setup_Pins();
    I2C_Slave_Init();
    
    while(1)
    {
        
    }
    return 1;
}
