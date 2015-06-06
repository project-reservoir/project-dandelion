#include "hwctrl.h"

void SystemClock_Config(void)
{
	/* Enable Power Controller clock */
	__PWR_CLK_ENABLE();

	/* The voltage scaling allows optimizing the power consumption when the device is
	clocked below the maximum system frequency, to update the voltage scaling value
	regarding system frequency refer to product datasheet. */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

/* Enable HSI Oscillator to be used as System clock source */

	/* Enable the Internal High Speed oscillator (HSI or HSIdiv4 */
	__HAL_RCC_HSI_CONFIG(RCC_HSI_ON);

	/* Wait till HSI is ready */
	while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == RESET)
	{
		;
	}

	/* Adjusts the Internal High Speed oscillator (HSI) calibration value.*/
	__HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(HSI_CALIB_DEFAULT);

/* Select HSI as system clock source and configure the HCLK, PCLK1 and PCLK2 clock dividers */

	__HAL_FLASH_SET_LATENCY(FLASH_LATENCY_1);

	/* Check that the new number of wait states is taken into account to access the Flash
	memory by reading the FLASH_ACR register */
	if((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_LATENCY_1)
	{
		//TODO: Do something if the flash memory hasn't updated correctly
		;
	}

	MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_SYSCLK_DIV1);

	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_SYSCLKSOURCE_HSI);

    while(__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_CFGR_SWS_HSI)
	{
    	;
	}

    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_HCLK_DIV1);
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, RCC_HCLK_DIV1);

	/* Select HSI as system clock source after Wake Up from Stop mode */
	__HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_StopWakeUpClock_HSI);
}

