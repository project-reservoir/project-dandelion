#include "led_task.h"
#include "coocox.h"
#include "stm32l0xx_hal.h"

// GLOBAL VARIABLES
OS_STK led_task_stack[LED_TASK_STACK_SIZE];

// LOCAL FUNCTION PROTOTYPES
static void LedTask(const void *argument);

// GLOBAL FUNCTION IMPLEMENTATIONS
void LedTaskCreate(void)
{
	CoCreateTask(LedTask, 0, 0, &led_task_stack[LED_TASK_STACK_SIZE-1], LED_TASK_STACK_SIZE);
}

// LOCAL FUNCTION IMPLEMENTATIONS
void LedTask(const void *argument)
{
	// Flash an LED on and off forever.
	while(1)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);

		CoTickDelay(100);

		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);

		CoTickDelay(100);
	}
}
