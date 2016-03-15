#ifndef _I2C_H
#define _I2C_H

#include "stm32l0xx_hal.h"

#define I2C_TIMEOUT_TICKS 500000

#define I2C_WRITE         0
#define I2C_READ          1

typedef enum I2C_Status_t {
    I2C_OK,
    I2C_TIMEOUT
} I2C_Status;

void            I2C_Reset(I2C_TypeDef* i2c);
void            I2C_Start(I2C_TypeDef* i2c, uint8_t addr, uint8_t rw);
void            I2C_Stop(I2C_TypeDef* i2c);
I2C_Status      I2C_WriteByte(I2C_TypeDef* i2c, uint8_t byte);
I2C_Status      I2C_WriteBytes(I2C_TypeDef* i2c, uint8_t* buffer, uint8_t numBytes);
I2C_Status      I2C_ReadBytes(I2C_TypeDef* i2c, uint8_t* buffer, uint8_t numBytes);
I2C_Status      I2C_WaitForTX(I2C_TypeDef* i2c);
I2C_Status      I2C_WaitForRX(I2C_TypeDef* i2c);
                
#endif // _I2C_H
