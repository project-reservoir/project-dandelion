#include "app_header.h"
#include "system_interrupts.h"

extern uint32_t Image$$ER_IROM1$$Length;
extern uint32_t Image$$RW_IRAM1$$Length;
extern uint32_t __initial_sp;

// This hacky struct is generated by forcing the shim interrupt vectors into certain sections, 
// which have been named to guarantee that they will be placed at certain offsets in the flash image.
// These memory addresses CANNOT BE CHANGED unless the memory map is changed as well.

// IAR Embedded Workbench doesn't require this many hacks to get a ROPI vector table going. It will, 
// be default, generate address-relative function pointers when asked. 

// This struct is called __Vectors because Keil puts a "--first=__Vectors" command line arg into the linker command string
// and doesn't provide a way to change that.

const APP_HEADER __Vectors __attribute__((section("HEADER"))) __attribute__((used)) = {
	0,          // Space reserved in the image for a CRC32 checksum
	0xDEADBEEF, // the CRC32 start mark
	0x00000001, // test version
	sizeof(APP_HEADER) + 0x00000090, // Reset_Handler, the entry point for this application (this calls SystemInit and __main, which in turn calls the application main)
	(uint32_t)(&Image$$ER_IROM1$$Length), // ro_size
	(uint32_t)(&Image$$RW_IRAM1$$Length), // rw_size
    
    (uint32_t)(&__initial_sp),
	sizeof(APP_HEADER) + 0x00000000, // ADC1_COMP
    sizeof(APP_HEADER) + 0x00000008, // DebugMon_Handler
    sizeof(APP_HEADER) + 0x00000010, // DMA1_Channel1
    sizeof(APP_HEADER) + 0x00000018, // DMA1_Channel2_3
    sizeof(APP_HEADER) + 0x00000020, // DMA1_Channel4_5_6_7
    sizeof(APP_HEADER) + 0x00000028, // EXTI0_1
    sizeof(APP_HEADER) + 0x00000030, // EXTI2_3
    sizeof(APP_HEADER) + 0x00000038, // EXTI4_15
    sizeof(APP_HEADER) + 0x00000040, // FLASH
    sizeof(APP_HEADER) + 0x00000048, // HardFault_Handler
    sizeof(APP_HEADER) + 0x00000050, // I2C1
    sizeof(APP_HEADER) + 0x00000058, // I2C2
    sizeof(APP_HEADER) + 0x00000060, // LCD
    sizeof(APP_HEADER) + 0x00000068, // LPTIM1
    sizeof(APP_HEADER) + 0x00000070, // NMI_Handler
    sizeof(APP_HEADER) + 0x00000078, // PendSV_Handler
    sizeof(APP_HEADER) + 0x00000080, // PVD_Handler
    sizeof(APP_HEADER) + 0x00000088, // RCC_CRS
    sizeof(APP_HEADER) + 0x00000090, // Reset_Handler
    sizeof(APP_HEADER) + 0x00000098, // RNG_LPUART1
    sizeof(APP_HEADER) + 0x000000A0, // RTC_Handler
    sizeof(APP_HEADER) + 0x000000A8, // SPI1
    sizeof(APP_HEADER) + 0x000000B0, // SPI2
    sizeof(APP_HEADER) + 0x000000B8, // SVC_Handler
    sizeof(APP_HEADER) + 0x000000C0, // SysTick_Handler
    sizeof(APP_HEADER) + 0x000000C8, // TIM21
    sizeof(APP_HEADER) + 0x000000D0, // TIM22
    sizeof(APP_HEADER) + 0x000000D8, // TMI2_Handler
    sizeof(APP_HEADER) + 0x000000E0, // TIM6_DAC
    sizeof(APP_HEADER) + 0x000000E8, // TSC
    sizeof(APP_HEADER) + 0x000000F0, // USART1
    sizeof(APP_HEADER) + 0x000000F8, // USART2
    sizeof(APP_HEADER) + 0x00000100, // USB
    sizeof(APP_HEADER) + 0x00000108  // WWDG_Handler
};