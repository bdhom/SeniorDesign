//*Edit File
#include "xc.h"
#include <stdbool.h>
#include <stdint.h>
#include "globals.h"

extern bool Change_Conversion;
extern uint16_t Conversion_Num;

uint16_t *ADC_Results;

void ADCInit(void) //*May need to set negative end of op amp //*Check CON4 control registers
{
   AD1CON1bits.ADON = 0;    //Turn ADC Off
   AD1CON1bits.AD12B = 0;   //Select 10-bit mode  
   AD1CON1bits.FORM = 0;    //Integer Format
   AD1CON1bits.SSRC = 0;    //Manual Conversion on sample bit clear 
   AD1CON1bits.SIMSAM = 0;  //Disable Simultaneous Sampling
   AD1CON1bits.ASAM = 1;    //Auto Sampling 
   AD1CON1bits.DONE = 0;    //Clearing Done bit
   AD1CON2bits.VCFG = 0;    //AVss and AVdd 
   AD1CON2bits.CSCNA = 0;   //Don't scan inputs
   AD1CON2bits.CHPS = 0;    //Select 1-channel mode (CH0)
   AD1CON2bits.ALTS = 0;    //Disable Alternate Input Selection
   AD1CON3bits.ADRC = 0;    //Conv. clock from system clock
   AD1CON3bits.ADCS =10;    //TAD=TCY 
   AD1CON3bits.SAMC = 30;
   
   AD1CON1bits.ADSIDL = 1;  //Discontinue in idle mode
   AD1CON1bits.SSRCG = 0;
   AD1CON2bits.SMPI = 0;    //ADC interrupt every conversion
   
   AD1CHS0bits.CH0SA = 0;            //Select VREF- for CH0 -ve input Select AN0 for CH0 +ve input
   AD1CHS0bits.CH0NA = 0;
   AD1CSSL = 0x0000;        //Channel Scanning is disabled. All bits left to their default state
   //AD1CHS123 = 0;           //Select VREF- for all and  AN0,AN1,AN2 for channel 1,2,3 
    
    //Clear ADC Interrupt Flag and Set Enable and Priority
    IFS0bits.AD1IF = 0;
    IEC0bits.AD1IE = 0;
    IPC3bits.AD1IP = 7;
}

bool ConversionDone(void)
{
    return (AD1CON1bits.DONE == 1);
}

void ClearConversionDone(void)
{
    AD1CON1bits.DONE = 0;
}

void StartConverting(void)
{
    AD1CON1bits.SAMP = 0; //S&H holding and adc converting
}

void ADCOff(void)
{            
    AD1CON1bits.ADON = 0;
    IEC0bits.AD1IE = 0;
}

void ADCOn(void)
{
    AD1CON1bits.ADON = 1;
    IEC0bits.AD1IE = 1;
}

void ChangeADCInput(uint16_t input)
{
    AD1CHS0bits.CH0SA = input;
}

void ChangeADCPointer(uint16_t *pointer)
{
    ADC_Results = pointer;
}

void __attribute__((__interrupt__,auto_psv)) _AD1Interrupt(void)
{ 
    if(IFS0bits.AD1IF == 1)
    {
        *(ADC_Results + Conversion_Num) = ADC1BUF0;
        
        if(Conversion_Num < (SAMPLE_SIZE - 1))
        {
            Conversion_Num++;
        }
        else 
        {
            Change_Conversion = true;
        }

        LATAbits.LATA2 ^= 1;
        
        IFS0bits.AD1IF = 0; 
    }
}
