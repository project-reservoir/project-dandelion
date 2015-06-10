#include "hwctrl.h"
#include "stm32l0xx_hal.h"
#include "led_task.h"
#include "usb_task.h"
#include "cmsis_os.h"

unsigned portBASE_TYPE makeFreeRtosPriority (osPriority priority)
{
  unsigned portBASE_TYPE fpriority = tskIDLE_PRIORITY;
  
  if (priority != osPriorityError) {
    fpriority += (priority - osPriorityIdle);
  }
  
  return fpriority;
}

int main(void)
{	
	xTaskHandle ledTaskHandle;
    xTaskHandle ledTask2Handle;

    // Configure the system clock
    SystemClock_Config();
    
    // Setup external ports on the MCU
    HwCtrl_Init();
    
    // Initialize the ST Micro Board Support Library
    HAL_Init();
    
    // Flash an LED on and off forever.
	/*while(1)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);

		HAL_Delay(1000);

		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);

		HAL_Delay(1000);
	}*/

    // Create an LED blink tasks	
    xTaskCreate(LedBlinkTask,
                "LEDTask",
                configMINIMAL_STACK_SIZE,
                NULL,
                makeFreeRtosPriority(osPriorityNormal),
                &ledTaskHandle);					
    
    xTaskCreate(Led2BlinkTask,
                "LED2Task",
                configMINIMAL_STACK_SIZE,
                NULL,
                makeFreeRtosPriority(osPriorityNormal),
                &ledTask2Handle);    
    
    // Start scheduler
    vTaskStartScheduler();

    // We should never get here as control is now taken by the scheduler
    for(;;);
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif
