#include "power.h"
#include "debug.h"
#include "cmsis_os.h"

ADC_HandleTypeDef             adc;
ADC_ChannelConfTypeDef        adcChannelConf;

int32_t                       chip_temperature;
uint32_t                      battery_voltage;

#define TEMP130_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FF8007E))
#define TEMP30_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FF8007A))
#define VREFINT_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FF80078))
#define VDD_CALIB ((uint16_t) (300))
#define VDD_APPLI ((uint16_t) (330))

int32_t ComputeTemperature(uint32_t measure)
{
    int32_t temperature;
    temperature = ((measure * VDD_APPLI / VDD_CALIB) - (int32_t) *TEMP30_CAL_ADDR );
    temperature = temperature * (int32_t)(130 - 30);
    temperature = temperature / (int32_t)(*TEMP130_CAL_ADDR - *TEMP30_CAL_ADDR);
    temperature = temperature + 30;
    return(temperature);
}

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
    adc.Init.LowPowerAutoOff       = DISABLE;
    adc.Init.LowPowerFrequencyMode = DISABLE;
    adc.Init.LowPowerAutoWait      = ENABLE;

    adc.Init.Resolution            = ADC_RESOLUTION12b;
    adc.Init.SamplingTime          = ADC_SAMPLETIME_239CYCLES_5;
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
    
    adcChannelConf.Channel = ADC_CHANNEL_0;
    
    if (HAL_ADC_ConfigChannel(&adc, &adcChannelConf) != HAL_OK)
    {
        WARN("Power ADC channel configuration failed\n");
    }
    
    adcChannelConf.Channel = BAT_MON_ADC_CHAN;
    
    if (HAL_ADC_ConfigChannel(&adc, &adcChannelConf) != HAL_OK)
    {
        WARN("Power ADC channel configuration failed\n");
    }
    
    adcChannelConf.Channel = ADC_CHANNEL_VREFINT;
    
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
    
    adc.Instance->CR |= ADC_CR_ADEN;
    
    while((adc.Instance->ISR & ADC_ISR_ADRDY) != ADC_ISR_ADRDY);
    
    while(1)
    {        
        // Start an ADC conversion
        adc.Instance->CR |= ADC_CR_ADSTART;
        
        // Wait for the zeroth conversion to finish
        // THIS CONVERSION IS A DUMMY CONVERSION AND THE RESULTS SHOULD BE DISCARDED
        while((adc.Instance->ISR & ADC_ISR_EOC) != ADC_ISR_EOC && (adc.Instance->ISR & ADC_ISR_EOS) != ADC_ISR_EOS);
        
        adc.Instance->ISR |= ADC_ISR_EOC;
        
        // Wait for first conversion to finish
        while((adc.Instance->ISR & ADC_ISR_EOC) != ADC_ISR_EOC && (adc.Instance->ISR & ADC_ISR_EOS) != ADC_ISR_EOS);
        
        // Read data from ADC DR, reset ISR flags
        uint16_t battery_val = adc.Instance->DR;
        
        // Wait for second conversion to finish
        while((adc.Instance->ISR & ADC_ISR_EOC) != ADC_ISR_EOC && (adc.Instance->ISR & ADC_ISR_EOS) != ADC_ISR_EOS);
        
        // Read data from ADC DRR, reset ISR flags
        uint16_t vref_val = adc.Instance->DR;
        
        // Wait for third conversion to finish
        while((adc.Instance->ISR & ADC_ISR_EOC) != ADC_ISR_EOC && (adc.Instance->ISR & ADC_ISR_EOS) != ADC_ISR_EOS);
        
        // Read data from ADC DRR, reset ISR flags
        uint16_t temp_val = adc.Instance->DR;
        
        adc.Instance->ISR |= ADC_ISR_EOS;
                
        chip_temperature = ComputeTemperature(temp_val);
        battery_voltage = (3000.0f * (float)(*VREFINT_CAL_ADDR) * (float)battery_val) / ((float)vref_val * 4095.0f);
        //battery_voltage = (uint32_t)((3300.0f / 4095.0f) * (float)battery_val));
        
        // Battery voltage = ADC voltage * 1.27
        DEBUG("POWER: Battery Voltage analog read is %d\n", battery_val);
        DEBUG("POWER: Chip Temperature Value is %d\n", temp_val);
        DEBUG("POWER: Vref Value is %d\n", vref_val);
        DEBUG("POWER: ADC CALFACT %d\n", adc.Instance->CALFACT);
        DEBUG("POWER: VREFINT Calibration %d\n", (*VREFINT_CAL_ADDR));
        
        osDelay(5000);
    }
}

int16_t GetChipTemperature(void)
{
    return chip_temperature;
}

uint16_t GetBatteryVoltage(void)
{
    return battery_voltage;
}