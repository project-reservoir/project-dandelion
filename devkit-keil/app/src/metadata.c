#include "app_header.h"
#include "system_interrupts.h"

extern int main(void);

extern uint32_t Image$$ER_RO$$Length;

const APP_HEADER __Vectors = {
	0, // uint32_t crc32;
	0xDEADBEEF, // uint32_t crc32_start_mark;
	0x00000001, // uint32_t version;
	(uint32_t)main, // uint32_t entry_point;
	(uint32_t)(&Image$$ER_RO$$Length), // uint32_t ro_size;

	(uint32_t)Reset_Handler,
	(uint32_t)NMI_Handler,
	(uint32_t)HardFault_Handler,
	(uint32_t)SVC_Handler,
	(uint32_t)DebugMon_Handler,
	(uint32_t)PendSV_Handler,
	(uint32_t)SysTick_Handler,
	(uint32_t)WWDG_Handler,
	(uint32_t)PVD_Handler,
	(uint32_t)RTC_Handler, 
	(uint32_t)FLASH_Handler,
	(uint32_t)RCC_CRS_Handler,
	(uint32_t)EXTI0_1_Handler,         
	(uint32_t)EXTI2_3_Handler,         
	(uint32_t)EXTI4_15_Handler,        
	(uint32_t)TSC_Handler,        
	(uint32_t)DMA1_Channel1_Handler,
	(uint32_t)DMA1_Channel2_3_Handler, 
	(uint32_t)DMA1_Channel4_5_6_7_Handler, 
	(uint32_t)ADC1_COMP_Handler,    
	(uint32_t)LPTIM1_Handler,              
	(uint32_t)TIM2_Handler,                 
	(uint32_t)TIM6_DAC_Handler,            
	(uint32_t)TIM21_Handler,               
	(uint32_t)TIM22_Handler,                  
	(uint32_t)I2C1_Handler,                  
	(uint32_t)I2C2_Handler,                   
	(uint32_t)SPI1_Handler,                   
	(uint32_t)SPI2_Handler,                   
	(uint32_t)USART1_Handler,                
	(uint32_t)USART2_Handler,                 
	(uint32_t)RNG_LPUART1_Handler,            
	(uint32_t)LCD_Handler,            
	(uint32_t)USB_Handler
};

