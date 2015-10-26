#include "sensors.h"
#include "hwctrl.h"
#include "cmsis_os.h"
#include "i2c.h"
#include "radio.h"
#include "sensor_conversions.h"
#include "radio_packets.h"
#include "debug.h"

// Global variables
I2C_HandleTypeDef   I2CxHandle;

uint8_t             i2cTxBuffer[I2C_BUFFER_SIZE];
uint8_t             i2cRxBuffer[I2C_BUFFER_SIZE];

osSemaphoreId       i2cSem;

SensorData          sensorData;

uint32_t            pollingRate = DEFAULT_POLL_RATE; // Poll once per minute by default

// Local function declarations
static uint8_t GetTmp102Addr(uint8_t index);
static I2C_Status InitTempSensor(uint8_t index);
static I2C_Status ReadTempSensor(uint8_t index, float* tempOut);
static I2C_Status ReadHumiditySensor(float* humidOut);
static I2C_Status InitHumiditySensor(void);
static I2C_Status ReadAirTempSensor(float* tempOut);
static I2C_Status ReprogramSoilMoistureAddress(void);
static I2C_Status InitSoilMoistureSensor(uint8_t sensor);
static I2C_Status ReadSoilMoisture(uint8_t sensor, float* out);
static I2C_Status PingI2C(uint8_t addr);
static uint8_t    GetSmsAddr(uint8_t index);
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
    uint8_t enabledSensors[7] = {3, 3, 3, 3, 3, 3, 3};
    uint8_t i;
    I2C_Status retVal = I2C_OK;
    
    INFO("(SENSORS_TASK) I2C Sensor failed to initialize\r\n");
    
    /*
    for(i = 0; i < 0x30; i++)
    {
        if(PingI2C(i) != I2C_OK)
        {
            I2C_Reset();
        }
        else
        {
            osDelay(1);
        }
    }
    */
    
    for(i = 0; i < 3; i++)
    {
        // If the temperature sensor initialized, set its enabled value to 3 (so it has 3 chances to respond to a read request)
        // Else, disable the sensor
        if(InitTempSensor(i) != I2C_OK)
        {
            I2C_Reset();
            enabledSensors[i] = 0;
            WARN("(SENSORS_TASK) I2C Sensor failed to initialize\r\n"); 
        }
    }
    
    if(InitHumiditySensor() != I2C_OK)
    {
        I2C_Reset();
        enabledSensors[3] = 0;
        WARN("(SENSORS_TASK) Humidity sensor failed to initialize\r\n");
    }
    
    for(i = 4; i < 7; i++)
    {
        // Initialize the soil sensors. If any sensor fails to initialize, disable it.
        if(InitSoilMoistureSensor(i - 4) != I2C_OK)
        {
            I2C_Reset();
            enabledSensors[i] = 0;
            WARN("(SENSORS_TASK) Soil Moisture sensor failed to initialize\r\n");
        }
    }
    
    // This code was used to initially reprogram the addresses on the soil moisture sensors.
    // It should NOT be enabled in the field
    /*    
    if(ReprogramSoilMoistureAddress() != I2C_OK)
    {
        osDelay(2000);
    }
    */
    
    // Let other tasks in the system warmup before entering the sensor polling loop
    osDelay(2000);
    
    // TODO: re-initialize the sensors once a day to check for failures and for sensors that have come back online
    // TODO: report to the base station when a sensor fails
    
    while(1)
    {
        for(i = 0; i < 3; i++)
        {
            if(enabledSensors[i] > 0)
            {
                switch(i)
                {
                    case 0:
                        retVal = ReadTempSensor(0, &sensorData.temp0);
                        break;
                    case 1:
                        retVal = ReadTempSensor(1, &sensorData.temp1);
                        break;
                    case 2:
                        retVal = ReadTempSensor(2, &sensorData.temp2);
                        break;
                    default:
                        retVal = ReadTempSensor(0, &sensorData.temp0);
                        break;
                }
                
                // If the sensor read failed, indicate that the sensor has one less chance to respond correctly before being disabled
                if(retVal != I2C_OK)
                {
                    I2C_Reset();
                    enabledSensors[i]--;
                    WARN("(SENSORS_TASK) Temp sensor read failed\r\n");
                }
                // The sensor is still alive! Restore it to a full 3 chances to respond
                else
                {
                    enabledSensors[i] = 3;
                    DEBUG("(SENSORS_TASK) Temp sensor connection restored\r\n");
                }
            }
        }
        
        if(enabledSensors[3] > 0)
        {
           do {
                if(ReadHumiditySensor(&sensorData.humid) != I2C_OK)
                {
                    I2C_Reset();
                    enabledSensors[3]--;
                    WARN("(SENSORS_TASK) Humidity sensor read failed\r\n");
                    break;
                }
                else
                {
                   enabledSensors[3] = 3;
                   DEBUG("(SENSORS_TASK) Humidity sensor connection restored\r\n");
                }
               
                if(ReadAirTempSensor(&sensorData.tempAir) != I2C_OK)
                {
                    I2C_Reset();
                    enabledSensors[3]--;
                    WARN("(SENSORS_TASK) Air temp sensor read failed\r\n");
                }
                else
                {
                   enabledSensors[3] = 3;
                   DEBUG("(SENSORS_TASK) Air temp sensor connection restored\r\n");
                }
            }
            while(0);
        }
        
        for(i = 4; i < 7; i++)
        {
            if(enabledSensors[i] > 0)
            {
                switch(i)
                {
                    case 4:
                        retVal = ReadSoilMoisture(0, &sensorData.moist0);
                        break;
                    case 5:
                        retVal = ReadSoilMoisture(1, &sensorData.moist1);
                        break;
                    case 6:
                        retVal = ReadSoilMoisture(2, &sensorData.moist2);
                        break;
                    default:
                        retVal = ReadSoilMoisture(0, &sensorData.moist0);
                        break;
                }
                
                // If the sensor read failed, indicate that the sensor has one less chance to respond correctly before being disabled
                if(retVal != I2C_OK)
                {
                    I2C_Reset();
                    enabledSensors[i]--;
                    WARN("(SENSORS_TASK) Soil moisture sensor read failed\r\n");
                }
                // The sensor is still alive! Restore it to a full 3 chances to respond
                else
                {
                    enabledSensors[i] = 3;
                    DEBUG("(SENSORS_TASK) Soil moisture sensor connection restored\r\n");
                }
            }
        }
        
        //ReadChipTemp(&sensorData.tempChip);
        
        // Send sensor Data to the base station
        SendSensorData();
        
        osDelay(pollingRate);
    }
}

void SendSensorData(void)
{
    uint16_t tmp = 0;
    sensor_message_t* radioMessage;
    
    // TODO: we should be wrapping this data in MAC layer data in the radio task, so
    // we shouldn't be allocating the full packet size here
    // TODO: do something clever if we ran out of RAM
    radioMessage = pvPortMalloc(RADIO_MAX_PACKET_LENGTH);
    
    radioMessage->cmd = SENSOR_MSG;
    
    // moist 0
    tmp = Float_To_SMS(sensorData.moist0);
    radioMessage->moisture0 = tmp;
    
    // moist 1
    tmp = Float_To_SMS(sensorData.moist1);
    radioMessage->moisture1 = tmp;
    
    // moist 2
    tmp = Float_To_SMS(sensorData.moist2);
    radioMessage->moisture2 = tmp;
    
    // humid
    tmp = Float_To_HTU21D_Humid(sensorData.humid);
    radioMessage->humid = tmp;
    
    // temp 0
    tmp = Float_To_TMP102(sensorData.temp0);
    radioMessage->temp0 = tmp;
    
    // temp 1
    tmp = Float_To_TMP102(sensorData.temp1);
    radioMessage->temp1 = tmp;
    
    // temp 2
    tmp = Float_To_TMP102(sensorData.temp2);
    radioMessage->temp2 = tmp;
   
    // air temp
    tmp = Float_To_HTU21D_Temp(sensorData.tempAir);
    radioMessage->air_temp = tmp;
    
    // TODO: find a way to collect battery and solar panel data
    
    // battery level
    radioMessage->battery_level = 0;
    
    // solar level
    radioMessage->solar_level = 0;
    
    DEBUG("(SENSORS_TASK) Sending sensor message to radio task\r\n");
    
    SendToBroadcast((uint8_t*)radioMessage, sizeof(sensor_message_t));
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

uint8_t GetSmsAddr(uint8_t index)
{
    uint8_t addr = SMS_0_ADDR;
    
    switch(index)
    {
        case 0:
            addr = SMS_0_ADDR;
            break;
        
        case 1:
            addr = SMS_1_ADDR;
            break;
        
        case 2:
            addr = SMS_2_ADDR;
            break;
        
        default:
            addr = SMS_0_ADDR;
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
        
        *tempOut = TMP102_To_Float(temp);
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
        
        // I2C Write
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
        
        *humidOut = HTU21D_Humid_To_Float(humidity);
        
    }while(0);
    
    taskEXIT_CRITICAL();
    
    return retVal;
}

I2C_Status InitSoilMoistureSensor(uint8_t sensor)
{
    // These sensors require no initialization for now
    return I2C_OK;
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
        
        *tempOut = HTU21D_Temp_To_Float(temperature);
        
    }while(0);
    
    taskEXIT_CRITICAL();
    
    return retVal;
}

I2C_Status ReprogramSoilMoistureAddress(void)
{
    I2C_Status retVal = I2C_OK;
    
    // This is a critical section: if the device is interrupted during an I2C transaction, it will probably fail.
    // Disable all interrupts during this transaction to ensure that it completes sucessfully.
    // NOTE: This means that the system clock will be wrong by ~15 ms (15 ticks) after this section of code is executed
    taskENTER_CRITICAL();
    
    // Use a do {} while(0); loop to allow the use of the "break" statement.
    // In C++ / Java this would be accomplished with a "try {} catch() {} finally {}" section, but C does not support this construct.
    do
    {
        i2cTxBuffer[0] = SMS_SET_ADDRESS;
        i2cTxBuffer[1] = 0x21;
        
        // I2C Write
        I2C_Start(SMS_0_ADDR, I2C_WRITE); 
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
        
        // I2C Write
        I2C_Start(SMS_0_ADDR, I2C_WRITE);
        retVal = I2C_WriteByte(0x06);
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
                
    } while(0);
    
    taskEXIT_CRITICAL();
    
    return retVal;
}

I2C_Status ReadSoilMoisture(uint8_t sensor, float* out)
{
    uint16_t capacitance = 0;
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
        I2C_Start(GetSmsAddr(sensor), I2C_WRITE); 
        retVal = I2C_WriteByte(SMS_GET_CAPACITANCE);
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
        I2C_Start(GetSmsAddr(sensor), I2C_READ);
        retVal = I2C_ReadBytes(i2cRxBuffer, 2);
        if(retVal != I2C_OK)
        {
            break;
        }
        I2C_Stop();
        
        capacitance |= i2cRxBuffer[0] << 8;
        capacitance |= i2cRxBuffer[1];
        
        *out = SMS_To_Float(capacitance);
        
    } while(0);
    
    taskEXIT_CRITICAL();
    
    return retVal;
}


I2C_Status PingI2C(uint8_t addr)
{
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
        I2C_Start(addr, I2C_WRITE); 
        retVal = I2C_WriteByte(SMS_GET_ADDRESS);
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
        I2C_Start(addr, I2C_READ);
        retVal = I2C_ReadBytes(i2cRxBuffer, 1);
        if(retVal != I2C_OK)
        {
            break;
        }
        I2C_Stop();
        
    } while(0);
    
    taskEXIT_CRITICAL();
    
    return retVal;
}

void SensorsChangePollingRate(uint32_t ms)
{
    pollingRate = ms;
}

uint32_t SensorsGetPollingRate(void)
{
    return pollingRate;
}