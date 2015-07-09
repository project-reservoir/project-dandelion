#ifndef _I2C_H
#define _I2C_H

#include "stm32l0xx_hal.h"

void    I2C_Start(uint8_t addr, uint8_t rw);
void    I2C_Stop(void);
void    I2C_WriteByte(uint8_t byte);
void    I2C_WriteBytes(uint8_t* buffer, uint8_t numBytes);
void    I2C_ReadBytes(uint8_t* buffer, uint8_t numBytes);
void    I2C_WaitForTX(void);
void    I2C_WaitForRX(void);

#endif // _I2C_H
