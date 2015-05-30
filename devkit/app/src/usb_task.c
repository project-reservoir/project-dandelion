#include "usb_task.h"
#include "cmsis_os.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "stm32l0xx_hal.h"

// DEFINES
#define USB_QUEUE_SIZE 8

// LOCAL FUNCTION PROTOTYPES
static int8_t HID_USER_Init(void);
static int8_t HID_USER_DeInit(void);
static int8_t HID_USER_OutEvent(uint8_t* buffer);

// LOCAL VARIABLES
static osMessageQId usbTaskQueue;

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
USBD_CUSTOM_HID_ItfTypeDef USBD_Callbacks =
{
  HID_CUSTOM_ReportDesc,
  HID_USER_Init,
  HID_USER_DeInit,
  HID_USER_OutEvent,
};

// GLOBAL FUNCTION IMPLEMENTATIONS
void UsbTaskInit(void)
{
	osMessageQDef(usbTaskQueue, USB_QUEUE_SIZE, USB_QUEUE_MSG*);
	usbTaskQueue = osMessageCreate(osMessageQ(usbTaskQueue), NULL);
}

void UsbTask(const void *argument)
{
	USB_QUEUE_MSG* msg;

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
		msg = (USB_QUEUE_MSG*)(osMessageGet(usbTaskQueue, osWaitForever).value.v);
	}

}

// LOCAL FUNCTION IMPLEMENTATIONS
static int8_t HID_USER_Init(void)
{
	return 0;
}

static int8_t HID_USER_DeInit(void)
{
	return 0;
}

static int8_t HID_USER_OutEvent(uint8_t* buffer)
{
	USB_QUEUE_MSG* msg = pvPortMalloc(sizeof(USB_QUEUE_MSG));

	assert_param(msg != NULL);

	// Copy the contents of the USB buffer into a new buffer that we're going to pass into the OS queue
	memcpy(buffer, msg->message, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);

	// Throw an assertion if the message queue is full. Use this for tuning the size of the message queue during development
	assert_param(osMessagePut(usbTaskQueue, (uint32_t)msg, 100) == osOK);

	return 0;
}
