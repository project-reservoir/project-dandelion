#include "hwctrl.h"
#include "stm32l0xx_hal.h"

NVIC_BLOCK ivt __attribute__((section(".patched_nvic_table")));

int main(void)
{
	uint32_t i;

    // Configure the system clock
    SystemClock_Config();

    for(i = 0; i < sizeof(NVIC_BLOCK); i += 4)
    {
    	*((uint32_t*)(SRAM_START + i)) = *((uint32_t*)(FLASH_START + i));
    }

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
