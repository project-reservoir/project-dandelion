#include "capacitance.h"
#include "debug.h"
#include "sensors.h"

TSC_HandleTypeDef TscHandle;

void CapacitanceInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    CAP_IN_1_CLK_ENABLE();
    CAP_IN_2_CLK_ENABLE();
    CAP_IN_3_CLK_ENABLE();

    CAP_SENSE_1_CLK_ENABLE();
    CAP_SENSE_2_CLK_ENABLE();
    CAP_SENSE_3_CLK_ENABLE();
    
    /*##-2- Configure Sampling Capacitor IOs (Alternate-Function Open-Drain) ###*/
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TSC;

    GPIO_InitStruct.Pin       = CAP_SENSE_1_PIN;
    HAL_GPIO_Init(CAP_SENSE_1_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin       = CAP_SENSE_2_PIN;
    HAL_GPIO_Init(CAP_SENSE_2_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin       = CAP_SENSE_3_PIN;
    HAL_GPIO_Init(CAP_SENSE_3_PORT, &GPIO_InitStruct);

    /*##-3- Configure Channel IOs (Alternate-Function Output PP) ######*/
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TSC;
    
    GPIO_InitStruct.Pin       = CAP_IN_1_PIN;
    HAL_GPIO_Init(CAP_IN_1_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin       = CAP_IN_2_PIN;
    HAL_GPIO_Init(CAP_IN_2_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin       = CAP_IN_3_PIN;
    HAL_GPIO_Init(CAP_IN_3_PORT, &GPIO_InitStruct);
    
    __TSC_CLK_ENABLE();
    
    TscHandle.Instance = TSC;
    TscHandle.Init.AcquisitionMode         = TSC_ACQ_MODE_NORMAL;
    TscHandle.Init.CTPulseHighLength       = TSC_CTPH_2CYCLES;
    TscHandle.Init.CTPulseLowLength        = TSC_CTPL_2CYCLES;
    TscHandle.Init.IODefaultMode           = TSC_IODEF_OUT_PP_LOW;
    TscHandle.Init.MaxCountInterrupt       = DISABLE;
    TscHandle.Init.MaxCountValue           = TSC_MCV_16383;
    TscHandle.Init.PulseGeneratorPrescaler = TSC_PG_PRESC_DIV2;
    TscHandle.Init.SpreadSpectrum          = DISABLE;
    TscHandle.Init.SpreadSpectrumDeviation = 127;
    TscHandle.Init.SpreadSpectrumPrescaler = TSC_SS_PRESC_DIV1;
    TscHandle.Init.SynchroPinPolarity      = TSC_SYNC_POL_FALL;

    TscHandle.Init.SamplingIOs             = CAP_SENSE_1_TSC | CAP_SENSE_2_TSC | CAP_SENSE_3_TSC;
    TscHandle.Init.ChannelIOs              = CAP_IN_1_TSC | CAP_IN_2_TSC | CAP_IN_3_TSC;
    TscHandle.Init.ShieldIOs               = 0;
    
    if (HAL_TSC_Init(&TscHandle) != HAL_OK)
    {
        /* Initialization Error */
        ERR("Capacitance hardware initialization failed\n");
    }
}

void CapacitanceRead(uint32_t* count_1, uint32_t* count_2, uint32_t* count_3)
{
    TscHandle.Instance->CR |= TSC_CR_START;
    
    // Wait for the acquisition to finish
    while((TscHandle.Instance->ISR & TSC_ISR_EOAF) != TSC_ISR_EOAF);
    
    *count_1 = TscHandle.Instance->IOGXCR[0];
    *count_2 = TscHandle.Instance->IOGXCR[1];
    *count_3 = TscHandle.Instance->IOGXCR[2];
    
    // Clear the End Of Acquisition interupt flag
    TscHandle.Instance->ICR |= TSC_ICR_EOAIC;
}
