#include "flash.h"

void flash_eeprom_unlock(void)
{
    /* (1) Wait till no operation is on going */
    /* (2) Check if the PELOCK is unlocked */
    /* (3) Perform unlock sequence */
    while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */
    {
        /* For robust implementation, add here time-out management */
    }
    if ((FLASH->PECR & FLASH_PECR_PELOCK) != 0) /* (2) */
    {
        FLASH->PEKEYR = FLASH_PEKEY1; /* (3) */
        FLASH->PEKEYR = FLASH_PEKEY2;
    }
}

void flash_eeprom_nvm_lock(void)
{
    /* (1) Wait till no operation is on going */
    /* (2) Locks the NVM by setting PELOCK in PECR */
    while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */
    {
        /* For robust implementation, add here time-out management */
    }
    FLASH->PECR |= FLASH_PECR_PELOCK; /* (2) */
}

void flash_nvm_unlock(void)
{
    /* (1) Wait till no operation is on going */
    /* (2) Check that the PELOCK is unlocked */
    /* (3) Check if the PRGLOCK is unlocked */
    /* (4) Perform unlock sequence */
    while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */
    {
        /* For robust implementation, add here time-out management */
    }
    if ((FLASH->PECR & FLASH_PECR_PELOCK) == 0) /* (2) */
    {
        if ((FLASH->PECR & FLASH_PECR_PRGLOCK) != 0) /* (3) */
        {
            FLASH->PRGKEYR = FLASH_PRGKEY1; /* (4) */
            FLASH->PRGKEYR = FLASH_PRGKEY2;
        }
    }
}

void flash_option_bytes_unlock(void)
{
    /* (1) Wait till no operation is on going */
    /* (2) Check that the PELOCK is unlocked */
    /* (3) Check if the OPTLOCK is unlocked */
    /* (4) Perform unlock sequence */
    while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */
    {
        /* For robust implementation, add here time-out management */
    }
    if ((FLASH->PECR & FLASH_PECR_PELOCK) == 0) /* (2) */
    {
        if ((FLASH->PECR & FLASH_PECR_OPTLOCK) != 0) /* (2) */
        {
            FLASH->OPTKEYR = FLASH_OPTKEY1; /* (3) */
            FLASH->OPTKEYR = FLASH_OPTKEY2;
        }
    }
}

void flash_option_byte_program(uint8_t index, uint16_t data)
{
    /* (1) Write a 32-bit word value at the option byte address,
    the 16-bit data is extended with its compemented value */
    /* (3) Wait until the BSY bit is reset in the FLASH_SR register */
    /* (4) Check the EOP flag in the FLASH_SR register */
    /* (5) Clear EOP flag by software by writing EOP at 1 */
    *(__IO uint32_t *)(OB_BASE + index) = (uint32_t)((~data << 16) | data);
    /* (1) */
    while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (2) */
    {
        /* For robust implementation, add here time-out management */
    }
    if ((FLASH->SR & FLASH_SR_EOP) != 0) /* (3) */
    {
        FLASH->SR |= FLASH_SR_EOP; /* (4) */
    }
    else
    {
        /* Manage the error cases */
    }
}

void flash_erase_option_byte(uint8_t index)
{
    /* (1) Set the ERASE bit in the FLASH_PECR register
    to enable option byte erasing */
    /* (2) Write a 32-bit word value at the option byte address to be erased
    to start the erase sequence */
    /* (3) Wait until the BSY bit is reset in the FLASH_SR register */
    /* (4) Check the EOP flag in the FLASH_SR register */
    /* (5) Clear EOP flag by software by writing EOP at 1 */
    /* (6) Reset the ERASE and PROG bits in the FLASH_PECR register
    to disable the page erase */
    FLASH->PECR |= FLASH_PECR_ERASE; /* (1) */
    *(__IO uint32_t *)(OB_BASE + index) = 0; /* (2) */
    while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (3) */
    {
        /* For robust implementation, add here time-out management */
    }
    if ((FLASH->SR & FLASH_SR_EOP) != 0) /* (4) */
    {
        FLASH->SR |= FLASH_SR_EOP; /* (5) */
    }
    else
    {
        /* Manage the error cases */
    }
    FLASH->PECR &= ~(FLASH_PECR_ERASE); /* (6) */
}

void flash_erase(uint32_t page_addr)
{
    /* (1) Set the ERASE and PROG bits in the FLASH_PECR register
    to enable page erasing */
    /* (2) Write a 32-bit word value in an address of the selected page
    to start the erase sequence */
    /* (3) Wait until the BSY bit is reset in the FLASH_SR register */
    /* (4) Check the EOP flag in the FLASH_SR register */
    /* (5) Clear EOP flag by software by writing EOP at 1 */
    /* (6) Reset the ERASE and PROG bits in the FLASH_PECR register
    to disable the page erase */
    FLASH->PECR |= FLASH_PECR_ERASE | FLASH_PECR_PROG; /* (1) */
    *(__IO uint32_t *)page_addr = (uint32_t)0; /* (2) */
    while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (3) */
    {
        /* For robust implementation, add here time-out management */
    }
    if ((FLASH->SR & FLASH_SR_EOP) != 0) /* (4) */
    {
        FLASH->SR |= FLASH_SR_EOP; /* (5) */
    }
    else
    {
        /* Manage the error cases */
    }
    FLASH->PECR &= ~(FLASH_PECR_ERASE | FLASH_PECR_PROG); /* (6) */
}

/**
* This function erases a page of flash.
* The Page Erase bit (PER) is set at the beginning and reset
* at the end of the function, in case of successive erase,
* these two operations could be performed outside the function.
* Param page_addr is an address inside the page to erase
* Retval None
*/


void flash_program(uint32_t start_address, uint32_t* buffer)
{
    uint8_t i;
    /* (1) Set the PROG and FPRG bits in the FLASH_PECR register
    to enable a half page programming */
    /* (2) Perform the data write (half-page) at the desired address */
    /* (3) Wait until the BSY bit is reset in the FLASH_SR register */
    /* (4) Check the EOP flag in the FLASH_SR register */
    /* (5) clear it by software by writing it at 1 */
    /* (6) Reset the PROG and FPRG bits to disable programming */
    FLASH->PECR |= FLASH_PECR_PROG | FLASH_PECR_FPRG; /* (1) */
    for (i = 0; i < ((FLASH_PAGE_SIZE/2) * 4); i+=4, buffer++)
    {
        *(__IO uint32_t*)(start_address + i) = *buffer; /* (2) */
    }
    while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (3) */
    {
        /* For robust implementation, add here time-out management */
    }
    if ((FLASH->SR & FLASH_SR_EOP) != 0) /* (4) */
    {
        FLASH->SR |= FLASH_SR_EOP; /* (5) */
    }
    else
    {
        /* Manage the error cases */
        
    }
    FLASH->PECR &= ~(FLASH_PECR_PROG | FLASH_PECR_FPRG); /* (6) */
}
