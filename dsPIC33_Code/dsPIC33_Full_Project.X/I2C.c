#include "xc.h"
#include <stdbool.h>
#include <stdint.h>
#include "globals.h"
#include "timers.h"

//*Need to look into the control/status registers
//*Add functions according to the different states the I2C slave is in
//*Add functions to create smoother experience when dealing with I2C slave

extern bool I2C_Change;
extern uint16_t I2C_Comm_Num;

extern uint16_t ADC_Voltage[SAMPLE_SIZE];
extern uint16_t ADC_Current1[SAMPLE_SIZE];
extern uint16_t ADC_Current2[SAMPLE_SIZE];
extern uint16_t ADC_Current3[SAMPLE_SIZE];
extern uint16_t PF_Avg[3];

unsigned char temp;
uint16_t Size;
uint16_t *I2C_Value;
uint8_t array;
uint8_t i;
bool arrayWrite = true;

void I2CSlaveInit(void)
{
    //Control registers
    //I2C Enable; Idle Continue; Clock Released; Support Disabled; 7-Bit Address;
    //Slew Rate Enabled; Enable I/O; General Call Enabled; Clock Stretch Disabled
    //I2C1CON1 = 0xC180;
    
    I2C1CON1bits.I2CEN = 0;     //I2C disabled
    I2C1CON1bits.I2CSIDL = 0;   //Discontinues in idle mode
    I2C1CON1bits.SCLREL = 1;    //Clock released
    I2C1CON1bits.STRICT = 0;    //No strict addressing
    I2C1CON1bits.A10M = 0;      //7-bit address
    I2C1CON1bits.DISSLW = 1;    //Disable slew rate control
    I2C1CON1bits.SMEN = 1;      //Enable input logic
    I2C1CON1bits.GCEN = 0;      //General call disabled
    I2C1CON1bits.STREN = 1;     //Clock stretching is enabled
    I2C1CON1bits.ACKDT = 0;     //Acknowledge send
    
    //Stop Condition Interrupt; Start Condition Interrupt; Overwrite Disabled; 
    //300ns SDA Hold Time; Collision Detect Disabled; Address Holding Disabled;
    //Data Holding Disabled
    //I2C1CON2 = 0x0068; //0x0068
    
    I2C1CON2bits.PCIE = 0;      //Stop condition interrupt
    I2C1CON2bits.SCIE = 0;      //Start condition interrupt
    I2C1CON2bits.BOEN = 0;      //Receive buffer overwrite disabled
    I2C1CON2bits.SDAHT = 1;     //300ns SDA hold time
    I2C1CON2bits.SBCDE = 0;     //Collision detect disabled
    I2C1CON2bits.AHEN = 0;      //Address holding disabled
    I2C1CON2bits.DHEN = 0;      //Data holding disabled
    
    //Slave address and mask registers
    I2C1ADD = 0x07;
    I2C1MSK = 0x00;
    
    I2C1BRG = 0x01;
    
    //Clear I2C Interrupt Flag and Set Enable and Priority
    IFS1bits.SI2C1IF = 0;
    IEC1bits.SI2C1IE = 0;
    IPC4bits.SI2C1IP = 6;
}

//void ChangeI2CPointer(uint16_t change)
//{
//    //I2C_Value = pointer;
//    
//    array = change;
//}

void ChangeI2CSamples(uint16_t samples)
{
    Size = samples;
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _SI2C1Interrupt ( void )
{
    if(_SI2C1IF == 1)
    {
        if( (I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 0) ) // Device address matched (write)
        {
            temp = I2C1RCV;
            
//            if(arrayWrite == true)
//            {
//                array = temp;
//                arrayWrite = false;
//            }
//            else
//            {
//                i = temp;
//                arrayWrite = true;
//            }
            
            I2C1CON1bits.SCLREL = 1;
        }
        else if( (I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 1) && (I2C1STATbits.ACKSTAT == 0)) //check for data (write)
        {
            temp = I2C1RCV;
            
            if(arrayWrite == true)
            {
                array = temp;
                arrayWrite = false;
            }
            else
            {
                i = temp;
                arrayWrite = true;
            }
            
            I2C1CON1bits.SCLREL = 1;
        }
         else if( (I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 0) ) // Device address matched (read)
        {
            I2CTimerStart();
            temp = I2C1RCV;
            
            switch(array)
            {
                case 0:
                    I2C1TRN = ADC_Voltage[i] >> 8;
                    break;
                case 1:
                    I2C1TRN = ADC_Current1[i] >> 8;
                    break;
                case 2:
                    I2C1TRN = ADC_Current2[i] >> 8;
                    break;
                case 3:
                    I2C1TRN = ADC_Current3[i] >> 8;
                    break;
                case 4:
                    I2C1TRN = PF_Avg[i] >> 8;
                    break;
            }
            //I2C1TRN = *(I2C_Value + I2C_Comm_Num) >> 8; //data out
            
            I2C1CON1bits.SCLREL = 1;
        }
        else if ( (I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 1) && (I2C1STATbits.ACKSTAT == 0 ))
        {
            I2CTimerStart();
            temp = I2C1RCV;
            
            switch(array)
            {
                case 0:
                    I2C1TRN = ADC_Voltage[i];
                    break;
                case 1:
                    I2C1TRN = ADC_Current1[i];
                    break;
                case 2:
                    I2C1TRN = ADC_Current2[i];
                    break;
                case 3:
                    I2C1TRN = ADC_Current3[i];
                    break;
                case 4:
                    I2C1TRN = PF_Avg[i];
                    break;
            }
            //I2C1TRN = *(I2C_Value + I2C_Comm_Num); //data out
            
//            if(*(I2C_Value + I2C_Comm_Num) == 0)
//            {
//                LATAbits.LATA2 ^= 1;
//            }
            
//            if(I2C_Comm_Num < 0 || I2C_Comm_Num > 255)
//            {
//                LATBbits.LATB4 ^= 1;
//            }

//            if(I2C_Comm_Num < (Size - 1))
//            {
//                I2C_Comm_Num++;
//            }
            if(array == 4 && i == 2)    //Last byte of data from PF_Avg
            {
                I2C_Change = true;
            }
            
            I2C1CON1bits.SCLREL = 1; 
        }

        _SI2C1IF = 0;
    }
}

//Functions to use control registers (I2C1CON1 & I2C1CON2)
void I2CEnable(void)
{
    I2C1CON1bits.I2CEN = 1;
    I2C1CON1bits.GCEN = 1;
    IEC1bits.SI2C1IE = 1;
}

void I2CDisable(void)
{
    I2C1CON1bits.I2CEN = 0;
    I2C1CON1bits.GCEN = 0;
    IEC1bits.SI2C1IE = 0;
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
