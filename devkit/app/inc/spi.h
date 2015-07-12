#ifndef _SPI_H
#define _SPI_H

#include "stm32l0xx_hal.h"

#define SPI_TIMEOUT_TICKS 5000000
#define SPI_BUSY_WAIT_EXTRA 80

typedef enum SPI_Status_t {
    SPI_OK,
    SPI_TIMEOUT
} SPI_Status;

void SPI_WaitForNotBusy(void);
void SPI_WaitForTX(void);
void SPI_WaitForRX(void);
void SPI_WriteByte(uint8_t byte);
void SPI_WriteBytes(uint8_t* bytes, uint8_t len);
void SPI_ReadByte(uint8_t* byte);
void SPI_ReadBytes(uint8_t* bytes, uint8_t len);
void SPI_WriteReadBytes(uint8_t* writeBytes, uint8_t* readBytes, uint8_t len);

#endif // _SPI_H
