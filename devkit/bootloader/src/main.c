#include "hwctrl.h"
#include "app_header.h"
#include "stm32l0xx_hal.h"
#include "stdbool.h"
#include "flash.h"

// DEFINES
#define ARM_THUMB_MODE 0x1

extern void launch(uint32_t stackPtr, uint32_t progCtr);

// GLOBAL VARIABLES
APP_HEADER* main_app = (APP_HEADER*)MAIN_APP_START;
APP_HEADER* backup_app = (APP_HEADER*)BACKUP_APP_START;

// This block has to be initialized to a non-zero value so that the linker doesn't try to glob it into
// the zero-initialized group, which this address is not a part of.
NVIC_BLOCK  ivt __attribute__((at(SRAM_BASE))) = {0xFF};

// LOCAL FUNCTION DECLARATIONS
void createIVT(APP_HEADER* app);
void launchImage(APP_HEADER* app);
uint32_t crc32(uint32_t crc, uint8_t *buf, uint32_t len);
void UpdateMainApp(void);
void SystemClock_Config(void);

// GLOBAL FUNCTIONS
int main(void)
{    
    __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_1);
    
    bool main_valid = (main_app->version != 0xFFFFFFFF && main_app->version != 0) && 
                      (main_app->image_size != 0xFFFFFFFF && main_app->image_size != 0) &&
                      ((MAIN_APP_START + main_app->image_size) < (MAIN_APP_START + APP_SIZE));
    
    bool backup_valid = (backup_app->version != 0xFFFFFFFF && backup_app->version != 0) && 
                        (backup_app->image_size != 0xFFFFFFFF && backup_app->image_size != 0) &&
                        ((BACKUP_APP_START + backup_app->image_size) < (BACKUP_APP_START + APP_SIZE));
    
    uint32_t main_app_crc32 = 0x00000000;
    uint32_t backup_app_crc32 = 0x00000000;
    
    if(main_valid)
    {
        main_app_crc32 = crc32(0x00000000, (uint8_t*)(MAIN_APP_START + 4), main_app->image_size - 4);
        main_valid = main_valid && ((main_app_crc32 == main_app->crc32));
    }
    
    if(backup_valid)
    {
        backup_app_crc32 = crc32(0x00000000, (uint8_t*)(BACKUP_APP_START + 4), backup_app->image_size - 4);
        backup_valid = backup_valid && ((backup_app_crc32 == backup_app->crc32));
    }
    
    if(main_valid && backup_valid)
    {
        if(main_app->version < backup_app->version)
        {
            UpdateMainApp();
        }
    }
    else if(backup_valid && !main_valid)
    {
        UpdateMainApp();
    }
    
    createIVT(main_app);
    launchImage(main_app);
    
    // TODO: blink a 'boot failure' code over the debug LED's
    for(;;);
}

void UpdateMainApp(void)
{        
    uint32_t updateBuffer[FLASH_PAGE_SIZE];
    
    flash_full_unlock();
    for(uint32_t i = 0; i < APP_SIZE; i += (FLASH_PAGE_SIZE * 4))
    {
        flash_erase(i + MAIN_APP_START);
        
        memcpy(updateBuffer, (uint32_t*)(BACKUP_APP_START + i), (FLASH_PAGE_SIZE * 4));
        
        flash_program(i + MAIN_APP_START, updateBuffer);
        flash_program(i + ((FLASH_PAGE_SIZE/2)*4) + MAIN_APP_START, &updateBuffer[FLASH_PAGE_SIZE/2]);
    }
}

// LOCAL FUNCTIONS
void createIVT(APP_HEADER* app)
{   
    // Set stack pointer to the same stack pointer 
    ivt.eram = app->initial_sp;
	ivt.Reset_Handler = (app->Reset_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.NMI_Handler = (app->NMI_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.HardFault_Handler = (app->HardFault_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.SVC_Handler = (app->SVC_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.DebugMon_Handler = (app->DebugMon_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.PendSV_Handler = (app->PendSV_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.SysTick_Handler = (app->SysTick_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.WWDG_Handler = (app->WWDG_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.PVD_Handler = (app->PVD_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.RTC_Handler = (app->RTC_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.FLASH_Handler = (app->FLASH_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.RCC_CRS_Handler = (app->RCC_CRS_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.EXTI0_1_Handler = (app->EXTI0_1_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.EXTI2_3_Handler = (app->EXTI2_3_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.EXTI4_15_Handler = (app->EXTI4_15_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.TSC_Handler = (app->TSC_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.DMA1_Channel1_Handler = (app->DMA1_Channel1_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.DMA1_Channel2_3_Handler = (app->DMA1_Channel2_3_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.DMA1_Channel4_5_6_7_Handler = (app->DMA1_Channel4_5_6_7_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.ADC1_COMP_Handler = (app->ADC1_COMP_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.LPTIM1_Handler = (app->LPTIM1_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.TIM2_Handler = (app->TIM2_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.TIM6_DAC_Handler = (app->TIM6_DAC_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.TIM21_Handler = (app->TIM21_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.TIM22_Handler = (app->TIM22_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.I2C1_Handler = (app->I2C1_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.I2C2_Handler = (app->I2C2_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.SPI1_Handler = (app->SPI1_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.SPI2_Handler = (app->SPI2_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.USART1_Handler = (app->USART1_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.USART2_Handler = (app->USART2_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.RNG_LPUART1_Handler = (app->RNG_LPUART1_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.LCD_Handler = (app->LCD_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.USB_Handler = (app->USB_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.BusFault_Handler = (app->BusFault_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.MemManage_Handler = (app->MemManage_Handler + (uint32_t)app) | ARM_THUMB_MODE;
    ivt.UsageFault_Handler = (app->UsageFault_Handler + (uint32_t)app) | ARM_THUMB_MODE;   
}

void launchImage(APP_HEADER* app)
{   
    uint32_t stackPtr = ivt.eram;
    uint32_t progCtr  = (app->entry_point + (uint32_t)app) | ARM_THUMB_MODE; 
    
    //Set NVIC to point to the new IVT
    SCB->VTOR = SRAM_BASE;
    
    launch(stackPtr, progCtr);
}

// Compute and return the CRC32
// This CRC32 should match the zlib.crc32 python routine
uint32_t crc32(uint32_t crc, uint8_t *buf, uint32_t len)
{
    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
    }
    return ~crc;
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif
