#ifndef __HWCTRL_H
#define __HWCTRL_H

#include "stm32l0xx_hal.h"

// DEFINES
#define LED3_PIN                                GPIO_PIN_4
#define LED3_GPIO_PORT                          GPIOB
#define LED3_GPIO_CLK_ENABLE()                  __GPIOB_CLK_ENABLE()
#define LED3_GPIO_CLK_DISABLE()                 __GPIOB_CLK_DISABLE()

#define LED4_PIN                                GPIO_PIN_5
#define LED4_GPIO_PORT                          GPIOA
#define LED4_GPIO_CLK_ENABLE()                  __GPIOA_CLK_ENABLE()
#define LED4_GPIO_CLK_DISABLE()                 __GPIOA_CLK_DISABLE()

/* User can use this section to tailor I2Cx/I2Cx instance used and associated 
   resources */
/* Definition for I2Cx clock resources */
#define I2Cx                            I2C1
#define I2Cx_CLK_ENABLE()               __I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __GPIOB_CLK_ENABLE() 

#define I2Cx_FORCE_RESET()              __I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()            __I2C1_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_8
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SCL_AF                     GPIO_AF4_I2C1
#define I2Cx_SDA_PIN                    GPIO_PIN_9
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SDA_AF                     GPIO_AF4_I2C1

/* Definition for I2Cx's NVIC */
#define I2Cx_IRQn                       I2C1_IRQn
#define I2Cx_IRQHandler                 I2C1_Handler

#define I2C_TIMING_100KHZ               0x10A13E56 /* Analog Filter ON, Rise Time 400ns, Fall Time 100ns */ 
#define I2C_TIMING_400KHZ               0x00B1112E /* Analog Filter ON, Rise Time 250ns, Fall Time 100ns */   

#define I2C_BUFFER_SIZE                 2

#define I2C_ADDRESS                     0x30F

// GLOBAL FUNCTIONS
void SystemClock_Config(void);
void HwCtrl_Init(void);
void HwCtrl_Led_Control(void);

#endif  // __HWCTRL_H
