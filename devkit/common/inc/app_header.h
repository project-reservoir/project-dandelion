#ifndef __APP_HEADER_H
#define __APP_HEADER_H

#include "stdint.h"

#define xstr(s) str(s)
#define str(s) #s

// App Version format:  0xAABBCCDD
//      AA = OS Major Revision
//      BB = OS Minor Revision
//      CC = HW ID (01 = DEVKIT, 02 = EVT1, 03 = EVT2)
//      DD = Build type (01 = DEBUG, 02 = PRODUCTION)

#define OS_MAJOR            4
#define OS_MINOR            10
#define HW_ID               2
#define BUILD_TYPE          1

#define APP_VERSION         ((OS_MAJOR << 24) | (OS_MINOR << 16) | (HW_ID << 8) | (BUILD_TYPE))
#define APP_VERSION_STR     xstr(APP_VERSION)

#define SRAM_START				0x20000000
#define FLASH_START				0x08000000
#define MAIN_APP_START		    0x08000800
#define BACKUP_APP_START        0x08008400
#define APP_SIZE                0x00007C00

typedef struct APP_HEADER_T {
	uint32_t crc32;
	uint32_t crc32_start_mark;
	uint32_t version;
	uint32_t entry_point;
	uint32_t image_size;
	uint32_t reserved;
    
    uint32_t initial_sp;
    
    // Note: these are sorted alphabetically (rather than in vector table order) 
    // due to a quirk of the linker.
    
	uint32_t ADC1_COMP_Handler;              /* ADC1; COMP1 and COMP2        */
    uint32_t DebugMon_Handler;
    uint32_t DMA1_Channel1_Handler;          /* DMA1 Channel 1               */
    uint32_t DMA1_Channel2_3_Handler;        /* DMA1 Channel 2 and Channel 3 */
    uint32_t DMA1_Channel4_5_6_7_Handler;    /* DMA1 Channel 4; Channel 5; Channel 6 and Channel 7*/
    uint32_t EXTI0_1_Handler;                /* EXTI Line 0 and 1            */
    uint32_t EXTI2_3_Handler;                /* EXTI Line 2 and 3            */
    uint32_t EXTI4_15_Handler;               /* EXTI Line 4 to 15            */
    uint32_t FLASH_Handler;                  /* FLASH                        */
    uint32_t HardFault_Handler;
    uint32_t I2C1_Handler;                   /* I2C1                         */
    uint32_t I2C2_Handler;                   /* I2C2                         */
    uint32_t LCD_Handler;                    /* LCD                          */
    uint32_t LPTIM1_Handler;                 /* LPTIM1                       */
    uint32_t NMI_Handler;
    uint32_t Fake_PendSV_Handler;
    uint32_t PVD_Handler;                    /* PVD through EXTI Line detection */
    uint32_t RCC_CRS_Handler;                /* RCC and CRS                  */
    uint32_t Reset_Handler;
    uint32_t RNG_LPUART1_Handler;            /* RNG and LPUART1              */
    uint32_t RTC_Handler;                    /* RTC through the EXTI line     */
    uint32_t SPI1_Handler;                   /* SPI1                         */
    uint32_t SPI2_Handler;                   /* SPI2                         */
    uint32_t SVC_Handler;
    uint32_t SysTick_Handler;
    uint32_t TIM21_Handler;                  /* TIM21                        */
    uint32_t TIM22_Handler;                  /* TIM22                        */
    uint32_t TIM2_Handler;                   /* TIM2                         */
    uint32_t TIM6_DAC_Handler;               /* TIM6 and DAC                 */
    uint32_t TSC_Handler;                    /* TSC                         */
    uint32_t USART1_Handler;                 /* USART1                       */
    uint32_t USART2_Handler;                 /* USART2                       */
    uint32_t USB_Handler;                    /* USB                          */
    uint32_t WWDG_Handler;                   /* Window WatchDog              */
    uint32_t BusFault_Handler;
    uint32_t MemManage_Handler;
    uint32_t UsageFault_Handler;
    uint32_t PendSV_Handler;                // This is the real location of the PendSV_Handler. We have to use a fake one above because the Keil compiler is stupid.
    
} APP_HEADER;

typedef struct NVIC_BLOCK_T {
	uint32_t eram;
	uint32_t Reset_Handler;
	uint32_t NMI_Handler;
	uint32_t HardFault_Handler;
    uint32_t MemManage_Handler;
	uint32_t BusFault_Handler;
	uint32_t UsageFault_Handler;
	
	uint32_t reserved_3;
	uint32_t reserved_4;
	uint32_t reserved_5;
	uint32_t reserved_6;

	uint32_t SVC_Handler;
	uint32_t DebugMon_Handler;

	uint32_t reserved_7;

	uint32_t PendSV_Handler;
	uint32_t SysTick_Handler;
	uint32_t WWDG_Handler;                /* Window WatchDog              */
	uint32_t PVD_Handler;                    /* PVD through EXTI Line detection */
	uint32_t RTC_Handler;                    /* RTC through the EXTI line     */
	uint32_t FLASH_Handler;                  /* FLASH                        */
	uint32_t RCC_CRS_Handler;                /* RCC and CRS                  */
	uint32_t EXTI0_1_Handler;                /* EXTI Line 0 and 1            */
	uint32_t EXTI2_3_Handler;                /* EXTI Line 2 and 3            */
	uint32_t EXTI4_15_Handler;               /* EXTI Line 4 to 15            */
	uint32_t TSC_Handler;                     /* TSC                           */
	uint32_t DMA1_Channel1_Handler;          /* DMA1 Channel 1               */
	uint32_t DMA1_Channel2_3_Handler;        /* DMA1 Channel 2 and Channel 3 */
	uint32_t DMA1_Channel4_5_6_7_Handler;    /* DMA1 Channel 4; Channel 5; Channel 6 and Channel 7*/
	uint32_t ADC1_COMP_Handler;              /* ADC1; COMP1 and COMP2        */
	uint32_t LPTIM1_Handler;                 /* LPTIM1                       */

	uint32_t reserved_8;

	uint32_t TIM2_Handler;                   /* TIM2                         */

	uint32_t reserved_9;

	uint32_t TIM6_DAC_Handler;               /* TIM6 and DAC                 */

	uint32_t reserved_10;
	uint32_t reserved_11;

	uint32_t TIM21_Handler;                  /* TIM21                        */

	uint32_t reserved_12;

	uint32_t TIM22_Handler;                  /* TIM22                        */
	uint32_t I2C1_Handler;                   /* I2C1                         */
	uint32_t I2C2_Handler;                   /* I2C2                         */
	uint32_t SPI1_Handler;                   /* SPI1                         */
	uint32_t SPI2_Handler;                   /* SPI2                         */
	uint32_t USART1_Handler;                 /* USART1                       */
	uint32_t USART2_Handler;                 /* USART2                       */
	uint32_t RNG_LPUART1_Handler;            /* RNG and LPUART1              */
	uint32_t LCD_Handler;                    /* LCD                          */
	uint32_t USB_Handler;                    /* USB                          */
} NVIC_BLOCK;

#endif // __APP_HEADER_H
