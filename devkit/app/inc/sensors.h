#ifndef _SENSORS_H
#define _SENSORS_H

#include "stm32l0xx_hal.h"

// DEFINES

#ifdef DEVKIT

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

#elif DANDELION

    // TODO: setup the second I2C device

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
    
    #define CAP_IN_1_PIN                    GPIO_PIN_0
    #define CAP_IN_1_PORT                   GPIOA
    #define CAP_IN_1_CLK_ENABLE()           __GPIOA_CLK_ENABLE()
    #define CAP_IN_1_TSC                    TSC_GROUP1_IO1
    
    #define CAP_SENSE_1_PIN                 GPIO_PIN_1
    #define CAP_SENSE_1_PORT                GPIOA
    #define CAP_SENSE_1_CLK_ENABLE()        __GPIOA_CLK_ENABLE()
    #define CAP_SENSE_1_TSC                 TSC_GROUP1_IO2
    
    #define CAP_IN_2_PIN                    GPIO_PIN_6
    #define CAP_IN_2_PORT                   GPIOA
    #define CAP_IN_2_CLK_ENABLE()           __GPIOA_CLK_ENABLE()
    #define CAP_IN_2_TSC                    TSC_GROUP2_IO3
    
    #define CAP_SENSE_2_PIN                 GPIO_PIN_7
    #define CAP_SENSE_2_PORT                GPIOA
    #define CAP_SENSE_2_CLK_ENABLE()        __GPIOA_CLK_ENABLE()
    #define CAP_SENSE_2_TSC                 TSC_GROUP2_IO4
    
    #define CAP_IN_3_PIN                    GPIO_PIN_0
    #define CAP_IN_3_PORT                   GPIOB
    #define CAP_IN_3_CLK_ENABLE()           __GPIOB_CLK_ENABLE()
    #define CAP_IN_3_TSC                    TSC_GROUP3_IO2
    
    #define CAP_SENSE_3_PIN                 GPIO_PIN_1
    #define CAP_SENSE_3_PORT                GPIOB
    #define CAP_SENSE_3_CLK_ENABLE()        __GPIOB_CLK_ENABLE()
    #define CAP_SENSE_3_TSC                 TSC_GROUP3_IO3
    
#else
    #error "Device type not selected. Define DEVKIT or DANDELION in project settings"
#endif

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

// HTU21D (Humidity and air temperature sensor) definitions
#define HTU21D_ADDR                 0x40

#define HTU21D_TEMP_MEAS_HOLD       0xE3
#define HTU21D_HUMID_MEAS_HOLD      0xE5
#define HTU21D_TEMP_MEAS_NO_HOLD    0xF3
#define HTU21D_HUMID_MEAS_NO_HOLD   0xF5
#define HTU21D_WRITE_USER_REG       0xE6
#define HTU21D_READ_USER_REG        0xE7
#define HTU21D_SOFT_RESET           0xFE

// Soil moisture sensor probes
#define SMS_0_ADDR                  0x20
#define SMS_1_ADDR                  0x2E
#define SMS_2_ADDR                  0x24

#define SMS_GET_CAPACITANCE         0x00
#define SMS_SET_ADDRESS             0x01
#define SMS_GET_ADDRESS             0x02

#define DEFAULT_POLL_RATE           5 * 1000

void        SensorsTaskOSInit(void);
void        SensorsTaskHwInit(void);
void        SensorsTask(void);
void        SensorsChangePollingRate(uint32_t ms);
uint32_t    SensorsGetPollingRate(void);

#endif // _SENSORS_H
