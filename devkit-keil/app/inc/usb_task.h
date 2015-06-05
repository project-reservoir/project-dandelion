#ifndef __USB_TASK_H
#define __USB_TASK_H

#include "usbd_customhid.h"

// STRUCTURES

typedef struct USB_QUEUE_MSG_T
{
	uint8_t message[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE];
} USB_QUEUE_MSG;

// INTERFACE
void UsbTask(const void *argument);
void UsbTaskInit(void);

#endif // __USB_TASK_H
