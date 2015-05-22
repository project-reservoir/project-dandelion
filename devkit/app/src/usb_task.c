#include "usb_task.h"
#include "cmsis_os.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_hid.h"
#include "stm32l0xx_hal.h"

// LOCAL FUNCTION PROTOTYPES
static void UsbClockEnable(void);
static void UsbClockDisable(void);

// GLOBAL VARIABLES
USBD_HandleTypeDef USBD_Device;

// GLOBAL FUNCTION IMPLEMENTATIONS
void UsbTask(const void *argument)
{
	// Initialize USB hardware

	// Initialize USB clock
	UsbClockEnable();

	// Init USB Device Library
	USBD_Init(&USBD_Device, &HID_Desc, 0);

	// Register the HID class
	USBD_RegisterClass(&USBD_Device, &USBD_HID);

	// Start Device Process
	USBD_Start(&USBD_Device);

	while(1)
	{
		osDelay(100);
	}

}

// LOCAL FUNCTION IMPLEMENTATIONS
void UsbClockEnable(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
  static RCC_CRSInitTypeDef RCC_CRSInitStruct;

  /* Enable HSI48 Oscillator to be used as USB clock source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  /* Select HSI48 as USB clock source */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  /*Configure the clock recovery system (CRS)**********************************/

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
}


/**
  * @brief  USB Clock Configuration: Disable Resources
  * @param  None
  * @retval None
  */
void UsbClockDisable(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable HSI48 Oscillator to be used as USB clock source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_OFF;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /*Enable CRS Clock*/
  __CRS_CLK_DISABLE();
}
