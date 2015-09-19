#include "system_interrupts.h"
#include "cmsis_os.h"
#include "app_header.h"
#include "stm32l0xx_hal.h"
#include "radio.h"
#include "hwctrl.h"
#include "console.h"

extern I2C_HandleTypeDef I2CxHandle;
extern SPI_HandleTypeDef SpiHandle;
extern UART_HandleTypeDef UartHandle;

extern void __main(void);

void Reset_Handler(void)
{
	SystemInit();
	__main();
}

void SysTick_Handler(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
	}

	HAL_IncTick();
}

void I2C2_Handler(void)
{
    HAL_I2C_EV_IRQHandler(&I2CxHandle);
    HAL_I2C_ER_IRQHandler(&I2CxHandle);
}

void SPI1_Handler(void)
{
    if(SpiHandle.State != HAL_SPI_STATE_RESET)
    {
        HAL_SPI_IRQHandler(&SpiHandle);
    }
}

void EXTI4_15_Handler(void)
{
    /* EXTI line interrupt detected */
    if(__HAL_GPIO_EXTI_GET_IT(RADIO_NIRQ_PIN) != RESET) 
    { 
        __HAL_GPIO_EXTI_CLEAR_IT(RADIO_NIRQ_PIN);        
        SignalRadioIRQ();
    }
}

void EXTI0_1_Handler(void)
{
    uint8_t* buff;
    /* EXTI line interrupt detected */
    if(__HAL_GPIO_EXTI_GET_IT(KEY_BUTTON_PIN) != RESET)
    { 
        __HAL_GPIO_EXTI_CLEAR_IT(KEY_BUTTON_PIN);
        
        buff = pvPortMalloc(7);
        
        buff[0] = 'B';
        buff[1] = 'U';
        buff[2] = 'T';
        buff[3] = 'T';
        buff[4] = 'O';
        buff[5] = 'N';
        buff[6] = '1';
        
        SendToBroadcast(buff, 7);
    }
}

void USARTx_Handler(void)
{
    // This code is here to ensure that the function isn't inlined
    if(UartHandle.State != HAL_UART_STATE_ERROR)
    {
        HAL_UART_IRQHandler(&UartHandle);
    }
    else
    {
        HAL_UART_IRQHandler(&UartHandle);
    }
}

// These functions are the "static" handlers. They're placed at fixed memory addresses 
// and call the real handlers, which can be placed anywhere.
// DO NOT CHANGE THESE FUNCTIONS. THEY ALL COMPILE DOWN TO EXACTLY 8 BYTES EACH AND CHANGING
// THAT AMOUNT WILL CAUSE THE BOOTLOADER TO FAIL. LIKEWISE, DO NOT CREATE INTERRUPT HANDLERS 
// THAT COMPILE DOWN TO LESS OR MORE THAN 8 INSTRUCTIONS. THE COMPILER WILL OPTIMIZE THEM AWAY AND MESS
// UP THE ALIGNMENT OF THIS SECTION.

__attribute__((section("!!!!0.RESET_HANDLER"))) void Reset_Handler_Standin()
{
     Reset_Handler();
}

__attribute__((section("!!!!0.NMI_HANDLER"))) void NMI_Handler_Standin()
{
    NMI_Handler();
}

__attribute__((section("!!!!0.HARDFAULT_HANDLER"))) void HardFault_Handler_Standin()
{
    HardFault_Handler();
}

__attribute__((section("!!!!1.MEMMANAGE_HANDLER")))void MemManage_Handler_Standin()
{
    MemManage_Handler();
}

__attribute__((section("!!!!1.BUSFAULT_HANDLER")))void BusFault_Handler_Standin()
{
    BusFault_Handler();
}

__attribute__((section("!!!!1.USAGEFAULT_HANDLER")))void UsageFault_Handler_Standin()
{
    UsageFault_Handler();
}

__attribute__((section("!!!!0.SVC_HANDLER"))) void SVC_Handler_Standin()
{
    SVC_Handler();
}

__attribute__((section("!!!!0.DEBUGMON_HANDLER"))) void DebugMon_Handler_Standin()
{
    DebugMon_Handler();
}

__attribute__((section("!!!!0.PENDSV_HANDLER"))) void PendSV_Handler_Standin()
{
    PendSV_Handler();
}

__attribute__((section("!!!!0.SYSTICK_HANDLER"))) void SysTick_Handler_Standin()
{
    SysTick_Handler();
}

__attribute__((section("!!!!0.WWDG_HANDLER"))) void WWDG_Handler_Standin()
{
    WWDG_Handler();
}

__attribute__((section("!!!!0.PVD_HANDLER"))) void PVD_Handler_Standin()
{
    PVD_Handler();
}

__attribute__((section("!!!!0.RTC_HANDLER"))) void RTC_Handler_Standin()
{
    RTC_Handler();
}

__attribute__((section("!!!!0.FLASH_HANDLER"))) void FLASH_Handler_Standin()
{
    FLASH_Handler();
}

__attribute__((section("!!!!0.RCC_CRS_HANDLER"))) void RCC_CRS_Handler_Standin()
{
    RCC_CRS_Handler();
}

__attribute__((section("!!!!0.EXTI0_1_HANDLER"))) void EXTI0_1_Handler_Standin()
{
    EXTI0_1_Handler();
}

__attribute__((section("!!!!0.EXTI2_3_HANDLER"))) void EXTI2_3_Handler_Standin()
{
    EXTI2_3_Handler();
}

__attribute__((section("!!!!0.EXTI4_15_HANDLER"))) void EXTI4_15_Handler_Standin()
{
    EXTI4_15_Handler();
}

__attribute__((section("!!!!0.TSC_HANDLER"))) void TSC_Handler_Standin()
{
    TSC_Handler();
}

__attribute__((section("!!!!0.DMA1_CHANNEL11_HANDLER"))) void DMA1_Channel1_Handler_Standin()
{
    DMA1_Channel1_Handler();
}

__attribute__((section("!!!!0.DMA1_CHANNEL2_3_HANDLER"))) void DMA1_Channel2_3_Handler_Standin()
{
    DMA1_Channel2_3_Handler();
}

__attribute__((section("!!!!0.DMA1_CHANNEL4_5_6_7_HANDLER"))) void DMA1_Channel4_5_6_7_Handler_Standin()
{
    DMA1_Channel4_5_6_7_Handler();
}

__attribute__((section("!!!!0.ADC1_COMP_HANDLER"))) void ADC1_COMP_Handler_Standin()
{
    ADC1_COMP_Handler();
}

__attribute__((section("!!!!0.LPTIM1_HANDLER"))) void LPTIM1_Handler_Standin()
{
    LPTIM1_Handler();
}

__attribute__((section("!!!!0.TIM2_HANDLER"))) void TIM2_Handler_Standin()
{
    TIM2_Handler();
}

__attribute__((section("!!!!0.TIM6_HANDLER"))) void TIM6_DAC_Handler_Standin()
{
    TIM6_DAC_Handler();
}

__attribute__((section("!!!!0.TIM21_HANDLER"))) void TIM21_Handler_Standin()
{
    TIM21_Handler();
}

__attribute__((section("!!!!0.TIM22_HANDLER"))) void TIM22_Handler_Standin()
{
    TIM22_Handler();
}

__attribute__((section("!!!!0.I2C1_HANDLER"))) void I2C1_Handler_Standin()
{
    I2C1_Handler();
}

__attribute__((section("!!!!0.I2C2_HANDLER"))) void I2C2_Handler_Standin()
{
    I2C2_Handler();
}

__attribute__((section("!!!!0.SPI1_HANDLER"))) void SPI1_Handler_Standin()
{
    SPI1_Handler();
}

__attribute__((section("!!!!0.SPI2_HANDLER"))) void SPI2_Handler_Standin()
{
    SPI2_Handler();
}

__attribute__((section("!!!!0.USART1_HANDLER"))) void USART1_Handler_Standin()
{
    USART1_Handler();
}

__attribute__((section("!!!!0.USART2_HANDLER"))) void USART2_Handler_Standin()
{
    USART2_Handler();
}

__attribute__((section("!!!!0.RNG_LPUART1_HANDLER"))) void RNG_LPUART1_Handler_Standin()
{
    RNG_LPUART1_Handler();
}

__attribute__((section("!!!!0.LCD_HANDLER"))) void LCD_Handler_Standin()
{
    LCD_Handler();
}

__attribute__((section("!!!!0.USB_HANDLER"))) void USB_Handler_Standin()
{
    USB_Handler();
}
