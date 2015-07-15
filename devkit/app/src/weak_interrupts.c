#include "system_interrupts.h"

void Dummy_Handler(void)
{
	for(;;);
}

__weak void Reset_Handler()
{
		Dummy_Handler();
}

__weak void NMI_Handler()
{
		Dummy_Handler();
}

__weak void HardFault_Handler()
{
		Dummy_Handler();
}

__weak void MemManage_Handler()
{
		Dummy_Handler();
}

__weak void BusFault_Handler()
{
		Dummy_Handler();
}

__weak void UsageFault_Handler()
{
		Dummy_Handler();
}

__weak void SVC_Handler()
{
		Dummy_Handler();
}

__weak void DebugMon_Handler()
{
		Dummy_Handler();
}

__weak void SysTick_Handler()
{
		Dummy_Handler();
}

__weak void WWDG_Handler()
{
		Dummy_Handler();
}

__weak void PVD_Handler()
{
		Dummy_Handler();
}

__weak void RTC_Handler()
{
		Dummy_Handler();
}

__weak void FLASH_Handler()
{
		Dummy_Handler();
}

__weak void RCC_CRS_Handler()
{
		Dummy_Handler();
}

__weak void EXTI0_1_Handler()
{
		Dummy_Handler();
}

__weak void EXTI2_3_Handler()
{
		Dummy_Handler();
}

__weak void EXTI4_15_Handler()
{
		Dummy_Handler();
}

__weak void TSC_Handler()
{
		Dummy_Handler();
}

__weak void DMA1_Channel1_Handler()
{
		Dummy_Handler();
}

__weak void DMA1_Channel2_3_Handler()
{
		Dummy_Handler();
}

__weak void DMA1_Channel4_5_6_7_Handler()
{
		Dummy_Handler();
}

__weak void ADC1_COMP_Handler()
{
		Dummy_Handler();
}

__weak void LPTIM1_Handler()
{
		Dummy_Handler();
}

__weak void TIM2_Handler()
{
		Dummy_Handler();
}

__weak void TIM6_DAC_Handler()
{
		Dummy_Handler();
}

__weak void TIM21_Handler()
{
		Dummy_Handler();
}

__weak void TIM22_Handler()
{
		Dummy_Handler();
}

__weak void I2C1_Handler()
{
		Dummy_Handler();
}

__weak void I2C2_Handler()
{
		Dummy_Handler();
}

__weak void SPI1_Handler()
{
		Dummy_Handler();
}

__weak void SPI2_Handler()
{
		Dummy_Handler();
}

__weak void USART1_Handler()
{
		Dummy_Handler();
}

__weak void USART2_Handler()
{
		Dummy_Handler();
}

__weak void RNG_LPUART1_Handler()
{
		Dummy_Handler();
}

__weak void LCD_Handler()
{
		Dummy_Handler();
}

__weak void USB_Handler()
{
		Dummy_Handler();
}
