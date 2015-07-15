#ifndef __HWCTRL_H
#define __HWCTRL_H

#include "stm32l0xx_hal.h"

// GLOBAL FUNCTIONS
void SystemClock_Config(void);
void HwCtrl_Init(void);
void HwCtrl_Led_Control(void);

#endif  // __HWCTRL_H
