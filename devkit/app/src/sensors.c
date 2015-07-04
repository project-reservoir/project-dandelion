#include "sensors.h"
#include "hwctrl.h"
#include "cmsis_os.h"

// Global variables
I2C_HandleTypeDef I2CxHandle;

uint8_t i2cTxBuffer[I2C_BUFFER_SIZE];
uint8_t i2cRxBuffer[I2C_BUFFER_SIZE];

osSemaphoreId i2cSem;

//SensorData sensorData;

// Local function declarations
static uint8_t GetTmp102Addr(uint8_t index);

// Global function implementations
void SensorsTaskHwInit(void) 
{
    I2CxHandle.Instance              = I2Cx;
    I2CxHandle.Init.AddressingMode   = I2C_ADDRESSINGMODE_10BIT;
    I2CxHandle.Init.Timing           = I2C_TIMING_400KHZ;
    I2CxHandle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLED;
    I2CxHandle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    I2CxHandle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLED;
    I2CxHandle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLED;
    I2CxHandle.Init.OwnAddress1      = I2C_ADDRESS;
    I2CxHandle.Init.OwnAddress2      = 0xFE;
    
    if(HAL_I2C_Init(&I2CxHandle) != HAL_OK)
    {
        // TODO: do something if we can't initialize the I2C module
    }	
}

void SensorsTaskOSInit(void)
{    
    osSemaphoreDef(i2cTransactSem);
    i2cSem = osSemaphoreCreate(osSemaphore(i2cTransactSem), 1);
}

void SensorsTask(void)
{
    
}

// Read one of the temperature sensors. This function will 
// block the calling task until the entire sensor read has been completed
void ReadTempSensor(uint8_t index)
{
    // Take the I2C semaphore so this task will block forever until the I2C transaction has completed
    // TODO: task shouldn't block forever waiting for I2C control. Maybe block for 2 seconds or something similar
    osSemaphoreWait(i2cSem, 0);
    
    // Write to the i2cTxBuffer
    i2cTxBuffer[0] = TMP102_CONFIG_1_ONESHOT_VAL;
    i2cTxBuffer[1] = TMP102_CONFIG_2_VAL;
    
    // Write to the config register to begin a one-shot temperature conversion
    while(HAL_I2C_Mem_Write_IT(&I2CxHandle, GetTmp102Addr(index), TMP102_CONFIG_ADDR, 2, i2cTxBuffer, 2))
    {
        if (HAL_I2C_GetError(&I2CxHandle) != HAL_I2C_ERROR_AF)
        {
            // TODO: do something if we can't transmit the required data
        }
    }
    
    // Take the I2C semaphore so this task will block forever until the I2C transaction has completed
    // TODO: task shouldn't block forever waiting for I2C control. Maybe block for 2 seconds or something similar
    osSemaphoreWait(i2cSem, 0);
    
    // Read the temperature registers from the 
    while(HAL_I2C_Mem_Read_IT(&I2CxHandle, GetTmp102Addr(index), TMP102_TEMP_ADDR, 2, i2cTxBuffer, 2))
    {
        if (HAL_I2C_GetError(&I2CxHandle) != HAL_I2C_ERROR_AF)
        {
            // TODO: do something if we can't get the required data
        }
    }
    
    // Take the I2C semaphore again. We aren't actually going to perform another transaction, but it will block until the 
    // current transaction has finished.
    
    osSemaphoreWait(i2cSem, 0);
    
    // Process returned data
    
    osSemaphoreRelease(i2cSem);
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
            break;
        
        case 1:
            addr = TMP102_0_ADDR;
            break;
        
        case 2:
            addr = TMP102_2_ADDR;
            break;
        
        default:
            addr = TMP102_0_ADDR;
    }
    
    return addr;
}
