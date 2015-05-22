#include "led_task.h"
#include "cmsis_os.h"
#include "stm32l0xx_hal.h"

void LedBlinkTask(const void *argument)
{
	// Flash an LED on and off forever.
	while(1)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);

		osDelay(500);

		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);

		osDelay(500);
	}
}
