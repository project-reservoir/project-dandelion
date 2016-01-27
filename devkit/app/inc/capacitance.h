#ifndef _CAPACITANCE_H
#define _CAPACITANCE_H

#include "stm32l0xx_hal.h"


void CapacitanceInit(void);
void CapacitanceRead(uint32_t* count_1, uint32_t* count_2, uint32_t* count_3);

#endif // _CAPACITANCE_H