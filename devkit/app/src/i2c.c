#include "i2c.h"
#include "stm32l0xx_hal.h"

void I2C_Reset(I2C_TypeDef* i2c)
{
    uint8_t i;
    i2c->CR1 = (i2c->CR1 & !I2C_CR1_PE);
    
    for(i = 0; i < 20; i++);
    
    i2c->CR1 |= I2C_CR1_PE;
}

void I2C_Configure(I2C_TypeDef* i2c)
{
    
}

// addr: the i2c address we want to communicate with
// rw: 1 if we are in read mode, 0 if we are in write mode
void I2C_Start(I2C_TypeDef* i2c, uint8_t addr, uint8_t rw)
{
    /* start I2C master transmission sequence */
    i2c->CR1 = I2C_CR1_PE;
    i2c->CR2 = (rw ? I2C_CR2_RD_WRN : 0) | (addr<<1);
    i2c->CR2 |= I2C_CR2_START;
}

void I2C_Stop(I2C_TypeDef* i2c)
{    
    i2c->CR2 = I2C_CR2_STOP;
}

I2C_Status I2C_WaitForTX(I2C_TypeDef* i2c)
{
    uint32_t i = 0;
    for(i = 0; i < I2C_TIMEOUT_TICKS && (i2c->ISR & I2C_ISR_TXE) != I2C_ISR_TXE; i++);
    
    if(i >= I2C_TIMEOUT_TICKS)
    {
        return I2C_TIMEOUT;
    }
    
    return I2C_OK;
}

I2C_Status I2C_WaitForRX(I2C_TypeDef* i2c)
{
    uint32_t i = 0;
    for(i = 0; i < I2C_TIMEOUT_TICKS && (i2c->ISR & I2C_ISR_RXNE) != I2C_ISR_RXNE; i++);
    
    if(i >= I2C_TIMEOUT_TICKS)
    {
        return I2C_TIMEOUT;
    }
    
    return I2C_OK;
}

// Write the byte over the I2C bus. Returns immediately: do not set the STOP condition until TX has finished.
// Will not write the byte until the I2C device is ready.
I2C_Status I2C_WriteByte(I2C_TypeDef* i2c, uint8_t byte)
{
    if(I2C_WaitForTX(i2c) != I2C_OK)
    {
        return I2C_TIMEOUT;
    }

    i2c->CR2 |= (1<<16); // Write n bytes
    i2c->TXDR = byte;
    
    return I2C_OK;
}

// Write the byte over the I2C bus. Does not return until the byte has been written.
// Will not write the byte until the I2C device is ready.
I2C_Status I2C_WriteBytes(I2C_TypeDef* i2c, uint8_t* buffer, uint8_t numBytes)
{
    uint8_t i;
    
    i2c->CR2 |= (numBytes<<16); // Write n bytes
    
    for(i = 0; i < numBytes; i++)
    {
        if(I2C_WaitForTX(i2c) != I2C_OK)
        {
            return I2C_TIMEOUT;
        }
        i2c->TXDR = buffer[i];
    }
    
    return I2C_OK;
}

// Receive a byte over the I2C bus. Does not return until a byte is received
I2C_Status I2C_ReadBytes(I2C_TypeDef* i2c, uint8_t* buffer, uint8_t numBytes)
{
    uint8_t i;
    i2c->CR2 |= (numBytes<<16) | I2C_CR2_RD_WRN; // Read 1 byte
    
    for(i = 0; i < numBytes; i++)
    {
        if(I2C_WaitForRX(i2c) != I2C_OK)
        {
            return I2C_TIMEOUT;
        }
        buffer[i] = i2c->RXDR;
    }
    
    return I2C_OK;
}
