#ifndef _POWER_H
#define _POWER_H

#include "stm32l0xx_hal.h"

#define BAT_MON_ADC_PIN             GPIO_PIN_2
#define BAT_MON_ADC_PORT            GPIOA

#define BAT_MON_ADC_CHAN            ADC_CHANNEL_2

#define BAT_MON_ADC_CLK_ENABLE()    __GPIOA_CLK_ENABLE()
#define BAT_MON_ADC_CLK_DISABLE()   __GPIOA_CLK_DISABLE()

void PowerHwInit(void);
void PowerTask(void);
int16_t GetChipTemperature(void);
uint16_t GetBatteryVoltage(void);

#endif // _POWER_H