#include "xc.h"
#include <stdbool.h>
#include <stdint.h>
#include "globals.h"
#include "timers.h"

extern bool Change_Conversion;

uint16_t Sample_Count;
uint16_t Sample_Size;
uint16_t *PF_Results;

void PFOn(uint16_t size, uint16_t *pointer)
{
    Sample_Count = 0;
    Sample_Size = size;
    PF_Results = pointer;
    
    IFS0bits.INT0IF = 0;
    IPC0bits.INT0IP = 6;
    INTCON2bits.INT0EP = 0;
    
    PFTimerStart();
    IEC0bits.INT0IE = 1;
}

void PFOff(void)
{
    PFTimerStop();
    IEC0bits.INT0IE = 0;
}

void PFReset(void)
{
    PFTimerStart();
    IEC0bits.INT0IE = 1;
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _INT0Interrupt ( void )
{
    if(_INT0IF == 1)
    {
        if(INTCON2bits.INT0EP == 0)
        {
            PFTimerStart();
        }
        else
        {
            PFTimerStop();
            
            *(PF_Results + Sample_Count) = GetPFTimer();
            Sample_Count++;
            
            if(Sample_Count >= Sample_Size)
            {
                PFOff();
                Change_Conversion = true;
            }
            else
            {
                PFReset();
            }
        }
        
        INTCON2bits.INT0EP ^= 1;
        _INT0IF = 0;
    }
}