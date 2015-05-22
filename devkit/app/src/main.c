#include "hwctrl.h"
#include "stm32l0xx_hal.h"

int main(void)
{
	// Initialize the ST Micro Board Support Library
    HAL_Init();

    // Configure the system clock
    SystemClock_Config();

    // Setup external ports on the MCU
    HwCtrl_Init();

    // Flash an LED on and off forever.
    while(1)
    {
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);

    	HAL_Delay(500);

    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);

    	HAL_Delay(500);
    }
}
