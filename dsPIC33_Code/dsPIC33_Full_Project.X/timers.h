#ifndef TIMER_H
#define	TIMER_H

void __attribute__((__interrupt__,no_auto_psv)) _T1Interrupt(void);

void ADCTimerInit(void);

void ADCTimerStart(void);

void ADCTimerStop(void);

void __attribute__((__interrupt__,no_auto_psv)) _T2Interrupt(void);

void I2CTimerInit(void);

void I2CTimerStart(void);

void I2CTimerStop(void);

void __attribute__((__interrupt__,no_auto_psv)) _T3Interrupt(void);

void PFTimerInit(void);

void PFTimerStart(void);

void PFTimerStop(void);

uint16_t GetPFTimer(void);

uint16_t GetPFPeriod(void);

uint16_t GetPFScaler(void);

#endif