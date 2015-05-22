#include "usb_task.h"
#include "cmsis_os.h"
#include "stm32l0xx_hal.h"

USBD_HandleTypeDef USBD_Device;

void UsbTask(const void *argument)
{
	// Initialize USB hardware

	// Initialize USB clock

	// Init USB Device Library
	USBD_Init(&USBD_Device, &HID_Desc, 0);

	// Register the HID class
	USBD_RegisterClass(&USBD_Device, &USBD_HID);

	// Start Device Process
	USBD_Start(&USBD_Device);

}
