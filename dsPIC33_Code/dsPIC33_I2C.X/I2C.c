#include "xc.h"
#include <stdbool.h>
#include <stdint.h>

//*Need to look into the control/status registers
//*Add functions according to the different states the I2C slave is in
//*Add functions to create smoother experience when dealing with I2C slave

unsigned char datain = 0;
unsigned char dataout = 0;
unsigned char temp;

void I2CSlaveInit(void)
{
    //Control registers
    //I2C Enable; Idle Continue; Clock Released; Support Disabled; 7-Bit Address;
    //Slew Rate Enabled; Enable I/O; General Call Enabled; Clock Stretch Disabled
    I2C1CON1 = 0xC180;
    //Stop Condition Interrupt; Start Condition Interrupt; Overwrite Disabled; 
    //300ns SDA Hold Time; Collision Detect Disabled; Address Holding Disabled;
    //Data Holding Disabled
    I2C1CON2 = 0x0068; //0x0068
    
    //Slave address and mask registers
    I2C1ADD = 0x07;
    I2C1MSK = 0x00;
    
    I2C1BRG = 0x01;
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _SI2C1Interrupt ( void )
{
    if( (I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 0) ) // Device address matched (write)
    {
        temp = I2C1RCV; //dummy read to clear RCV buffer
        I2C1CON1bits.SCLREL = 1;
    }
    else if( (I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 1) ) //check for data (write)
    {
        datain = I2C1RCV; //transfer data from RCV buffer
        I2C1CON1bits.SCLREL = 1;
    }
     else if( (I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 0) ) // Device address matched (read)
    {
        temp = I2C1RCV;
        I2C1TRN = dataout++;
        I2C1CON1bits.SCLREL = 1;
    }
    else if ( (I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 1) && (I2C1STATbits.ACKSTAT == 0 ))
    {
        temp = I2C1RCV;
        I2C1TRN = dataout++;
        I2C1CON1bits.SCLREL = 1; 
    }

    _SI2C1IF = 0; //clear I2C1 Slave interrupt flag 
}

//Functions to use control registers (I2C1CON1 & I2C1CON2)
void I2CEnable(void)
{
    I2C1CON1bits.I2CEN = 1;
}

void I2CDisable(void)
{
    I2C1CON1bits.I2CEN = 0;
}

void I2CSlaveClockHold(void)
{
    I2C1CON1bits.STREN = 1;
    I2C1CON1bits.SCLREL = 0;
}

void I2CSlaveClockRelease(void)
{
    I2C1CON1bits.STREN = 0;
    I2C1CON1bits.SCLREL = 1;
}

void I2CAddressSet(uint16_t address)
{
    I2C1ADD = address;
}

void I2CMaskSet(uint16_t mask)
{
    I2C1MSK = mask;
}

void I2CEnableAddressHolding(void)
{
    I2C1CON2bits.AHEN = 1;
}

void I2CDisableAddressHolding(void)
{
    I2C1CON2bits.AHEN = 0;
}

void I2CEnableDataHolding(void)
{
    I2C1CON2bits.DHEN = 1;
}

void I2CDisableDataHolding(void)
{
    I2C1CON2bits.DHEN = 0;
}

//Functions to check status register (I2C1STAT)
bool  I2CReceivedACK(void)
{
    return (I2C1STATbits.ACKSTAT == 0);
}

bool I2CAcknowledgeInProgress(void)
{
    return (I2C1STATbits.ACKTIM == 1);
}

bool I2CAddressReceived(void) //For 7-Bit Address
{
    return (I2C1STATbits.GCSTAT == 1);
}

bool I2CReceiveOverflow(void)
{
    return (I2C1STATbits.I2COV == 1);
}

bool I2CReceivedData(void)
{
    return (I2C1STATbits.D_A == 1);
}

bool I2CReceivedStop(void)
{
    return (I2C1STATbits.P == 1);
}

bool I2CReceivedRestart(void)
{
    return (I2C1STATbits.S == 1);
}

bool I2CRead(void) //An I2C read is an output from the slave
{
    return (I2C1STATbits.R_W == 1);
}

bool I2CReceiveFull(void) //Hardware clears RCV register when software reads
{
    return (I2C1STATbits.RBF == 1);
}

bool I2CTransmitInProgress(void)
{
    return (I2C1STATbits.TBF == 1); //Transmit in progress. Buffer full
}

void I2CTransmitRegister(uint8_t value)
{
    I2C1TRN = value;
}

uint8_t I2CReadReceiveRegister(void)
{
    return (uint8_t)I2C1RCV;
}
