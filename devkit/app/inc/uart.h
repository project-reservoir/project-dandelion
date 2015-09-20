#ifndef _UART_H
#define _UART_H

#include "stm32l0xx_hal.h"

#define UART_TIMEOUT_TICKS 500000

typedef enum {
    UART_OK,
    UART_TIMEOUT,
    UART_PARAM_ERROR,
    UART_NOT_READY
} UART_Status;

UART_Status UART_StartRX(USART_TypeDef* uart);
UART_Status UART_StartTX(USART_TypeDef* uart, char* buff, uint8_t size);
void        UART_ContinueTX(USART_TypeDef* uart);
UART_Status UART_ReadyTX(USART_TypeDef* uart);
void        UART_ContinueTX(USART_TypeDef* uart);
void        UART_CharTX(USART_TypeDef* uart, char c);                

#endif // _UART_H
