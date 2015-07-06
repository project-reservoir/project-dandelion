#include "sensors.h"
#include "hwctrl.h"
#include "cmsis_os.h"

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
    HAL_NVIC_SetPriority(I2Cx_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(I2Cx_IRQn);
}

void SensorsTaskOSInit(void)
{   
    osSemaphoreDef(i2cTransactSem);
        
    i2cSem = osSemaphoreCreate(osSemaphore(i2cTransactSem), 1);
    
    I2CxHandle.Instance              = I2Cx;
    I2CxHandle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    I2CxHandle.Init.Timing           = I2C_TIMING_300KHZ_NEW;
    I2CxHandle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLED;
    I2CxHandle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    I2CxHandle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLED;
    I2CxHandle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLED;
    I2CxHandle.Init.OwnAddress1      = I2C_ADDRESS;
    I2CxHandle.Init.OwnAddress2      = 0x02;
    
    assert_param(HAL_I2C_Init(&I2CxHandle) == HAL_OK);
}

void SensorsTask(void)
{
    osDelay(2000);
    
    //InitTempSensor(0);
    
    //osDelay(2000);
    
    while(1)
    {
        //ReadTempSensor(0);
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
    // Take the I2C semaphore so this task will block forever until the I2C transaction has completed
    // TODO: task shouldn't block forever waiting for I2C control. Maybe block for 2 seconds or something similar
    osSemaphoreWait(i2cSem, osWaitForever);
    
    // Write to the i2cTxBuffer
    i2cTxBuffer[0] = TMP102_CONFIG_1_ONESHOT_VAL;
    i2cTxBuffer[1] = TMP102_CONFIG_2_VAL;
    
    // Write to the config register to begin a one-shot temperature conversion
    while(HAL_I2C_Mem_Write(&I2CxHandle, (uint16_t)GetTmp102Addr(index), (uint16_t)TMP102_CONFIG_ADDR, 1, i2cTxBuffer, 2, 1000) != HAL_OK)
    {
        // If the error as anything other than "device did not respond to address" then do something
        if(HAL_I2C_GetError(&I2CxHandle) != HAL_I2C_ERROR_AF)
        {
            
        }
    }
    
    // Take the I2C semaphore so this task will block forever until the I2C transaction has completed
    // TODO: task shouldn't block forever waiting for I2C control. Maybe block for 2 seconds or something similar
    osSemaphoreWait(i2cSem, osWaitForever);
    
    // Read the temperature registers from the 
    while(HAL_I2C_Mem_Read(&I2CxHandle, (uint16_t)GetTmp102Addr(index), (uint16_t)TMP102_TEMP_ADDR, 1, i2cRxBuffer, 2, 1000) != HAL_OK)
    {
        // If the error as anything other than "device did not respond to address" then do something
        if(HAL_I2C_GetError(&I2CxHandle) != HAL_I2C_ERROR_AF)
        {
            
        }
    }
    
    // Take the I2C semaphore again. We aren't actually going to perform another transaction, but it will block until the 
    // current transaction has finished.
    
    osSemaphoreWait(i2cSem, osWaitForever);
    
    // Process returned data
    
    osSemaphoreRelease(i2cSem);
    
    return 0.0;
}

void InitTempSensor(uint8_t index)
{
    uint32_t error;
    // Take the I2C semaphore so this task will block forever until the I2C transaction has completed
    // TODO: task shouldn't block forever waiting for I2C control. Maybe block for 2 seconds or something similar
    osSemaphoreWait(i2cSem, osWaitForever);
    
    // Write to the i2cTxBuffer
    i2cTxBuffer[0] = TMP102_CONFIG_1_VAL;
    i2cTxBuffer[1] = TMP102_CONFIG_2_VAL;
        
    // Write to the config register to begin a one-shot temperature conversion
    while(HAL_I2C_Mem_Write(&I2CxHandle, (uint16_t)GetTmp102Addr(index), (uint16_t)TMP102_CONFIG_ADDR, 1, i2cTxBuffer, 2, 2000) != HAL_OK)
    {
        error = HAL_I2C_GetError(&I2CxHandle);
        // If the error as anything other than "device did not respond to address" then do something
        if(HAL_I2C_GetError(&I2CxHandle) != HAL_I2C_ERROR_AF)
        {
            
        }
    }
}
