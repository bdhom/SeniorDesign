#ifndef TIMER_H
#define	TIMER_H

void __attribute__((__interrupt__,no_auto_psv)) _T1Interrupt(void);

void setupTimer1(void);

#endif	/* XC_HEADER_TEMPLATE_H */

