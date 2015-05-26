#include "hwctrl.h"
#include "stm32l0xx_hal.h"
#include "led_task.h"
#include "usb_task.h"
#include "coocox.h"

int main(void)
{
	// Initialize the ST Micro Board Support Library
    HAL_Init();

    // Configure the system clock
    SystemClock_Config();

    // Setup external ports on the MCU
    HwCtrl_Init();

    // Initialize the OS
    CoInitOS();

    // Create an LED blink task
    LedTaskCreate();

    // Create the USB Comm task
	//osThreadDef(USBTask, UsbTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);

    // Start scheduler
    CoStartOS();

    // We should never get here as control is now taken by the scheduler
    for(;;);
}
