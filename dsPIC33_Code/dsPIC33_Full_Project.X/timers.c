#include "xc.h"
#include "ADC.h"
#include "PF.h"

extern bool I2C_Fault;
extern bool PF_Fault;

////////////////////////Timer functions for ADC/////////////////////////////////
void __attribute__((__interrupt__,no_auto_psv)) _T1Interrupt(void)
{
    if(IFS0bits.T1IF == 1)
    {
        ClearConversionDone();
        StartConverting();
        
        IFS0bits.T1IF = 0;
    }
}

void ADCTimerInit(void) //64 samples/cycle or ~3.839 ksps
{
    //EQN: T = (1/F) * PR1 * pre-scaler
    TMR1 = 0x00;
    
    //Period
    PR1 = 0x0078;
    
    //Timer control register
    T1CONbits.TON = 0;          //Timer off
    T1CONbits.TCS = 0;          //Internal instruction cycle clock -> 3.685e6 Hz
    T1CONbits.TGATE = 0;        //Disable gated timer mode
    T1CONbits.TCKPS = 0b01;    //1:8 pre-scaler
    
    
    IPC0bits.T1IP = 5;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 0;
}

void ADCTimerStart(void)
{
    T1CONbits.TON = 0;
    
    TMR1 = 0x00;
    
    IEC0bits.T1IE = 1;
    T1CONbits.TON = 1;
}

void ADCTimerStop(void)
{
    T1CONbits.TON = 0;
    IEC0bits.T1IE = 0;
}

////////////////////////Timer functions for I2C/////////////////////////////////
void __attribute__((__interrupt__,no_auto_psv)) _T2Interrupt(void)
{
    if(IFS0bits.T2IF == 1)
    {
        I2C_Fault = true;
        
        IFS0bits.T2IF = 0;
    }
}

void I2CTimerInit(void) //~1s reset when no I2C comm
{
    //EQN: T = (1/F) * PR1 * pre-scaler
    TMR2 = 0x00;
    
    //Period:  w/ 256 pre-scaler on control register
    PR2 = 0x383B;
    
    //Timer control register
    T2CONbits.TON = 0;          //Timer off
    T2CONbits.TCS = 0;          //Internal instruction cycle clock -> 3.685e6 Hz
    T2CONbits.TGATE = 0;        //Disable gated timer mode
    T2CONbits.TCKPS = 0b11;    //1:256 pre-scaler //*Change this pre-scaler
    T2CONbits.T32 = 0;
    
    IPC1bits.T2IP = 4;
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 0;
}

void I2CTimerStart(void)
{
    T2CONbits.TON = 0;
    
    TMR2 = 0x00;
    
    IEC0bits.T2IE = 1;
    T2CONbits.TON = 1;
}

void I2CTimerStop(void)
{
    T2CONbits.TON = 0;
    IEC0bits.T2IE = 0;
}

////////////////////////Timer functions for PF//////////////////////////////////
void __attribute__((__interrupt__,no_auto_psv)) _T3Interrupt(void)
{
    if(IFS0bits.T3IF == 1)
    {
        PFOff();
        
        PF_Fault = true;
        IFS0bits.T3IF = 0;
    }
}

void PFTimerInit(void) //~(1/60)s  or 16.67ms reset when no PF interrupt
{
    //EQN: T = (1/F) * PR1 * pre-scaler
    TMR3 = 0x00;
    
    //Period:  w/ 256 pre-scaler on control register
    PR3 = 0x00F0;
    
    //Timer control register
    T3CONbits.TON = 0;          //Timer off
    T3CONbits.TCS = 0;          //Internal instruction cycle clock -> 3.685e6 Hz
    T3CONbits.TGATE = 0;        //Disable gated timer mode
    T3CONbits.TCKPS = 0b11;    //1:256 pre-scaler //*Change this pre-scaler
    
    IPC2bits.T3IP = 3;
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 0;
}

void PFTimerStart(void)
{
    T3CONbits.TON = 0;
    
    TMR3 = 0x00;
    
    IEC0bits.T3IE = 1;
    T3CONbits.TON = 1;
}

void PFTimerStop(void)
{
    T3CONbits.TON = 0;
    IEC0bits.T3IE = 0;
}

uint16_t GetPFTimer(void)
{
    return (uint16_t)TMR3;
}

uint16_t GetPFPeriod(void)
{
    return (uint16_t)PR3;
}

uint16_t GetPFScaler(void)
{
    switch(T3CONbits.TCKPS)
    {
        case 1:
            return (uint16_t)8;
        case 2:
            return (uint16_t)64;
        case 3:
            return (uint16_t)256;
        default:
            return (uint16_t)1;
    }
}
