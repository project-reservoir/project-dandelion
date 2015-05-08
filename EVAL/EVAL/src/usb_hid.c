/*
 * usb_hid.c
 *
 * Created: 5/8/2015 1:44:58 AM
 *  Author: Stuart Alldritt
 */ 
#include <asf.h>

// This function is called when the USB device is enumerated successfully by the host
bool usb_generic_enable(void)
{
    return true;
}

// This function is called when the device is unplugged or is reset by the host
void usb_generic_disable(void)
{

}

// This function is used to receive OUT reports
void usb_generic_report_out(uint8_t *report)
{

}

// This function is used to receive SET_FEATURE reports
void usb_generic_set_feature(uint8_t *report_feature)
{

}