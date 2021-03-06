#define _XTAL_FREQ 3685000UL
#define FCY (_XTAL_FREQ/2)

#define PF_SAMPLES 10

#include "config.h"
#include "xc.h"
#include <stdint.h>
#include <stdbool.h>
#include "pins.h"
#include "globals.h"
#include "timers.h"
#include "I2C.h"
#include "ADC.h"
#include "PF.h"

volatile bool Change_Conversion = true;
volatile bool I2C_Change = false;
volatile bool I2C_Fault = false;
volatile bool PF_Fault = false;
volatile uint16_t Conversion_Num = 0;
volatile uint16_t ADC_Voltage[SAMPLE_SIZE];
volatile uint16_t ADC_Current1[SAMPLE_SIZE];
volatile uint16_t ADC_Current2[SAMPLE_SIZE];
volatile uint16_t ADC_Current3[SAMPLE_SIZE];
volatile uint16_t PF_Avg[3];

bool PF_Calculations = false;
uint8_t Conversion_Type = 0;
uint8_t I2C_Type = 0;
uint16_t PF_Timings[PF_SAMPLES];

int main(void) 
{
    SetupPins();
    EnableInterrupts();
    ADCInit();
    I2CSlaveInit();
    ADCTimerInit();
    I2CTimerInit();
    PFTimerInit();
    
    while(1)
    {
        //Step 1: ADC conversions of voltage and currents. Case 5 PF timings
        if(Change_Conversion)
        {
            Conversion_Type++;
            Conversion_Type = (Conversion_Type < 6)? Conversion_Type : 0;
            
            ADCTimerStop();
            
            switch(Conversion_Type)
            {
                case 1:
                    ADCOn();
                    ChangeADCPointer(ADC_Voltage);
                    ChangeADCInput(5);
                    ADCTimerStart();
                    break;
                case 2:
                    ChangeADCPointer(ADC_Current1);
                    ChangeADCInput(0);
                    ADCTimerStart();
                    break;
                case 3:
                    ChangeADCPointer(ADC_Current2);
                    ChangeADCInput(4);
                    ADCTimerStart();
                    break;
                case 4:
                    ChangeADCPointer(ADC_Current3);
                    ChangeADCInput(1);
                    ADCTimerStart();
                    break;
                case 5:       //For PF timer
                    ADCOff();
                    PFOn(PF_SAMPLES, PF_Timings);
                    break;
                default:        //Case 0
                    PFOff();
                    PF_Calculations = true;
                    break;
            }
            
            Conversion_Num = 0;
            Change_Conversion = false;
        }
        
        //Step 2: Averaging of PF timings
        if(PF_Calculations)
        {
            uint16_t Total_Time = 0;
            uint16_t i = 0;
            
            for(; i < PF_SAMPLES; i++)
            {
                Total_Time += PF_Timings[i];
            }
            
            PF_Avg[0] = Total_Time / PF_SAMPLES;
            PF_Avg[1] = GetPFPeriod();
            PF_Avg[2] = GetPFScaler();
            
            PF_Calculations = false;
            I2C_Change = true;
        }
        
        //Step 3: Send conversions and PF over I2C
        if(I2C_Change)
        {
            I2C_Type++;
            I2C_Type = (I2C_Type < 2)? I2C_Type : 0;
            
            I2CTimerStop();
            
            switch(I2C_Type)
            {
                case 1:
                    I2CEnable();
                    break;
                default:
                    while(I2CTransmitInProgress());
                    I2CDisable();
                    Change_Conversion = true;
                    break;
            }
            
            I2C_Change = false;
        }
        
        //Implement fault if needed for I2C
        if(I2C_Fault)
        {
            I2C_Fault = false;
        }
        
        //For PF signal "stalls"
        if(PF_Fault)
        {
            Change_Conversion = true;
            Conversion_Type = 5;
            
            PF_Fault = false;
        }
    }
    
    return 1;
}
