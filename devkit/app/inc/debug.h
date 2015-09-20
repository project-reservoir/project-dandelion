#ifndef __DEBUG_H
#define __DEBUG_H

#include "stm32l0xx_hal.h"

void ERR(char* a);
void WARN(char* a);
void INFO(char* a);
void DEBUG(char* a);

void ToggleInfo(void);
void ToggleWarn(void);
void ToggleError(void);
void ToggleDebug(void);

#endif //__DEBUG_H
