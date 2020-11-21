#ifndef ADC_H
#define	ADC_H

#include <stdbool.h>
#include <stdint.h>

void ADCInit(void);

bool ConversionDone(void);

void ClearConversionDone(void);

void StartConverting(void);

void ADCOff(void);

void ADCOn(void);

void ChangeADCInput(uint16_t input);

void ChangeADCPointer(uint16_t *pointer);

void __attribute__((__interrupt__,auto_psv)) _AD1Interrupt(void);

#endif