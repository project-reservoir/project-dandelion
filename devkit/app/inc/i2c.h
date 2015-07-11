#ifndef _I2C_H
#define _I2C_H

#include "stm32l0xx_hal.h"

#define I2C_TIMEOUT_TICKS 5000000

typedef enum I2C_Status_t {
    I2C_OK,
    I2C_TIMEOUT
} I2C_Status;

void            I2C_Reset(void);
void            I2C_Start(uint8_t addr, uint8_t rw);
void            I2C_Stop(void);
I2C_Status      I2C_WriteByte(uint8_t byte);
I2C_Status      I2C_WriteBytes(uint8_t* buffer, uint8_t numBytes);
I2C_Status      I2C_ReadBytes(uint8_t* buffer, uint8_t numBytes);
I2C_Status      I2C_WaitForTX(void);
I2C_Status      I2C_WaitForRX(void);
                
#endif // _I2C_H
