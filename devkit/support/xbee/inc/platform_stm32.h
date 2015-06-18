#ifndef __XBEE_PLATFORM_STM32
#define __XBEE_PLATFORM_STM32

	#define LITTLE_ENDIAN	1234
	#define BIG_ENDIAN		4321
	#define PDP_ENDIAN		3412

	#define BYTE_ORDER		LITTLE_ENDIAN

	#define _f_memcpy		memcpy
	#define _f_memset		memset

    // Elements needed to keep track of serial port settings.  Must have a
    // baudrate memember, other fields are platform-specific.
	typedef struct xbee_serial_t {
		uint32_t			baudrate;
		uint8_t				port;
	} xbee_serial_t;

	// We'll use 1/1/2000 as the epoch, to match ZigBee.
	#define ZCL_TIME_EPOCH_DELTA	0

	// our millisecond timer has a 4ms resolution
	#define XBEE_MS_TIMER_RESOLUTION 10
#endif		// __XBEE_PLATFORM_STM32

