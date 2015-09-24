#ifndef _FW_UPDATE_H
#define _FW_UPDATE_H

#include "stm32l0xx_hal.h"

uint32_t FwUpdateGetCurrentRegion(void);

uint32_t FwUpdateGetBaseAddress(void);

void FwUpdateHwInit(void);

void FwUpdateOsInit(void);

void FwUpdateTask(void);

void FwUpdateStart(void);

void FwUpdateEnd(void);

uint8_t FwUpdateWriteWord(uint32_t word);

#endif // _FW_UPDATE_H
