#include "usb_task.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_customhid.h"
#include "stm32l0xx_hal.h"

// LOCAL FUNCTION PROTOTYPES
static int8_t HID_USER_Init(void);
static int8_t HID_USER_DeInit(void);
static int8_t HID_USER_OutEvent(uint8_t event_idx, uint8_t state);

__ALIGN_BEGIN static uint8_t HID_CUSTOM_ReportDesc[USBD_CUSTOM_HID_REPORT_DESC_SIZE]  __ALIGN_END =
{
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x00,                    // USAGE (Undefined)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
	0x85, 0x01,                    //   REPORT_ID (1)
	0x75, 0x40,                    //   REPORT_SIZE (64)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x09, 0x00,                    //   USAGE (Undefined)
	0x81, 0x82,                    //   INPUT (Data,Var,Abs,Vol)
	0x85, 0x02,                    //   REPORT_ID (2)
	0x75, 0x40,                    //   REPORT_SIZE (64)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x09, 0x00,                    //   USAGE (Undefined)
	0x91, 0x82,                    //   OUTPUT (Data,Var,Abs,Vol)
	0xC0                           // END_COLLECTION
};

// GLOBAL VARIABLES
USBD_HandleTypeDef USBD_Device;
USBD_CUSTOM_HID_ItfTypeDef USBD_Callbacks = {
											  HID_CUSTOM_ReportDesc,
											  HID_USER_Init,
											  HID_USER_DeInit,
											  HID_USER_OutEvent,
											};

// GLOBAL FUNCTION IMPLEMENTATIONS
void UsbTask(const void *argument)
{
	// Initialize USB hardware
	USBD_Device.pUserData = &USBD_Callbacks;

	// Init USB Device Library
	USBD_Init(&USBD_Device, &HID_Desc, 0);

	// Register the HID class
	USBD_RegisterClass(&USBD_Device, &USBD_CUSTOM_HID);

	// Start Device Process
	USBD_Start(&USBD_Device);

	while(1)
	{
		;
	}

}

static int8_t HID_USER_Init(void)
{
	return 0;
}

static int8_t HID_USER_DeInit(void)
{
	return 0;
}

static int8_t HID_USER_OutEvent(uint8_t event_idx, uint8_t state)
{
	return 0;
}
