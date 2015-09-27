#include "capacitance.h"
#include "debug.h"

TSC_HandleTypeDef TscHandle;

void CapacitanceInit(uint16_t tsc_a, uint16_t tsc_b, uint16_t tsc_c)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /*##-1- Enable TSC and GPIO clocks #########################################*/
    __TSC_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();

    /*##-2- Configure Sampling Capacitor IOs (Alternate-Function Open-Drain) ###*/
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TSC;
    
    /* Channel Sampling Capacitor IO : TSC_GROUP1_IO4 = PA3, TSC_GROUP2_IO4 = PA7 */
    GPIO_InitStruct.Pin       = tsc_a | tsc_b;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Channel Sampling Capacitor IO : TSC_GROUP3_IO3 = PB1 */
    GPIO_InitStruct.Pin       = tsc_c;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*##-3- Configure Channel IOs (Alternate-Function Output PP) ######*/
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TSC;
    
    /* Channel IO = TSC_GROUP1_IO3 = PA2 */
    /* Channel IO = TSC_GROUP2_IO3 = PA6 */
    GPIO_InitStruct.Pin       = GPIO_PIN_2 | GPIO_PIN_6;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Channel IO = TSC_GROUP3_IO2 = PB0 */
    GPIO_InitStruct.Pin       = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* Configure the TSC peripheral */
    TscHandle.Instance = TSC;
    TscHandle.Init.AcquisitionMode         = TSC_ACQ_MODE_NORMAL;
    TscHandle.Init.CTPulseHighLength       = TSC_CTPH_2CYCLES;
    TscHandle.Init.CTPulseLowLength        = TSC_CTPL_2CYCLES;
    TscHandle.Init.IODefaultMode           = TSC_IODEF_OUT_PP_LOW;
    TscHandle.Init.MaxCountInterrupt       = DISABLE;
    TscHandle.Init.MaxCountValue           = TSC_MCV_8191;
    TscHandle.Init.PulseGeneratorPrescaler = TSC_PG_PRESC_DIV2;
    TscHandle.Init.SpreadSpectrum          = DISABLE;
    TscHandle.Init.SpreadSpectrumDeviation = 127;
    TscHandle.Init.SpreadSpectrumPrescaler = TSC_SS_PRESC_DIV1;
    TscHandle.Init.SynchroPinPolarity      = TSC_SYNC_POL_FALL;

    /* All channel, shield and sampling IOs must be declared below */
    TscHandle.Init.ChannelIOs              = TSC_GROUP1_IO3 | TSC_GROUP2_IO3 | TSC_GROUP3_IO2;
    TscHandle.Init.SamplingIOs             = TSC_GROUP1_IO4 | TSC_GROUP2_IO4 | TSC_GROUP3_IO3;
    TscHandle.Init.ShieldIOs               = 0;
    if (HAL_TSC_Init(&TscHandle) != HAL_OK)
    {
        /* Initialization Error */
        ERR("Capacitance hardware initialization failed");
    }
}
