#include "hwctrl.h"
#include "stm32l0xx_hal.h"
#include "led_task.h"
#include "usb_task.h"
#include "cmsis_os.h"

uint32_t __initial_sp = 0x0000;

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
	
		// Initialize the ST Micro Board Support Library
    HAL_Init();

    // Configure the system clock
    SystemClock_Config();

    // Setup external ports on the MCU
    HwCtrl_Init();

    // Create an LED blink task		
		xTaskCreate(LedBlinkTask,
              "LEDTask",
              configMINIMAL_STACK_SIZE,
              NULL,
              makeFreeRtosPriority(osPriorityNormal),
              &ledTaskHandle);					

		// Start scheduler
		vTaskStartScheduler();

    // We should never get here as control is now taken by the scheduler
    for(;;);
}

// Dummy reset handler to satisfy Keil
void Reset_Handler(void) 
{
	
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
