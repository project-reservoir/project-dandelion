#ifndef _FLASH_H
#define _FLASH_H

#include "stm32l0xx_hal.h"

#define FLASH_PAGE_SIZE 32

void flash_eeprom_unlock(void);
void flash_nvm_unlock(void);
void flash_eeprom_nvm_lock(void);
void flash_option_bytes_unlock(void);
void flash_option_byte_program(uint8_t index, uint16_t data);
void flash_erase_option_byte(uint8_t index);

// Erase a page of flash memory on the device
void flash_erase(uint32_t start_address);

// This function ALWAYS programs 16 words from the buffer pointer to flash. It runs in RAM,
// and does not return until the bytes have been programmed.
void flash_program(uint32_t start_address, uint32_t* buffer);

#endif // _FLASH_H
