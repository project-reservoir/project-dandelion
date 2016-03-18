#ifndef _SENSORS_H
#define _SENSORS_H

#include "stm32l0xx_hal.h"
#include "radio_packets.h"

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
    
    // SLB I2C
    #define SLB_I2C                         I2C2
    #define SLB_I2C_CLK_ENABLE()            __I2C2_CLK_ENABLE()
    #define SLB_I2C_SDA_GPIO_CLK_ENABLE()   __GPIOB_CLK_ENABLE()
    #define SLB_I2C_SCL_GPIO_CLK_ENABLE()   __GPIOB_CLK_ENABLE() 

    #define SLB_I2C_FORCE_RESET()           __I2C2_FORCE_RESET()
    #define SLB_I2C_RELEASE_RESET()         __I2C2_RELEASE_RESET()

    /* Definition for SLB_I2C Pins */
    #define SLB_I2C_SCL_PIN                 GPIO_PIN_10
    #define SLB_I2C_SCL_GPIO_PORT           GPIOB
    #define SLB_I2C_SCL_AF                  GPIO_AF6_I2C2
    
    #define SLB_I2C_SDA_PIN                 GPIO_PIN_11
    #define SLB_I2C_SDA_GPIO_PORT           GPIOB
    #define SLB_I2C_SDA_AF                  GPIO_AF6_I2C2

    /* Definition for I2Cx's NVIC */
    #define SLB_I2C_IRQn                    I2C2_IRQn
    #define SLB_I2C_Handler                 I2C2_Handler
    
    // ALB I2C
    #define ALB_I2C                         I2C1
    #define ALB_I2C_CLK_ENABLE()            __I2C1_CLK_ENABLE()
    #define ALB_I2C_SDA_GPIO_CLK_ENABLE()   __GPIOB_CLK_ENABLE()
    #define ALB_I2C_SCL_GPIO_CLK_ENABLE()   __GPIOB_CLK_ENABLE() 

    #define ALB_I2C_FORCE_RESET()           __I2C1_FORCE_RESET()
    #define ALB_I2C_RELEASE_RESET()         __I2C1_RELEASE_RESET()

    /* Definition for I2Cx Pins */
    #define ALB_I2C_SCL_PIN                 GPIO_PIN_6
    #define ALB_I2C_SCL_GPIO_PORT           GPIOB
    #define ALB_I2C_SCL_AF                  GPIO_AF1_I2C1
    
    #define ALB_I2C_SDA_PIN                 GPIO_PIN_7
    #define ALB_I2C_SDA_GPIO_PORT           GPIOB
    #define ALB_I2C_SDA_AF                  GPIO_AF1_I2C1

    /* Definition for I2Cx's NVIC */
    #define ALB_I2C_IRQn                    I2C1_IRQn
    #define ALB_I2C_Handler                 I2C1_Handler
    
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

#define I2C_BUFFER_SIZE                 4

#define I2C_ADDRESS                     0x01


// TMP102 (Temperature Sensor) definitions
#define TMP102_0_ADDR               0x48
#define TMP102_1_ADDR               0x49
#define TMP102_2_ADDR               0x4A

#define TMP102_CONFIG_1_ONESHOT_VAL 0xE1
#define TMP102_CONFIG_1_VAL         0x61
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

// MPL311 (Air Pressure sensor) definitions
#define MPL311_ADDR                 0x60
#define MPL311_CONFIG_1_VAL         0x80
#define MPL311_CONFIG_1_REG         0x26
#define MPL311_CONFIG_2_VAL         0x07
#define MPL311_CONFIG_2_REG         0x13
#define MPL311_ALT_MSB_REG          0x01
#define MPL311_ALT_CSB_REG          0x02
#define MPL311_ALT_LSB_REG          0x03
#define MPL311_TMP_MSB_REG          0x04
#define MPL311_TMP_LSB_REG          0x05

#define MPL3115A2_REGISTER_STATUS               (0x00)
#define MPL3115A2_REGISTER_STATUS_TDR 0x02
#define MPL3115A2_REGISTER_STATUS_PDR 0x04
#define MPL3115A2_REGISTER_STATUS_PTDR 0x08

#define MPL3115A2_REGISTER_PRESSURE_MSB         (0x01)
#define MPL3115A2_REGISTER_PRESSURE_CSB         (0x02)
#define MPL3115A2_REGISTER_PRESSURE_LSB         (0x03)

#define MPL3115A2_REGISTER_TEMP_MSB             (0x04)
#define MPL3115A2_REGISTER_TEMP_LSB             (0x05)

#define MPL3115A2_REGISTER_DR_STATUS            (0x06)

#define MPL3115A2_OUT_P_DELTA_MSB               (0x07)
#define MPL3115A2_OUT_P_DELTA_CSB               (0x08)
#define MPL3115A2_OUT_P_DELTA_LSB               (0x09)

#define MPL3115A2_OUT_T_DELTA_MSB               (0x0A)
#define MPL3115A2_OUT_T_DELTA_LSB               (0x0B)

#define MPL3115A2_WHOAMI                        (0x0C)

#define MPL3115A2_PT_DATA_CFG 0x13
#define MPL3115A2_PT_DATA_CFG_TDEFE 0x01
#define MPL3115A2_PT_DATA_CFG_PDEFE 0x02
#define MPL3115A2_PT_DATA_CFG_DREM 0x04

#define MPL3115A2_CTRL_REG1                     (0x26)
#define MPL3115A2_CTRL_REG1_SBYB 0x01
#define MPL3115A2_CTRL_REG1_OST 0x02
#define MPL3115A2_CTRL_REG1_RST 0x04
#define MPL3115A2_CTRL_REG1_OS1 0x00
#define MPL3115A2_CTRL_REG1_OS2 0x08
#define MPL3115A2_CTRL_REG1_OS4 0x10
#define MPL3115A2_CTRL_REG1_OS8 0x18
#define MPL3115A2_CTRL_REG1_OS16 0x20
#define MPL3115A2_CTRL_REG1_OS32 0x28
#define MPL3115A2_CTRL_REG1_OS64 0x30
#define MPL3115A2_CTRL_REG1_OS128 0x38
#define MPL3115A2_CTRL_REG1_RAW 0x40
#define MPL3115A2_CTRL_REG1_ALT 0x80
#define MPL3115A2_CTRL_REG1_BAR 0x00
#define MPL3115A2_CTRL_REG2                     (0x27)
#define MPL3115A2_CTRL_REG3                     (0x28)
#define MPL3115A2_CTRL_REG4                     (0x29)
#define MPL3115A2_CTRL_REG5                     (0x2A)

#define MPL3115A2_REGISTER_STARTCONVERSION      (0x12)

// LIS2DH Accelerometer
#define LIS2DH_ADDR                 0x19
#define LIS2DH_CTRL_REG1            0x20
#define LIS2DH_CTRL_REG1_VAL        0x57
#define LIS2DH_CTRL_REG4            0x23
#define LIS2DH_CTRL_REG4_VAL        0x10
#define LIS2DH_INT_CFG_REG          0x30
#define LIS2DH_INT_CFG_REG_VAL      0x2A
#define LIS2DH_INT_THS_REG          0x32
#define LIS2DH_INT_THS_REG_VAL      0x3F
#define LIS2DH_INT_COUNTER_REG      0x0E
#define LIS2DH_WHOAMI_REG           0x0F
#define LIS2DH_WHOAMI_REG_VAL       0x33

// Task options
#define DEFAULT_POLL_RATE           5 * 1000

void        SensorsTaskOSInit(void);
void        SensorsTaskHwInit(void);
void        SensorsTask(void);
void        SensorsChangePollingRate(uint32_t ms);
uint32_t    SensorsGetPollingRate(void);
void        SensorsCmd(sensor_cmd_payload_t cmd);

#endif // _SENSORS_H
