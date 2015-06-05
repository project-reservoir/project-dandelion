#include "hwctrl.h"

void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
	static RCC_CRSInitTypeDef RCC_CRSInitStruct;

	/* Enable Power Controller clock */
	__PWR_CLK_ENABLE();

	/* The voltage scaling allows optimizing the power consumption when the device is
	clocked below the maximum system frequency, to update the voltage scaling value
	regarding system frequency refer to product datasheet. */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	/* Enable HSI Oscillator to be used as System clock source */
	/* Enable HSI48 Oscillator to be used as USB clock source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI48;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = 0x00; // Max value of 0x1F, this is used to compensate for temperature fluctuations
	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	/* Select HSI48 as USB clock source */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
	PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

	/*Configure the clock recovery system (CRS)*/

	/*Enable CRS Clock*/
	__CRS_CLK_ENABLE();

	/* Default Synchro Signal division factor (not divided) */
	RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
	/* Set the SYNCSRC[1:0] bits according to CRS_Source value */
	RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;
	/* HSI48 is synchronized with USB SOF at 1KHz rate */
	RCC_CRSInitStruct.ReloadValue =  __HAL_RCC_CRS_CALCULATE_RELOADVALUE(48000000, 1000);
	RCC_CRSInitStruct.ErrorLimitValue = RCC_CRS_ERRORLIMIT_DEFAULT;
	/* Set the TRIM[5:0] to the default value*/
	RCC_CRSInitStruct.HSI48CalibrationValue = 0x20;
	/* Start automatic synchronization */
	HAL_RCCEx_CRSConfig (&RCC_CRSInitStruct);

	/* Select HSI as system clock source and configure the HCLK, PCLK1 and PCLK2
	 clock dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

	/* Select HSI as system clock source after Wake Up from Stop mode */
	__HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_StopWakeUpClock_HSI);
}

void HwCtrl_Init(void)
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

	__GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = LED4_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

	HAL_GPIO_Init(LED4_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(LED4_GPIO_PORT, LED4_PIN, GPIO_PIN_RESET);
}
