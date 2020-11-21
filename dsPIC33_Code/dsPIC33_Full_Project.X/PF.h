#ifndef PF_H
#define	PF_H

void PFOn(uint16_t size, uint16_t *pointer);

void PFOff(void);

void PFReset(void);

void __attribute__ ( ( interrupt, no_auto_psv ) ) _INT0Interrupt ( void );

#endif