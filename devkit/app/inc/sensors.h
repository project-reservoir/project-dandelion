#ifndef _SENSORS_H
#define _SENSORS_H

#include "stm32l0xx_hal.h"

// DEFINES
#define I2Cx                            I2C1
#define I2Cx_CLK_ENABLE()               __I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __GPIOB_CLK_ENABLE() 

#define I2Cx_FORCE_RESET()              __I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()            __I2C1_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_6
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SCL_AF                     GPIO_AF1_I2C1
#define I2Cx_SDA_PIN                    GPIO_PIN_7
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SDA_AF                     GPIO_AF1_I2C1

/* Definition for I2Cx's NVIC */
#define I2Cx_IRQn                       I2C1_IRQn
#define I2Cx_Handler                    I2C1_Handler
                                        
#define I2C_TIMING_100KHZ               0x10A13E56 /* Analog Filter ON, Rise Time 400ns, Fall Time 100ns */ 
#define I2C_TIMING_400KHZ               0x00B1112E /* Analog Filter ON, Rise Time 250ns, Fall Time 100ns */
#define I2C_TIMING_300KHZ               0x0060134B /* Analog Filert ON, Rise Time 100ns, Fall Time 10ns */
#define I2C_TIMING_300KHZ_NEW           0x00300619

#define I2C_BUFFER_SIZE                 2

#define I2C_ADDRESS                     0x01


// TMP102 (Temperature Sensor) definitions
#define TMP102_0_ADDR               0x48
#define TMP102_1_ADDR               0x49
#define TMP102_2_ADDR               0x4A

#define TMP102_CONFIG_1_ONESHOT_VAL 0xE1
#define TMP102_CONFIG_1_VAL         0x41
#define TMP102_CONFIG_2_VAL         0xA0

#define TMP102_TEMP_ADDR            0x00
#define TMP102_CONFIG_ADDR          0x01
#define TMP102_TLOW_ADDR            0x02
#define TMP102_THIGH_ADDR           0x03
#define TMP102_LSB_INC              0.0625f

// HTU21D (Humidity and air temperature sensor) definitions
#define HTU21D_ADDR                 0x40

#define HTU21D_TEMP_MEAS_HOLD       0xE3
#define HTU21D_HUMID_MEAS_HOLD      0xE5
#define HTU21D_TEMP_MEAS_NO_HOLD    0xF3
#define HTU21D_HUMID_MEAS_NO_HOLD   0xF5
#define HTU21D_WRITE_USER_REG       0xE6
#define HTU21D_READ_USER_REG        0xE7
#define HTU21D_SOFT_RESET           0xFE


typedef struct SensorData_t {
    float temp0;
    float temp1;
    float temp2;
    float tempChip;
    float tempRadio;
    float tempAir;
    float moist0;
    float moist1;
    float moist2;
    float humid;
} SensorData;

void SensorsTaskOSInit(void);
void SensorsTaskHwInit(void);
void SensorsTask(void);

#endif // _SENSORS_H
