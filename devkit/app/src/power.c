#include "power.h"


void PowerHwInit(void)
{
    // Initialize battery monitor ADC
    
    ADC_HandleTypeDef adc;
    adc.Instance = ADC1;
    adc.Init.LowPowerAutoOff = 0;
    
    HAL_ADC_Init(&adc);
}