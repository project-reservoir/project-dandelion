#include "i2c.h"
#include "stm32l0xx_hal.h"

void I2C_Configure()
{
    
}

// addr: the i2c address we want to communicate with
// rw: 1 if we are in read mode, 0 if we are in write mode
void I2C_Start(uint8_t addr, uint8_t rw)
{
    /* start I2C master transmission sequence */
    I2C1->CR1 = I2C_CR1_PE;
    I2C1->CR2 = (rw ? I2C_CR2_RD_WRN : 0) | (addr<<1);
    I2C1->CR2 |= I2C_CR2_START;
}

void I2C_Stop(void)
{    
    I2C1->CR2 = I2C_CR2_STOP;
}

void I2C_WaitForTX(void)
{
    while((I2C1->ISR & I2C_ISR_TXE) != I2C_ISR_TXE);
}

void I2C_WaitForRX(void)
{
    while((I2C1->ISR & I2C_ISR_RXNE) != I2C_ISR_RXNE);
}

// Write the byte over the I2C bus. Does not return until the byte has been written.
// Will not write the byte until the I2C device is ready.
void I2C_WriteByte(uint8_t byte)
{
    I2C_WaitForTX();
    
    I2C1->TXDR = byte;
    I2C1->CR2 |= (1<<16); // Write 1 byte
}

// Receive a byte over the I2C bus. Does not return until a byte is received
void I2C_ReadBytes(uint8_t* buffer, uint8_t numBytes)
{
    uint8_t i;
    I2C1->CR2 |= (numBytes<<16) | I2C_CR2_RD_WRN; // Read 1 byte
    
    for(i = 0; i < numBytes; i++)
    {
        I2C_WaitForRX();
        buffer[i] = I2C1->RXDR;
    }
}
