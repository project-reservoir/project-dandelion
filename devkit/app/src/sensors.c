#include "sensors.h"
#include "hwctrl.h"
#include "cmsis_os.h"
#include "i2c.h"

// Global variables
I2C_HandleTypeDef I2CxHandle;

uint8_t i2cTxBuffer[I2C_BUFFER_SIZE];
uint8_t i2cRxBuffer[I2C_BUFFER_SIZE];

osSemaphoreId i2cSem;

SensorData sensorData;

// Local function declarations
static uint8_t GetTmp102Addr(uint8_t index);
static void InitTempSensor(uint8_t index);
static float ReadTempSensor(uint8_t index);

// Global function implementations
void SensorsTaskHwInit(void) 
{    
    GPIO_InitTypeDef  GPIO_InitStruct;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    I2Cx_SCL_GPIO_CLK_ENABLE();
    I2Cx_SDA_GPIO_CLK_ENABLE();
    /* Enable I2Cx clock */
    I2Cx_CLK_ENABLE(); 

    /*##-2- Configure peripheral GPIO ##########################################*/  
    /* I2C TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = I2Cx_SCL_AF;

    HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);

    /* I2C RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = I2Cx_SDA_PIN;
    GPIO_InitStruct.Alternate = I2Cx_SDA_AF;

    HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);

    /*##-3- Configure the NVIC for I2C #########################################*/   
    /* NVIC for I2C1 */
    //HAL_NVIC_SetPriority(I2Cx_IRQn, 0, 1);
    //HAL_NVIC_EnableIRQ(I2Cx_IRQn);
}

void SensorsTaskOSInit(void)
{   
    osSemaphoreDef(i2cTransactSem);
        
    i2cSem = osSemaphoreCreate(osSemaphore(i2cTransactSem), 1);
    
    I2CxHandle.Instance              = I2Cx;
    I2CxHandle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    I2CxHandle.Init.Timing           = I2C_TIMING_100KHZ;
    I2CxHandle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLED;
    I2CxHandle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    I2CxHandle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLED;
    I2CxHandle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLED;
    I2CxHandle.Init.OwnAddress1      = 0x00;
    I2CxHandle.Init.OwnAddress2      = 0x00;
    
    assert_param(HAL_I2C_Init(&I2CxHandle) == HAL_OK);
}

void SensorsTask(void)
{    
    InitTempSensor(0);
    
    //osDelay(2000);
    
    while(1)
    {
        ReadTempSensor(0);
        osDelay(1000);
    }
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c == &I2CxHandle)
    {
        osSemaphoreRelease(i2cSem);
    }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c == &I2CxHandle)
    {
        osSemaphoreRelease(i2cSem);
    }
}

// Local function implementations

uint8_t GetTmp102Addr(uint8_t index)
{
    uint8_t addr = TMP102_0_ADDR;
    
    switch(index)
    {
        case 0:
            addr = TMP102_0_ADDR;
            break;
        
        case 1:
            addr = TMP102_1_ADDR;
            break;
        
        case 2:
            addr = TMP102_2_ADDR;
            break;
        
        default:
            addr = TMP102_0_ADDR;
    }
    
    return addr;
}

// Read one of the temperature sensors. This function will 
// block the calling task until the entire sensor read has been completed
float ReadTempSensor(uint8_t index)
{
    uint16_t temp = 0;
    // Take the I2C semaphore so this task will block forever until the I2C transaction has completed
    // TODO: task shouldn't block forever waiting for I2C control. Maybe block for 2 seconds or something similar
    osSemaphoreWait(i2cSem, osWaitForever);
    
    __disable_irq();
    
    // Write to the i2cTxBuffer
    i2cTxBuffer[0] = TMP102_CONFIG_1_ONESHOT_VAL;
    i2cTxBuffer[1] = TMP102_CONFIG_2_VAL;
    
    // Write to the config register to begin a one-shot temperature conversion
    I2C_Start(GetTmp102Addr(index), 0); // I2C Write
    I2C_WriteByte(TMP102_CONFIG_ADDR);
    I2C_WaitForTX();
    I2C_Stop();
    
    I2C_Start(GetTmp102Addr(index), 0); // I2C Write
    I2C_WriteBytes(i2cTxBuffer, 2);
    I2C_WaitForTX();
    I2C_Stop();
    
    // Read the temperature registers from the sensor
    I2C_Start(GetTmp102Addr(index), 0); // I2C Write
    I2C_WriteByte(TMP102_TEMP_ADDR);
    I2C_WaitForTX();
    I2C_Stop();
    
    I2C_Start(GetTmp102Addr(index), 1); // I2C Read
    I2C_ReadBytes(i2cRxBuffer, 2);
    I2C_Stop();
    
    // Process returned data
    temp |= i2cRxBuffer[0] << 4;
    temp |= (i2cRxBuffer[1] & 0x0F);
    
    __enable_irq();
    
    return temp * TMP102_LSB_INC;
}

void InitTempSensor(uint8_t index)
{
    uint32_t error = 0;
    // Take the I2C semaphore so this task will block forever until the I2C transaction has completed
    // TODO: task shouldn't block forever waiting for I2C control. Maybe block for 2 seconds or something similar
    osSemaphoreWait(i2cSem, osWaitForever);
    
    // Write the config values into the TX buffer
    i2cTxBuffer[0] = TMP102_CONFIG_1_VAL;
    i2cTxBuffer[1] = TMP102_CONFIG_2_VAL;
    
    __disable_irq();
    
    I2C_Start(GetTmp102Addr(index), 0); // I2C Write
    I2C_WriteByte(TMP102_CONFIG_ADDR);
    I2C_WaitForTX();
    I2C_Stop();
    
    I2C_Start(GetTmp102Addr(index), 0); // I2C Read
    I2C_WriteBytes(i2cTxBuffer, 2);
    I2C_WaitForTX();
    I2C_Stop();
    
    __enable_irq();
    
    osSemaphoreRelease(i2cSem);
}
