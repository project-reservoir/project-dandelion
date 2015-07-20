#include "sensors.h"
#include "hwctrl.h"
#include "cmsis_os.h"
#include "i2c.h"
#include "radio.h"

// Global variables
I2C_HandleTypeDef I2CxHandle;

uint8_t i2cTxBuffer[I2C_BUFFER_SIZE];
uint8_t i2cRxBuffer[I2C_BUFFER_SIZE];

osSemaphoreId i2cSem;

SensorData sensorData;

// Local function declarations
static uint8_t GetTmp102Addr(uint8_t index);
static I2C_Status InitTempSensor(uint8_t index);
static I2C_Status ReadTempSensor(uint8_t index, float* tempOut);
static I2C_Status ReadHumiditySensor(float* humidOut);
static I2C_Status InitHumiditySensor(void);
static I2C_Status ReadAirTempSensor(float* tempOut);
static void       SendSensorData(void);

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
    // Array storing the status of each physical sensor.
    // If a sensor fails to initialize, or fails 3 sensor reads in a row, 
    // it will be disabled here until the next system reboot
    uint8_t enabledSensors[4] = {3, 3, 3, 3};
    uint8_t i;
    
    for(i = 0; i < 3; i++)
    {
        // If the temperature sensor initialized, set its enabled value to 3 (so it has 3 chances to respond to a read request)
        // Else, disable the sensor
        if(InitTempSensor(i) != I2C_OK)
        {
            I2C_Reset();
            enabledSensors[i] = 0;
        }
    }
    
    if(InitHumiditySensor() != I2C_OK)
    {
        I2C_Reset();
        enabledSensors[3] = 0;
    }
    
    // Let other tasks in the system warmup before entering the sensor polling loop
    osDelay(2000);
    
    // TODO: re-initialize the sensors once a day to check for failures and for sensors that have come back online
    // TODO: report to the base station when a sensor fails
    
    while(1)
    {
        if(enabledSensors[0] > 0)
        {
            // If the sensor read failed, indicate that the sensor has one less chance to respond correctly before being disabled
            if(ReadTempSensor(0, &sensorData.temp0) != I2C_OK)
            {
                I2C_Reset();
                enabledSensors[0]--;
            }
            // The sensor is still alive! Restore it to a full 3 chances to respond
            else
            {
                enabledSensors[0] = 3;
            }
        }
        
        if(enabledSensors[1] > 0)
        {
            if(ReadTempSensor(1, &sensorData.temp1) != I2C_OK)
            {
                I2C_Reset();
                enabledSensors[1]--;
            }
            else
            {
                enabledSensors[1] = 3;
            }
        }
        
        if(enabledSensors[2] > 0)
        {
            if(ReadTempSensor(2, &sensorData.temp2) != I2C_OK)
            {
                I2C_Reset();
                enabledSensors[2]--;
            }
            else
            {
                enabledSensors[2] = 3;
            }
        }
        
        if(enabledSensors[3] > 0)
        {
           if(ReadHumiditySensor(&sensorData.humid) != I2C_OK)
           {
                I2C_Reset();
                enabledSensors[3]--;
           }
        }
        if(enabledSensors[3] > 0)
        {
           if(ReadAirTempSensor(&sensorData.tempAir) != I2C_OK)
           {
                I2C_Reset();
                enabledSensors[3]--;
           }
        }
        
        //ReadChipTemp(&sensorData.tempChip);
        
        // Send sensor Data to the base station
        SendSensorData();
        
        osDelay(1000);
    }
}

void SendSensorData(void)
{
    uint16_t tmp = 0;
    uint8_t* radioMessage;
    
    // TODO: we should be wrapping this data in MAC layer data in the radio task, so
    // we shouldn't be allocating the full packet size here
    radioMessage = pvPortMalloc(RADIO_MAX_PACKET_LENGTH);
    
    // moist 0
    radioMessage[0] = 0x00; 
    radioMessage[1] = 0x00; 
    
    // moist 1
    radioMessage[2] = 0x00;
    radioMessage[3] = 0x00;
    
    // moist 2
    radioMessage[4] = 0x00;
    radioMessage[5] = 0x00;
    
    // humid
    tmp = ((sensorData.humid + 6.0f) / 125.0f) * ((float)(1 << 16));
    radioMessage[6] = (tmp >> 8) & 0xFF;     // MSB
    radioMessage[7] = tmp & 0xFF;            // LSB
    
    // temp 0
    tmp = sensorData.temp0 / TMP102_LSB_INC;       
    radioMessage[8] = (tmp >> 8) & 0xFF;     // MSB
    radioMessage[9] = tmp & 0xFF;            // LSB
    
    // temp 1
    tmp = sensorData.temp1 / TMP102_LSB_INC; 
    radioMessage[10] = (tmp >> 8) & 0xFF;     // MSB
    radioMessage[11] = tmp & 0xFF;            // LSB
    
    // temp 2
    tmp = sensorData.temp2 / TMP102_LSB_INC;
    radioMessage[12] = (tmp >> 8) & 0xFF;     // MSB
    radioMessage[13] = tmp & 0xFF;            // LSB
   
    // air temp
    tmp = (((sensorData.tempAir + 46.85f) / 175.72f) * ((float)(1 << 16)));
    radioMessage[14] = (tmp >> 8) & 0xFF;     // MSB
    radioMessage[15] = tmp & 0xFF;            // LSB
    
    // battery level
    radioMessage[16] = 0x00;
    radioMessage[17] = 0x00;
    
    // solar level
    radioMessage[18] = 0x00;
    radioMessage[19] = 0x00;
    
    SendToBroadcast(radioMessage, RADIO_MAX_PACKET_LENGTH);
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
I2C_Status ReadTempSensor(uint8_t index, float* tempOut)
{
    I2C_Status retVal = I2C_OK;
    uint16_t temp = 0;
    
    taskENTER_CRITICAL();
    
    do
    {
        // Write to the i2cTxBuffer
        i2cTxBuffer[0] = TMP102_CONFIG_1_ONESHOT_VAL;
        i2cTxBuffer[1] = TMP102_CONFIG_2_VAL;

        // Write to the config register to begin a one-shot temperature conversion
        // I2C Write
        I2C_Start(GetTmp102Addr(index), 0); 
        if((retVal = I2C_WriteByte(TMP102_CONFIG_ADDR)) != I2C_OK)
        {
            break;
        }
        
        if((retVal = I2C_WaitForTX()) != I2C_OK)
        {
            break;
        }
            
        I2C_Stop();
        
        // I2C Write
        I2C_Start(GetTmp102Addr(index), 0); 
        
        if((retVal = I2C_WriteBytes(i2cTxBuffer, 2)) != I2C_OK)
        {
            break;
        }
        
        if((retVal = I2C_WaitForTX()) != I2C_OK)
        {
            break;
        }
        
        I2C_Stop();

        // Read the temperature registers from the sensor
        // I2C Write
        I2C_Start(GetTmp102Addr(index), 0); 
        
        if((retVal = I2C_WriteByte(TMP102_TEMP_ADDR)) != I2C_OK)
        {
            break;
        }
        
        if((retVal = I2C_WaitForTX()) != I2C_OK)
        {
            break;
        }
        
        I2C_Stop();
        
        // I2C Read
        I2C_Start(GetTmp102Addr(index), 1);
        
        if((retVal = I2C_ReadBytes(i2cRxBuffer, 2)) != I2C_OK)
        {
            break;
        }
        
        I2C_Stop();

        // Process returned data
        temp |= i2cRxBuffer[0] << 4;
        temp |= (i2cRxBuffer[1] & 0x0F);
        
        *tempOut = temp * TMP102_LSB_INC;
    } while(0);
    
    taskEXIT_CRITICAL();
    
    return retVal;
}

I2C_Status InitTempSensor(uint8_t index)
{        
    I2C_Status retVal = I2C_OK;
    
    // Write the config values into the TX buffer
    i2cTxBuffer[0] = TMP102_CONFIG_1_VAL;
    i2cTxBuffer[1] = TMP102_CONFIG_2_VAL;
    
    taskENTER_CRITICAL();
    
    // TODO: log something if the sensor fails to initialize
    do
    {
        // I2C Write
        I2C_Start(GetTmp102Addr(index), 0);
        retVal = I2C_WriteByte(TMP102_CONFIG_ADDR);
        if(retVal != I2C_OK)
        {
            break;
        }
        retVal = I2C_WaitForTX();
        if(retVal != I2C_OK)
        {
            break;
        }
        I2C_Stop();
        
        // I2C Read
        I2C_Start(GetTmp102Addr(index), 0); 
        retVal = I2C_WriteBytes(i2cTxBuffer, 2);
        if(retVal != I2C_OK)
        {
            break;
        }
        retVal = I2C_WaitForTX();
        if(retVal != I2C_OK)
        {
            break;
        }
        I2C_Stop();
    }while(0);
    
    taskEXIT_CRITICAL();
    
    return retVal;
}

I2C_Status InitHumiditySensor(void)
{
    // Humidity sensor does not need initialization
    return I2C_OK;
}

// Send a command to the humidity sensor to start a humidity value conversion
// This process takes about 15 miliseconds, and the sensor will NACK all I2C
// transactions that are initiated until the conversion is complete
// This function will block the OS from operating until it has completed, meaning 
// the SysTick timer will be out of sync by 15ms once this operation has completed.
I2C_Status ReadHumiditySensor(float* humidOut)
{
    uint16_t humidity = 0;
    I2C_Status retVal = I2C_OK;
    
    // This is a critical section: if the device is interrupted during an I2C transaction, it will probably fail.
    // Disable all interrupts during this transaction to ensure that it completes sucessfully.
    // NOTE: This means that the system clock will be wrong by ~15 ms (15 ticks) after this section of code is executed
    taskENTER_CRITICAL();
    
    // Use a do {} while(0); loop to allow the use of the "break" statement.
    // In C++ / Java this would be accomplished with a "try {} catch() {} finally {}" section, but C does not support this construct.
    do
    {
        // I2C Write
        I2C_Start(HTU21D_ADDR, 0); 
        
        retVal = I2C_WriteByte(HTU21D_HUMID_MEAS_HOLD);
        if(retVal != I2C_OK)
        {
            break;
        }
        
        retVal = I2C_WaitForTX();
        if(retVal != I2C_OK)
        {
            break;
        }
        
        // I2C Read
        I2C_Start(HTU21D_ADDR, 1);
        
        retVal = I2C_ReadBytes(i2cRxBuffer, 3);
        if(retVal != I2C_OK)
        {
            break;
        }
        
        I2C_Stop();
        
        humidity |= i2cRxBuffer[0] << 8;
        humidity |= i2cRxBuffer[1];
        
        *humidOut = -6.0f + (125.0f * ((float)humidity / (float)(1 << 16)));
        
    }while(0);
    
    taskEXIT_CRITICAL();
    
    return retVal;
}

I2C_Status ReadAirTempSensor(float* tempOut)
{
    uint16_t temperature = 0;
    I2C_Status retVal = I2C_OK;
    
    // This is a critical section: if the device is interrupted during an I2C transaction, it will probably fail.
    // Disable all interrupts during this transaction to ensure that it completes sucessfully.
    // NOTE: This means that the system clock will be wrong by ~15 ms (15 ticks) after this section of code is executed
    taskENTER_CRITICAL();
    
    // Use a do {} while(0); loop to allow the use of the "break" statement.
    // In C++ / Java this would be accomplished with a "try {} catch() {} finally {}" section, but C does not support this construct.
    do
    {
        // I2C Write
        I2C_Start(HTU21D_ADDR, 0); 
        retVal = I2C_WriteByte(HTU21D_TEMP_MEAS_HOLD);
        if(retVal != I2C_OK)
        {
            break;
        }
        retVal = I2C_WaitForTX();
        if(retVal != I2C_OK)
        {
            break;
        }
        
        // I2C Read
        I2C_Start(HTU21D_ADDR, 1);
        retVal = I2C_ReadBytes(i2cRxBuffer, 3);
        if(retVal != I2C_OK)
        {
            break;
        }
        I2C_Stop();
        
        temperature |= i2cRxBuffer[0] << 8;
        temperature |= i2cRxBuffer[1];
        
        *tempOut = -46.85f + (175.72f * ((float)temperature / (float)(1 << 16)));
        
    }while(0);
    
    taskEXIT_CRITICAL();
    
    return retVal;
}
