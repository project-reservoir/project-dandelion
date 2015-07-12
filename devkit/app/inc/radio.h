#ifndef _RADIO_H
#define _RADIO_H

#include "stm32l0xx_hal.h"

#define SPIx                             SPI2
#define SPIx_CLK_ENABLE()                __SPI2_CLK_ENABLE()
#define SPIx_SCK_GPIO_CLK_ENABLE()       __GPIOB_CLK_ENABLE()
#define SPIx_MISO_GPIO_CLK_ENABLE()      __GPIOB_CLK_ENABLE() 
#define SPIx_MOSI_GPIO_CLK_ENABLE()      __GPIOB_CLK_ENABLE() 

#define SPIx_FORCE_RESET()               __SPI2_FORCE_RESET()
#define SPIx_RELEASE_RESET()             __SPI2_RELEASE_RESET()

/* Definition for SPIx Pins */
#define SPIx_NSS_PIN                     GPIO_PIN_12
#define SPIx_NSS_GPIO_PORT               GPIOB
#define SPIx_NSS_AF                      GPIO_AF0_SPI2

#define SPIx_SCK_PIN                     GPIO_PIN_13
#define SPIx_SCK_GPIO_PORT               GPIOB
#define SPIx_SCK_AF                      GPIO_AF0_SPI2

#define SPIx_MISO_PIN                    GPIO_PIN_14
#define SPIx_MISO_GPIO_PORT              GPIOB
#define SPIx_MISO_AF                     GPIO_AF0_SPI2

#define SPIx_MOSI_PIN                    GPIO_PIN_15
#define SPIx_MOSI_GPIO_PORT              GPIOB
#define SPIx_MOSI_AF                     GPIO_AF0_SPI2

/* Definition for SPIx's NVIC */
#define SPIx_IRQn                        SPI2_IRQn

#define RADIO_NIRQ_PIN                   GPIO_PIN_0
#define RADIO_NIRQ_GPIO_PORT             GPIOB
#define NIRQ_IRQn                        EXTI0_1_IRQn

#define RADIO_SDL_PIN                    GPIO_PIN_2
#define RADIO_SDL_GPIO_PORT              GPIOB

#define RADIO_GP1_PIN                    GPIO_PIN_10
#define RADIO_GP1_GPIO_PORT              GPIOB

/* Size of buffer */
#define BUFFSIZE                         255

#define RADIO_MSG_QUEUE_SIZE             8

// OS Task related functions
void RadioTaskHwInit(void);
void RadioTaskOSInit(void);
void RadioTask(void);

// Public Radio API
void SendToBaseStation(uint8_t* data, uint8_t size);
void SentToBroadcast(uint8_t* data, uint8_t size);

#endif // _RADIO_H
