#ifndef I2C_H
#define	I2C_H

#include <stdbool.h>
#include <stdint.h>

void I2CSlaveInit(void);

void ChangeI2CPointer(uint16_t *pointer);

void ChangeI2CSamples(uint16_t samples);

void __attribute__ ( ( interrupt, no_auto_psv ) ) _SI2C1Interrupt ( void );

void I2CEnable(void);

void I2CDisable(void);

void I2CSlaveClockHold(void);

void I2CSlaveClockRelease(void);

void I2CAddressSet(uint16_t address);

void I2CMaskSet(uint16_t mask);

void I2CEnableAddressHolding(void);

void I2CDisableAddressHolding(void);

void I2CEnableDataHolding(void);

void I2CDisableDataHolding(void);

bool  I2CReceivedACK(void);

bool I2CAcknowledgeInProgress(void);

bool I2CAddressReceived(void);

bool I2CReceiveOverflow(void);

bool I2CReceivedData(void);

bool I2CReceivedStop(void);

bool I2CReceivedRestart(void);

bool I2CRead(void);

bool I2CReceiveFull(void);

bool I2CTransmitInProgress(void);

void I2CTransmitRegister(uint8_t value);

uint8_t I2CReadReceiveRegister(void);

#endif

