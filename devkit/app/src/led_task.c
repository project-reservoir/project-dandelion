#include "led_task.h"
#include "cmsis_os.h"
#include "stm32l0xx_hal.h"

void LedBlinkTaskHwInit(void)
{
    // Initialize LED3 and LED4
	GPIO_InitTypeDef  GPIO_InitStruct;

	/* Enable the GPIO_LED Clock */
	__GPIOB_CLK_ENABLE();

	/* Configure the GPIO_LED pin */
	GPIO_InitStruct.Pin = LED3_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

	HAL_GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_RESET);

	/*__GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = LED4_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

	HAL_GPIO_Init(LED4_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(LED4_GPIO_PORT, LED4_PIN, GPIO_PIN_RESET);*/
}


void LedBlinkTask(const void *argument)
{
	// Flash an LED on and off forever.
	while(1)
	{
		HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_RESET);
        //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		osDelay(100);
        
        //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_SET);

		osDelay(100);
	}
}