#ifndef __HWCTRL_H
#define __HWCTRL_H

#include "stm32l0xx_hal.h"

// DEFINES

// Max value of 0x1F; this is used to compensate for temperature fluctuations
#define HSI_CALIB_DEFAULT	0x00

// GLOBAL FUNCTIONS
void SystemClock_Config(void);

#endif  // __HWCTRL_H
