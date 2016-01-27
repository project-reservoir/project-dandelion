#include "power.h"
#include "debug.h"
#include "cmsis_os.h"

ADC_HandleTypeDef             adc;
ADC_ChannelConfTypeDef        adcChannelConf;

void PowerHwInit(void)
{
    // Initialize battery monitor ADC
    
    /* ### - 1 - Initialize ADC peripheral #################################### */
  /*
   *  Instance                  = ADC1.
   *  OversamplingMode          = Disabled
   *  ClockPrescaler            = PCLK clock with no division.
   *  LowPowerAutoOff           = Enabled
   *  LowPowerFrequencyMode     = Enabled
   *  LowPowerAutoWait          = Enabled (New conversion starts only when the previous conversion is completed)       
   *  Resolution                = 12 bit (increased to 16 bit with oversampler)
   *  SamplingTime              = 7.5 cycles od ADC clock.
   *  ScanDirection             = Upward 
   *  DataAlign                 = Right
   *  ContinuousConvMode        = Enabled
   *  DiscontinuousConvMode     = Enabled
   *  ExternalTrigConvEdge      = None (Software start)
   *  EOCSelection              = End Of Conversion event
   *  DMAContinuousRequests     = DISABLE
   */
    
    __ADC1_CLK_ENABLE();

    adc.Instance                   = ADC1;
    adc.Init.OversamplingMode      = DISABLE;  
    adc.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV1;
    adc.Init.LowPowerAutoOff       = ENABLE;
    adc.Init.LowPowerFrequencyMode = ENABLE;
    adc.Init.LowPowerAutoWait      = ENABLE;

    adc.Init.Resolution            = ADC_RESOLUTION12b;
    adc.Init.SamplingTime          = ADC_SAMPLETIME_7CYCLES_5;
    adc.Init.ScanDirection         = ADC_SCAN_DIRECTION_UPWARD;
    adc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    adc.Init.ContinuousConvMode    = DISABLE;
    adc.Init.DiscontinuousConvMode = DISABLE;
    adc.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIG_EDGE_NONE;
    adc.Init.EOCSelection          = EOC_SINGLE_CONV;
    adc.Init.DMAContinuousRequests = DISABLE;
    
    if(HAL_ADC_Init(&adc) != HAL_OK)
    {
        WARN("Power ADC initialization failed\n");
    }
   
    if(HAL_ADCEx_Calibration_Start(&adc, ADC_SINGLE_ENDED) != HAL_OK)
    {
        WARN("Power ADC calibration failed\n");
    }
    
    __HAL_ADC_CLEAR_FLAG(&adc, ADC_FLAG_EOCAL);
    
    adcChannelConf.Channel = BAT_MON_ADC_CHAN;
    
    if (HAL_ADC_ConfigChannel(&adc, &adcChannelConf) != HAL_OK)
    {
        WARN("Power ADC channel configuration failed\n");
    }
    
    adcChannelConf.Channel = ADC_CHANNEL_TEMPSENSOR;
    
    if (HAL_ADC_ConfigChannel(&adc, &adcChannelConf) != HAL_OK)
    {
        WARN("Power ADC channel configuration failed\n");
    }

    // Configure battery ADC pin
    GPIO_InitTypeDef  GPIO_InitStruct;

	/* Enable the GPIO Clock */
    BAT_MON_ADC_CLK_ENABLE();
    
	/* Configure the GPIO_LED pin */
	GPIO_InitStruct.Pin = BAT_MON_ADC_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;;
	GPIO_InitStruct.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(BAT_MON_ADC_PORT, &GPIO_InitStruct);
}

void PowerTask(void)
{
    // Wait for other tasks to spin up
    osDelay(4000);
    
    while(1)
    {
        HAL_ADC_Start(&adc);
        
        if(HAL_ADC_PollForConversion(&adc, 100) != HAL_OK)
        {
            WARN("POWER: ADC Battery conversion failed!\n");
        }
        else
        {
            // Battery voltage = ADC voltage * 1.27
            INFO("POWER: ADC Battery Value is %d\n", HAL_ADC_GetValue(&adc)); 
        }
        
        if(HAL_ADC_PollForConversion(&adc, 100) != HAL_OK)
        {
            WARN("POWER: ADC Temperature conversion failed!\n");
        }
        else
        {
            // Battery voltage = ADC voltage * 1.27
            INFO("POWER: ADC Temperature Value is %d\n", HAL_ADC_GetValue(&adc)); 
        }
        
        __HAL_ADC_CLEAR_FLAG(&adc, ADC_FLAG_EOS | ADC_FLAG_EOSMP);
        
        osDelay(5000);
    }
}