#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include "xbee/platform.h"
#include "xbee/serial.h"
#include "xbee/cbuf.h"

// Could change XBEE_SER_CHECK to an assert, or even ignore it if not in debug.
#if defined XBEE_SERIAL_DEBUG
	#define XBEE_SER_CHECK(ptr)	\
		do { if (xbee_ser_invalid(ptr)) return -EINVAL; } while (0)
#else
	#define XBEE_SER_CHECK(ptr)
#endif

bool_t xbee_ser_invalid( xbee_serial_t *serial)
{
	return 1;
}


const char *xbee_ser_portname( xbee_serial_t *serial)
{
	return "(invalid)";
}


int xbee_ser_write( xbee_serial_t *serial, const void FAR *buffer,
	int length)
{
	return 0;
}

int xbee_ser_read( xbee_serial_t *serial, void FAR *buffer, int bufsize)
{
	return 0;
}


int xbee_ser_putchar( xbee_serial_t *serial, uint8_t ch)
{
	return 0;
}


int xbee_ser_getchar( xbee_serial_t *serial)
{
	return 0;
}


// Since we're using blocking transmit, there isn't a transmit buffer.
// Therefore, have xbee_ser_tx_free() and xbee_ser_tx_used() imply that
// we have an empty buffer that can hold an unlimited amount of data.

int xbee_ser_tx_free( xbee_serial_t *serial)
{
	return INT_MAX;
}

int xbee_ser_tx_used( xbee_serial_t *serial)
{
	return 0;
}

int xbee_ser_tx_flush( xbee_serial_t *serial)
{
	return 0;
}

int xbee_ser_rx_free( xbee_serial_t *serial)
{
	
	return 0;
}


int xbee_ser_rx_used( xbee_serial_t *serial)
{
	return 0;
}


int xbee_ser_rx_flush( xbee_serial_t *serial)
{
	return 0;
}


int xbee_ser_baudrate( xbee_serial_t *serial, uint32_t baudrate)
{
	return 0;
}

int xbee_ser_open( xbee_serial_t *serial, uint32_t baudrate)
{
	return 0;
}


int xbee_ser_close( xbee_serial_t *serial)
{
	return 0;
}


int xbee_ser_break( xbee_serial_t *serial, bool_t enabled)
{
	return 0;
}


int xbee_ser_flowcontrol( xbee_serial_t *serial, bool_t enabled)
{
	return 0;
}

int xbee_ser_set_rts( xbee_serial_t *serial, bool_t asserted)
{
	return 0;
}


int xbee_ser_get_cts( xbee_serial_t *serial)
{
	return 0;
}

//@}
