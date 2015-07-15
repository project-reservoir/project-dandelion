/**
  ******************************************************************************
  * @file    stm32l0xx_it.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    18-June-2014
  * @brief   This file contains the headers of the interrupt handlers.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32L0xx_IT_H
#define __STM32L0xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Dummy_Handler(void) __attribute__((used));
     
void NMI_Handler(void) __attribute__((used));
void HardFault_Handler(void) __attribute__((used));
void MemManage_Handler(void) __attribute__((used));
void BusFault_Handler(void) __attribute__((used));
void UsageFault_Handler(void) __attribute__((used));
void SVC_Handler(void) __attribute__((used));
void DebugMon_Handler(void) __attribute__((used));
void PendSV_Handler(void) __attribute__((used));
void SysTick_Handler(void) __attribute__((used));
void Reset_Handler(void) __attribute__((used));
void WWDG_Handler(void) __attribute__((used));
void PVD_Handler(void) __attribute__((used));
void RTC_Handler(void) __attribute__((used));
void FLASH_Handler(void) __attribute__((used));
void RCC_CRS_Handler(void) __attribute__((used));
void EXTI0_1_Handler(void) __attribute__((used));
void EXTI2_3_Handler(void) __attribute__((used));
void EXTI4_15_Handler(void) __attribute__((used));
void TSC_Handler(void) __attribute__((used));
void DMA1_Channel1_Handler(void) __attribute__((used));
void DMA1_Channel2_3_Handler(void) __attribute__((used));
void DMA1_Channel4_5_6_7_Handler(void) __attribute__((used));
void ADC1_COMP_Handler(void) __attribute__((used));
void LPTIM1_Handler(void) __attribute__((used));
void TIM2_Handler(void) __attribute__((used));
void TIM6_DAC_Handler(void) __attribute__((used));
void TIM21_Handler(void) __attribute__((used));
void TIM22_Handler(void) __attribute__((used));
void I2C1_Handler(void) __attribute__((used));
void I2C2_Handler(void) __attribute__((used));
void SPI1_Handler(void) __attribute__((used));
void SPI2_Handler(void) __attribute__((used));
void USART1_Handler(void) __attribute__((used));
void USART2_Handler(void) __attribute__((used));
void RNG_LPUART1_Handler(void) __attribute__((used));
void LCD_Handler(void) __attribute__((used));
void USB_Handler(void) __attribute__((used));

void NMI_Handler_Standin(void) __attribute__((used));
void HardFault_Handler_Standin(void) __attribute__((used));
void MemManage_Handler_Standin(void) __attribute__((used));
void BusFault_Handler_Standin(void) __attribute__((used));
void UsageFault_Handler_Standin(void) __attribute__((used));
void PendSV_Handler_Standin(void) __attribute__((used));
void SVC_Handler_Standin(void) __attribute__((used));
void DebugMon_Handler_Standin(void) __attribute__((used));
void SysTick_Handler_Standin(void) __attribute__((used));
void Reset_Handler_Standin(void) __attribute__((used));
void WWDG_Handler_Standin(void) __attribute__((used));
void PVD_Handler_Standin(void) __attribute__((used));
void RTC_Handler_Standin(void) __attribute__((used));
void FLASH_Handler_Standin(void) __attribute__((used));
void RCC_CRS_Handler_Standin(void) __attribute__((used));
void EXTI0_1_Handler_Standin(void) __attribute__((used));
void EXTI2_3_Handler_Standin(void) __attribute__((used));
void EXTI4_15_Handler_Standin(void) __attribute__((used));
void TSC_Handler_Standin(void) __attribute__((used));
void DMA1_Channel1_Handler_Standin(void) __attribute__((used));
void DMA1_Channel2_3_Handler_Standin(void) __attribute__((used));
void DMA1_Channel4_5_6_7_Handler_Standin(void) __attribute__((used));
void ADC1_COMP_Handler_Standin(void) __attribute__((used));
void LPTIM1_Handler_Standin(void) __attribute__((used));
void TIM2_Handler_Standin(void) __attribute__((used));
void TIM6_DAC_Handler_Standin(void) __attribute__((used));
void TIM21_Handler_Standin(void)__attribute__((used));
void TIM22_Handler_Standin(void) __attribute__((used));
void I2C1_Handler_Standin(void) __attribute__((used));
void I2C2_Handler_Standin(void) __attribute__((used));
void SPI1_Handler_Standin(void) __attribute__((used));
void SPI2_Handler_Standin(void) __attribute__((used));
void USART1_Handler_Standin(void) __attribute__((used));
void USART2_Handler_Standin(void) __attribute__((used));
void RNG_LPUART1_Handler_Standin(void) __attribute__((used));
void LCD_Handler_Standin(void) __attribute__((used));
void USB_Handler_Standin(void) __attribute__((used));

#ifdef __cplusplus
}
#endif

#endif /* __STM32L0xx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
