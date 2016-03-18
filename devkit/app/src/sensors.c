#include "sensors.h"
#include "hwctrl.h"
#include "cmsis_os.h"
#include "i2c.h"
#include "radio.h"
#include "sensor_conversions.h"
#include "radio_packets.h"
#include "debug.h"
#include "power.h"
#include "capacitance.h"

// Global variables
I2C_HandleTypeDef   ALBI2CHandle;
I2C_HandleTypeDef   SLBI2CHandle;

uint8_t             i2cTxBuffer[I2C_BUFFER_SIZE];
uint8_t             i2cRxBuffer[I2C_BUFFER_SIZE];

osSemaphoreId       i2cSem;

SensorData          sensorData;

uint32_t            pollingRate = DEFAULT_POLL_RATE; // Poll once per minute by default

// Local function declarations
static void       ReadSoilMoisture(float* moist1, float* moist2, float* moist3);
static uint8_t    GetTmp102Addr(uint8_t index);
static I2C_Status InitTempSensor(uint8_t index);
static I2C_Status ReadTempSensor(uint8_t index, uint16_t* tempOut);
static I2C_Status ReadHumiditySensor(float* humidOut);
static I2C_Status InitHumiditySensor(void);
static I2C_Status InitPressureSensor(void);
static I2C_Status ReadPressureSensor(uint32_t* altOut);
static I2C_Status InitAccelerometer(void);
static I2C_Status ReadAccelerometer(uint8_t* interrupts);
static I2C_Status ReadAirTempSensor(float* tempOut);
static uint8_t    GetSmsAddr(uint8_t index);
static void       SendSensorData(void);

// Global function implementations
void SensorsTaskHwInit(void) 
{    
    GPIO_InitTypeDef  GPIO_InitStruct;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    ALB_I2C_SCL_GPIO_CLK_ENABLE();
    ALB_I2C_SDA_GPIO_CLK_ENABLE();
    
    SLB_I2C_SCL_GPIO_CLK_ENABLE();
    SLB_I2C_SDA_GPIO_CLK_ENABLE();
    
    /* Enable I2Cx clock */
    ALB_I2C_CLK_ENABLE(); 
    SLB_I2C_CLK_ENABLE(); 
    
    /*##-2- Configure peripheral GPIO ##########################################*/  
    /* I2C TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = ALB_I2C_SCL_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = ALB_I2C_SCL_AF;

    HAL_GPIO_Init(ALB_I2C_SCL_GPIO_PORT, &GPIO_InitStruct);

    /* I2C RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = ALB_I2C_SDA_PIN;
    GPIO_InitStruct.Alternate = ALB_I2C_SDA_AF;

    HAL_GPIO_Init(ALB_I2C_SDA_GPIO_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin       = SLB_I2C_SCL_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = SLB_I2C_SCL_AF;

    HAL_GPIO_Init(SLB_I2C_SCL_GPIO_PORT, &GPIO_InitStruct);

    /* I2C RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = SLB_I2C_SDA_PIN;
    GPIO_InitStruct.Alternate = SLB_I2C_SDA_AF;

    HAL_GPIO_Init(SLB_I2C_SDA_GPIO_PORT, &GPIO_InitStruct);
    
    // Configure the capsense driver
    CapacitanceInit();
}

void SensorsTaskOSInit(void)
{   
    osSemaphoreDef(i2cTransactSem);
        
    i2cSem = osSemaphoreCreate(osSemaphore(i2cTransactSem), 1);
    
    ALBI2CHandle.Instance              = ALB_I2C;
    ALBI2CHandle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    ALBI2CHandle.Init.Timing           = I2C_TIMING_100KHZ;
    ALBI2CHandle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLED;
    ALBI2CHandle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    ALBI2CHandle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLED;
    ALBI2CHandle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLED;
    ALBI2CHandle.Init.OwnAddress1      = 0x00;
    ALBI2CHandle.Init.OwnAddress2      = 0x00;
    
    assert_param(HAL_I2C_Init(&ALBI2CHandle) == HAL_OK);
    
    SLBI2CHandle.Instance              = SLB_I2C;
    SLBI2CHandle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    SLBI2CHandle.Init.Timing           = I2C_TIMING_100KHZ;
    SLBI2CHandle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLED;
    SLBI2CHandle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    SLBI2CHandle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLED;
    SLBI2CHandle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLED;
    SLBI2CHandle.Init.OwnAddress1      = 0x00;
    SLBI2CHandle.Init.OwnAddress2      = 0x00;
    
    assert_param(HAL_I2C_Init(&SLBI2CHandle) == HAL_OK);
}

void SensorsTask(void)
{   
    // Array storing the status of each physical sensor.
    // If a sensor fails to initialize, or fails 3 sensor reads in a row, 
    // it will be disabled here until the next system reboot
    // 0 = tmp0
    // 1 = tmp1
    // 2 = tmp3
    // 3 = humidity / air temp
    // 4 = pressure
    // 5 = accelerometer
    uint8_t enabledSensors[7] = {3, 3, 3, 3, 3, 3, 3};
    uint8_t i;
    I2C_Status retVal = I2C_OK;
    
    INFO("(SENSORS_TASK) I2C Sensor failed to initialize\r\n");
    
    for(i = 0; i < 3; i++)
    {
        // If the temperature sensor initialized, set its enabled value to 3 (so it has 3 chances to respond to a read request)
        // Else, disable the sensor
        if(InitTempSensor(i) != I2C_OK)
        {
            I2C_Reset(SLB_I2C);
            enabledSensors[i] = 0;
            WARN("(SENSORS_TASK) I2C Sensor failed to initialize\r\n"); 
        }
    }
    
    if(InitHumiditySensor() != I2C_OK)
    {
        I2C_Reset(ALB_I2C);
        enabledSensors[3] = 0;
        WARN("(SENSORS_TASK) Humidity sensor failed to initialize\r\n");
    }
    
    if(InitPressureSensor() != I2C_OK)
    {
        I2C_Reset(ALB_I2C);
        enabledSensors[4] = 0;
        WARN("(SENSORS_TASK) Pressure sensor failed to initialize\r\n");
    }
    
    if(InitAccelerometer() != I2C_OK)
    {
        I2C_Reset(ALB_I2C);
        enabledSensors[5] = 0;
        WARN("(SENSORS_TASK) Accelerometer failed to initialize\r\n");
    }
    
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
                    I2C_Reset(SLB_I2C);
                    enabledSensors[i]--;
                    WARN("(SENSORS_TASK) Temp sensor read failed\r\n");
                }
                // The sensor is still alive! Restore it to a full 3 chances to respond
                else if(enabledSensors[i] != 3)
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
                    I2C_Reset(ALB_I2C);
                    enabledSensors[3]--;
                    WARN("(SENSORS_TASK) Humidity sensor read failed\r\n");
                    break;
                }
                else if(enabledSensors[3] != 3)
                {
                   enabledSensors[3] = 3;
                   DEBUG("(SENSORS_TASK) Humidity sensor connection restored\r\n");
                }
               
                if(ReadAirTempSensor(&sensorData.tempAir) != I2C_OK)
                {
                    I2C_Reset(ALB_I2C);
                    enabledSensors[3]--;
                    WARN("(SENSORS_TASK) Air temp sensor read failed\r\n");
                }
                else if(enabledSensors[3] != 3)
                {
                   enabledSensors[3] = 3;
                   DEBUG("(SENSORS_TASK) Air temp sensor connection restored\r\n");
                }
            }
            while(0);
        }
        
        if(enabledSensors[4] > 0)
        {
            if(ReadPressureSensor(&sensorData.alt) != I2C_OK)
            {
                I2C_Reset(ALB_I2C);
                enabledSensors[4]--;
                WARN("(SENSORS_TASK) Altimeter sensor read failed\r\n");
            }
        }
        
        if(enabledSensors[5] > 0)
        {
            if(ReadAccelerometer(&sensorData.acc) != I2C_OK)
            {
                I2C_Reset(ALB_I2C);
                enabledSensors[5]--;
                WARN("(SENSORS_TASK) Accelerometer sensor read failed\r\n");
            }
        }
        
        ReadSoilMoisture(&sensorData.moist0, &sensorData.moist1, &sensorData.moist2);
        
        // Send sensor Data to the base station
        SendSensorData();
        
        osDelay(pollingRate);
    }
}

void SendSensorData(void)
{
    uint16_t tmp = 0;
    generic_message_t* radioMessage;
    
    // TODO: we should be wrapping this data in MAC layer data in the radio task, so
    // we shouldn't be allocating the full packet size here
    // TODO: do something clever if we ran out of RAM
    radioMessage = pvPortMalloc(RADIO_MAX_PACKET_LENGTH);
    
    radioMessage->cmd = SENSOR_MSG;
    
    // moist 0
    tmp = Float_To_SMS(sensorData.moist0);
    radioMessage->payload.sensor_message.moisture0 = tmp;
    
    // moist 1
    tmp = Float_To_SMS(sensorData.moist1);
    radioMessage->payload.sensor_message.moisture1 = tmp;
    
    // moist 2
    tmp = Float_To_SMS(sensorData.moist2);
    radioMessage->payload.sensor_message.moisture2 = tmp;
    
    // humid
    tmp = Float_To_HTU21D_Humid(sensorData.humid);
    radioMessage->payload.sensor_message.humid = tmp;
    
    // temp 0
    radioMessage->payload.sensor_message.temp0 = sensorData.temp0;
    
    // temp 1
    radioMessage->payload.sensor_message.temp1 = sensorData.temp1;
    
    // temp 2
    radioMessage->payload.sensor_message.temp2 = sensorData.temp2;
   
    // air temp
    tmp = Float_To_HTU21D_Temp(sensorData.tempAir);
    radioMessage->payload.sensor_message.air_temp = tmp;
    
    // battery level
    radioMessage->payload.sensor_message.battery_level = GetBatteryVoltage();
    
    // Chip temperature
    radioMessage->payload.sensor_message.chip_temp = GetChipTemperature();
    
    // Sensor altitude
    radioMessage->payload.sensor_message.alt = sensorData.alt;
    
    // Sensor acceleration
    radioMessage->payload.sensor_message.acc = sensorData.acc;
    
    DEBUG("(SENSORS_TASK) Sending sensor message to radio task\r\n");
    
    SendToBaseStation((uint8_t*)radioMessage, sizeof(generic_message_t));
}

void SensorsCmd(sensor_cmd_payload_t cmd)
{
    // Field 0 == sensor polling rate change
    if(cmd.valid_fields & 0x00000001)
    {
        DEBUG("COMMAND FROM BASE STATION: Changing sensor polling period to %d", cmd.sensor_polling_period);
        SensorsChangePollingRate(cmd.sensor_polling_period);
    }
    
    // Room for future expansion here
    
    if(cmd.valid_fields & 0x40000000)
    {
        // Set SLEEPDEEP bit
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        // Ensure PDDS bit = 0
        // Ensure WUF bit = 0

        // STOPWUCK selects HSI16 as wakeup clock
        
        // Disable SysTick interrupt
        NVIC_DisableIRQ(SysTick_IRQn);
        
        __WFI();
        
        // Reset the SLEEPDEEP bit
        SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPDEEP_Msk);

        // Reconfigure system clock
        SystemClock_Config();
        
        NVIC_EnableIRQ(SysTick_IRQn);
    }
    
    if(cmd.valid_fields & 0x80000000)
    {
        DEBUG("COMMAND FROM BASE STATION: Resetting...\r\n");
        NVIC_SystemReset();
    }
}

// Local function implementations

void ReadSoilMoisture(float* moist1, float* moist2, float* moist3)
{
    uint32_t capsense1, capsense2, capsense3;
    CapacitanceRead(&capsense1, &capsense2, &capsense3);
    
    // TODO: real moisture calibration equation here. Needs in-air calibration data first.
    *moist1 = capsense1;
    *moist2 = capsense2;
    *moist3 = capsense3;
}

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
I2C_Status ReadTempSensor(uint8_t index, uint16_t* tempOut)
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
        I2C_Start(SLB_I2C, GetTmp102Addr(index), 0); 
        if((retVal = I2C_WriteByte(SLB_I2C, TMP102_CONFIG_ADDR)) != I2C_OK)
        {
            break;
        }
        
        if((retVal = I2C_WaitForTX(SLB_I2C)) != I2C_OK)
        {
            break;
        }
            
        I2C_Stop(SLB_I2C);
        
        // I2C Write
        I2C_Start(SLB_I2C, GetTmp102Addr(index), 0); 
        
        if((retVal = I2C_WriteBytes(SLB_I2C, i2cTxBuffer, 2)) != I2C_OK)
        {
            break;
        }
        
        if((retVal = I2C_WaitForTX(SLB_I2C)) != I2C_OK)
        {
            break;
        }
        
        I2C_Stop(SLB_I2C);       
        
        // Read the temperature registers from the sensor
        // I2C Write
        I2C_Start(SLB_I2C, GetTmp102Addr(index), 0); 
        
        if((retVal = I2C_WriteByte(SLB_I2C, TMP102_TEMP_ADDR)) != I2C_OK)
        {
            break;
        }
        
        if((retVal = I2C_WaitForTX(SLB_I2C)) != I2C_OK)
        {
            break;
        }
        
        I2C_Stop(SLB_I2C);
        
        // I2C Read
        I2C_Start(SLB_I2C, GetTmp102Addr(index), 1);
        
        if((retVal = I2C_ReadBytes(SLB_I2C, i2cRxBuffer, 2)) != I2C_OK)
        {
            break;
        }
        
        I2C_Stop(SLB_I2C);

        // Process returned data
        temp = (((uint16_t)i2cRxBuffer[0]) << 4) & 0xFFF0;
        temp |= (((uint16_t)i2cRxBuffer[1]) >> 4) & 0x000F;
        
        *tempOut = temp;
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
        I2C_Start(SLB_I2C, GetTmp102Addr(index), 0);
        retVal = I2C_WriteByte(SLB_I2C, TMP102_CONFIG_ADDR);
        if(retVal != I2C_OK)
        {
            break;
        }
        retVal = I2C_WaitForTX(SLB_I2C);
        if(retVal != I2C_OK)
        {
            break;
        }
        I2C_Stop(SLB_I2C);
        
        // I2C Write
        I2C_Start(SLB_I2C, GetTmp102Addr(index), 0);
        retVal = I2C_WriteBytes(SLB_I2C, i2cTxBuffer, 2);
        if(retVal != I2C_OK)
        {
            break;
        }
        retVal = I2C_WaitForTX(SLB_I2C);
        if(retVal != I2C_OK)
        {
            break;
        }
        I2C_Stop(SLB_I2C);
        
        // I2C Read
        I2C_Start(SLB_I2C, GetTmp102Addr(index), 1);
        
        if((retVal = I2C_ReadBytes(SLB_I2C, i2cRxBuffer, 2)) != I2C_OK)
        {
            break;
        }
        
        I2C_Stop(SLB_I2C);
        
    }while(0);
    
    taskEXIT_CRITICAL();
    
    return retVal;
}

I2C_Status InitHumiditySensor(void)
{
    // Humidity sensor does not need initialization
    return I2C_OK;
}

I2C_Status PressureSensorRegWrite(uint8_t reg, uint8_t val)
{
    I2C_Status retVal = I2C_OK;
    uint8_t tmp[2];
    
    tmp[0] = reg;
    tmp[1] = val;
    
    do
    {
        I2C_Start(ALB_I2C, MPL311_ADDR, 0);
        retVal = I2C_WriteBytes(ALB_I2C, tmp, 2);
        if(retVal != I2C_OK)
        {
            break;
        }
        
        retVal = I2C_WaitForTX(ALB_I2C);
        if(retVal != I2C_OK)
        {
            break;
        }
        
        I2C_Stop(ALB_I2C);
    } while(0);
    
    return retVal;
}

I2C_Status PressureSensorRegRead(uint8_t reg, uint8_t* val)
{
    I2C_Status retVal = I2C_OK;
    
    do
    {
        I2C_Start(ALB_I2C, MPL311_ADDR, 0);
        retVal = I2C_WriteByte(ALB_I2C, reg);
        if(retVal != I2C_OK)
        {
            break;
        }
        retVal = I2C_WaitForTX(ALB_I2C);
        if(retVal != I2C_OK)
        {
            break;
        }
        
        I2C_Start(ALB_I2C, MPL311_ADDR, 1);
        
        if((retVal = I2C_ReadBytes(ALB_I2C, val, 1)) != I2C_OK)
        {
            break;
        }
        
        I2C_Stop(ALB_I2C);
    } while(0);
    
    return retVal;
}

I2C_Status InitPressureSensor(void)
{
    I2C_Status retVal = I2C_OK;
        
    taskENTER_CRITICAL();
    
    do
    {
        retVal = PressureSensorRegWrite(MPL3115A2_CTRL_REG1, MPL3115A2_CTRL_REG1_SBYB | MPL3115A2_CTRL_REG1_OS128 | MPL3115A2_CTRL_REG1_ALT);
        
        if(retVal != I2C_OK)
        {
            break;
        }
        
        retVal = PressureSensorRegWrite(MPL3115A2_PT_DATA_CFG, MPL3115A2_PT_DATA_CFG | MPL3115A2_PT_DATA_CFG_PDEFE | MPL3115A2_PT_DATA_CFG_DREM);
    }while(0);
    
    taskEXIT_CRITICAL();
    
    return retVal;
}

// Read the altimeter. This function will 
// block the calling task until the entire sensor read has been completed
I2C_Status ReadPressureSensor(uint32_t* altOut)
{
    I2C_Status retVal = I2C_OK;
    uint32_t temp = 0;
    
    taskENTER_CRITICAL();
    
    do
    {
        retVal = PressureSensorRegWrite(MPL3115A2_CTRL_REG1, MPL3115A2_CTRL_REG1_SBYB | MPL3115A2_CTRL_REG1_OS128 | MPL3115A2_CTRL_REG1_ALT);

        if(retVal != I2C_OK)
        {
            break;
        }
        
        uint8_t sta = 0;
        while (! (sta & MPL3115A2_REGISTER_STATUS_PDR)) {
            retVal = PressureSensorRegRead(MPL3115A2_REGISTER_STATUS, &sta);
            // TODO: a delay here would be nice to prevent slamming the I2C bus
        }
        
        I2C_Start(ALB_I2C, MPL311_ADDR, 0);
        
        retVal = I2C_WriteByte(ALB_I2C, MPL3115A2_REGISTER_PRESSURE_MSB);
        
        if(retVal != I2C_OK)
        {
            break;
        }
        
        retVal = I2C_WaitForTX(ALB_I2C);
        
        if(retVal != I2C_OK)
        {
            break;
        }
        
        I2C_Start(ALB_I2C, MPL311_ADDR, 1);
        
        if((retVal = I2C_ReadBytes(ALB_I2C, i2cRxBuffer, 3)) != I2C_OK)
        {
            break;
        }
        
        I2C_Stop(ALB_I2C);
        
        // Process returned data
        temp = i2cRxBuffer[0] << 24;
        temp |= i2cRxBuffer[1] << 16;
        temp |= i2cRxBuffer[2] << 8;
        
        *altOut = temp;
    } while(0);
    
    taskEXIT_CRITICAL();
    
    return retVal;
}

I2C_Status AccelerometerRegWrite(uint8_t reg, uint8_t val)
{
    I2C_Status retVal = I2C_OK;
    uint8_t tmp[2];
    
    tmp[0] = reg;
    tmp[1] = val;
    
    do
    {
        I2C_Start(ALB_I2C, LIS2DH_ADDR, 0);
        retVal = I2C_WriteBytes(ALB_I2C, tmp, 2);
        if(retVal != I2C_OK)
        {
            break;
        }
        
        retVal = I2C_WaitForTX(ALB_I2C);
        if(retVal != I2C_OK)
        {
            break;
        }
        
        I2C_Stop(ALB_I2C);
    } while(0);
    
    return retVal;
}

I2C_Status AccelerometerRegRead(uint8_t reg, uint8_t* val)
{
    I2C_Status retVal = I2C_OK;
    
    do
    {
        I2C_Start(ALB_I2C, LIS2DH_ADDR, 0);
        retVal = I2C_WriteByte(ALB_I2C, reg);
        if(retVal != I2C_OK)
        {
            break;
        }
        retVal = I2C_WaitForTX(ALB_I2C);
        if(retVal != I2C_OK)
        {
            break;
        }
        
        I2C_Start(ALB_I2C, LIS2DH_ADDR, 1);
        
        if((retVal = I2C_ReadBytes(ALB_I2C, val, 1)) != I2C_OK)
        {
            break;
        }
        
        I2C_Stop(ALB_I2C);
    } while(0);
    
    return retVal;
}

I2C_Status InitAccelerometer(void)
{
    I2C_Status retVal = I2C_OK;
        
    taskENTER_CRITICAL();
    
    do
    {
        retVal = AccelerometerRegWrite(LIS2DH_CTRL_REG1, LIS2DH_CTRL_REG1_VAL);
        
        if(retVal != I2C_OK)
        {
            break;
        }
        
        retVal = AccelerometerRegWrite(LIS2DH_CTRL_REG4, LIS2DH_CTRL_REG4_VAL);
        
        if(retVal != I2C_OK)
        {
            break;
        }
        
        retVal = AccelerometerRegWrite(LIS2DH_INT_CFG_REG, LIS2DH_INT_CFG_REG_VAL);
        
        if(retVal != I2C_OK)
        {
            break;
        }
        
        retVal = AccelerometerRegWrite(LIS2DH_INT_THS_REG, LIS2DH_INT_THS_REG_VAL);
        
        if(retVal != I2C_OK)
        {
            break;
        }
        
        uint8_t val;
        
        retVal = AccelerometerRegRead(LIS2DH_WHOAMI_REG, &val);
        
        if(val != LIS2DH_WHOAMI_REG_VAL)
        {
            retVal = I2C_TIMEOUT;
        }
        
    }while(0);
    
    taskEXIT_CRITICAL();
    
    return retVal;
}

I2C_Status ReadAccelerometer(uint8_t* interrupts)
{
    I2C_Status retVal = I2C_OK;
        
    taskENTER_CRITICAL();
    
    do
    {
        retVal = AccelerometerRegRead(LIS2DH_INT_COUNTER_REG, interrupts);
        
        if(retVal != I2C_OK)
        {
            break;
        }
        
    }while(0);
    
    taskEXIT_CRITICAL();
    
    return retVal;
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
        I2C_Start(ALB_I2C, HTU21D_ADDR, 0); 
        
        retVal = I2C_WriteByte(ALB_I2C, HTU21D_HUMID_MEAS_HOLD);
        if(retVal != I2C_OK)
        {
            break;
        }
        
        retVal = I2C_WaitForTX(ALB_I2C);
        if(retVal != I2C_OK)
        {
            break;
        }
        
        // I2C Read
        I2C_Start(ALB_I2C, HTU21D_ADDR, 1);
        
        retVal = I2C_ReadBytes(ALB_I2C, i2cRxBuffer, 3);
        if(retVal != I2C_OK)
        {
            break;
        }
        
        I2C_Stop(ALB_I2C);
        
        humidity |= i2cRxBuffer[0] << 8;
        humidity |= i2cRxBuffer[1];
        
        *humidOut = HTU21D_Humid_To_Float(humidity);
        
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
        I2C_Start(ALB_I2C, HTU21D_ADDR, 0); 
        retVal = I2C_WriteByte(ALB_I2C, HTU21D_TEMP_MEAS_HOLD);
        if(retVal != I2C_OK)
        {
            break;
        }
        retVal = I2C_WaitForTX(ALB_I2C);
        if(retVal != I2C_OK)
        {
            break;
        }
        
        // I2C Read
        I2C_Start(ALB_I2C, HTU21D_ADDR, 1);
        retVal = I2C_ReadBytes(ALB_I2C, i2cRxBuffer, 3);
        if(retVal != I2C_OK)
        {
            break;
        }
        I2C_Stop(ALB_I2C);
        
        temperature |= i2cRxBuffer[0] << 8;
        temperature |= i2cRxBuffer[1];
        
        *tempOut = HTU21D_Temp_To_Float(temperature);
        
    }while(0);
    
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