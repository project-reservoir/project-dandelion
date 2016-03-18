#include "led_task.h"
#include "cmsis_os.h"
#include "stm32l0xx_hal.h"

void LedBlinkTaskHwInit(void)
{
    // Initialize All LED's
	GPIO_InitTypeDef  GPIO_InitStruct;

	LEDG_GPIO_CLK_ENABLE();
    LEDR_GPIO_CLK_ENABLE();
    LEDB_GPIO_CLK_ENABLE();

	GPIO_InitStruct.Pin = LEDG_PIN;
	GPIO_InitStruct.Mode = LED_GPIO_MODE;
	GPIO_InitStruct.Pull = LED_GPIO_PULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

	HAL_GPIO_Init(LEDG_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LEDR_PIN;
    HAL_GPIO_Init(LEDR_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LEDB_PIN;
    HAL_GPIO_Init(LEDB_GPIO_PORT, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(LEDG_GPIO_PORT, LEDG_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LEDR_GPIO_PORT, LEDR_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LEDB_GPIO_PORT, LEDB_PIN, GPIO_PIN_SET);
}


void LedBlinkTask(const void *argument)
{
    //"Chirp" the LED to indicate power on
    for(uint8_t i = 0; i < 20; i++)
    {
        HAL_GPIO_WritePin(LEDG_GPIO_PORT, LEDG_PIN, GPIO_PIN_RESET);
        //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        osDelay(50);
        
        //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LEDG_GPIO_PORT, LEDG_PIN, GPIO_PIN_SET);

        osDelay(50);
    }
    
	// Flash an LED on and off forever.
	while(1)
	{
		HAL_GPIO_WritePin(LEDG_GPIO_PORT, LEDG_PIN, GPIO_PIN_RESET);
        //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		osDelay(250);
        
        //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LEDG_GPIO_PORT, LEDG_PIN, GPIO_PIN_SET);

		osDelay(250);
	}
}