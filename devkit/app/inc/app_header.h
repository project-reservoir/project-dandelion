#pragma once

#define SRAM_START			0x20000000
#define FLASH_START			0x08000000
#define MAIN_APP_START		0x08000800
#define BACKUP_APP_START    0x08008400

typedef struct APP_HEADER_T {
	uint32_t crc32;
	uint32_t crc32_start_mark;
	uint32_t version;
	uint32_t entry_point;
	uint32_t ro_size;

	uint32_t Reset_Handler;
	uint32_t NMI_Handler;
	uint32_t HardFault_Handler;
	uint32_t SVC_Handler;
	uint32_t DebugMon_Handler;
	uint32_t PendSV_Handler;
	uint32_t SysTick_Handler;
	uint32_t WWDG_IRQHandler;                /* Window WatchDog              */
	uint32_t PVD_IRQHandler;                    /* PVD through EXTI Line detection */
	uint32_t RTC_IRQHandler;                    /* RTC through the EXTI line     */
	uint32_t FLASH_IRQHandler;                  /* FLASH                        */
	uint32_t RCC_CRS_IRQHandler;                /* RCC and CRS                  */
	uint32_t EXTI0_1_IRQHandler;                /* EXTI Line 0 and 1            */
	uint32_t EXTI2_3_IRQHandler;                /* EXTI Line 2 and 3            */
	uint32_t EXTI4_15_IRQHandler;               /* EXTI Line 4 to 15            */
	uint32_t TSC_IRQHandler;                     /* TSC                           */
	uint32_t DMA1_Channel1_IRQHandler;          /* DMA1 Channel 1               */
	uint32_t DMA1_Channel2_3_IRQHandler;        /* DMA1 Channel 2 and Channel 3 */
	uint32_t DMA1_Channel4_5_6_7_IRQHandler;    /* DMA1 Channel 4; Channel 5; Channel 6 and Channel 7*/
	uint32_t ADC1_COMP_IRQHandler;              /* ADC1; COMP1 and COMP2        */
	uint32_t LPTIM1_IRQHandler;                 /* LPTIM1                       */
	uint32_t TIM2_IRQHandler;                   /* TIM2                         */
	uint32_t TIM6_DAC_IRQHandler;               /* TIM6 and DAC                 */
	uint32_t TIM21_IRQHandler;                  /* TIM21                        */
	uint32_t TIM22_IRQHandler;                  /* TIM22                        */
	uint32_t I2C1_IRQHandler;                   /* I2C1                         */
	uint32_t I2C2_IRQHandler;                   /* I2C2                         */
	uint32_t SPI1_IRQHandler;                   /* SPI1                         */
	uint32_t SPI2_IRQHandler;                   /* SPI2                         */
	uint32_t USART1_IRQHandler;                 /* USART1                       */
	uint32_t USART2_IRQHandler;                 /* USART2                       */
	uint32_t RNG_LPUART1_IRQHandler;            /* RNG and LPUART1              */
	uint32_t LCD_IRQHandler;                    /* LCD                          */
	uint32_t USB_IRQHandler;                    /* USB                          */
} APP_HEADER;

typedef struct NVIC_BLOCK_T {
	uint32_t eram;
	uint32_t Reset_Handler;
	uint32_t NMI_Handler;
	uint32_t HardFault_Handler;

	uint32_t reserved_0;
	uint32_t reserved_1;
	uint32_t reserved_2;
	uint32_t reserved_3;
	uint32_t reserved_4;
	uint32_t reserved_5;
	uint32_t reserved_6;

	uint32_t SVC_Handler;
	uint32_t DebugMon_Handler;

	uint32_t reserved_7;

	uint32_t PendSV_Handler;
	uint32_t SysTick_Handler;
	uint32_t WWDG_IRQHandler;                /* Window WatchDog              */
	uint32_t PVD_IRQHandler;                    /* PVD through EXTI Line detection */
	uint32_t RTC_IRQHandler;                    /* RTC through the EXTI line     */
	uint32_t FLASH_IRQHandler;                  /* FLASH                        */
	uint32_t RCC_CRS_IRQHandler;                /* RCC and CRS                  */
	uint32_t EXTI0_1_IRQHandler;                /* EXTI Line 0 and 1            */
	uint32_t EXTI2_3_IRQHandler;                /* EXTI Line 2 and 3            */
	uint32_t EXTI4_15_IRQHandler;               /* EXTI Line 4 to 15            */
	uint32_t TSC_IRQHandler;                     /* TSC                           */
	uint32_t DMA1_Channel1_IRQHandler;          /* DMA1 Channel 1               */
	uint32_t DMA1_Channel2_3_IRQHandler;        /* DMA1 Channel 2 and Channel 3 */
	uint32_t DMA1_Channel4_5_6_7_IRQHandler;    /* DMA1 Channel 4; Channel 5; Channel 6 and Channel 7*/
	uint32_t ADC1_COMP_IRQHandler;              /* ADC1; COMP1 and COMP2        */
	uint32_t LPTIM1_IRQHandler;                 /* LPTIM1                       */

	uint32_t reserved_8;

	uint32_t TIM2_IRQHandler;                   /* TIM2                         */

	uint32_t reserved_9;

	uint32_t TIM6_DAC_IRQHandler;               /* TIM6 and DAC                 */

	uint32_t reserved_10;
	uint32_t reserved_11;

	uint32_t TIM21_IRQHandler;                  /* TIM21                        */

	uint32_t reserved_12;

	uint32_t TIM22_IRQHandler;                  /* TIM22                        */
	uint32_t I2C1_IRQHandler;                   /* I2C1                         */
	uint32_t I2C2_IRQHandler;                   /* I2C2                         */
	uint32_t SPI1_IRQHandler;                   /* SPI1                         */
	uint32_t SPI2_IRQHandler;                   /* SPI2                         */
	uint32_t USART1_IRQHandler;                 /* USART1                       */
	uint32_t USART2_IRQHandler;                 /* USART2                       */
	uint32_t RNG_LPUART1_IRQHandler;            /* RNG and LPUART1              */
	uint32_t LCD_IRQHandler;                    /* LCD                          */
	uint32_t USB_IRQHandler;                    /* USB                          */
} NVIC_BLOCK;
