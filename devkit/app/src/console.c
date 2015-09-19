#include "cmsis_os.h"
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_uart.h"
#include "console.h"

UART_HandleTypeDef UartHandle;
osMessageQId uartRxMsgQ;

char rxBuff[CONSOLE_MAX_MSG_SIZE];
char txBuff[CONSOLE_MAX_MSG_SIZE];

void processString(const char* str);
uint8_t string_len(const char* str);

void ConsoleTaskHwInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    USARTx_TX_GPIO_CLK_ENABLE();
    USARTx_RX_GPIO_CLK_ENABLE();
    /* Enable USART2 clock */
    USARTx_CLK_ENABLE(); 

    /*##-2- Configure peripheral GPIO ##########################################*/  
    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = USARTx_TX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = USARTx_TX_AF;

    HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

    /* UART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = USARTx_RX_PIN;
    GPIO_InitStruct.Alternate = USARTx_RX_AF;

    HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

    /*##-3- Configure the NVIC for UART ########################################*/
    /* NVIC for USART1 */
    HAL_NVIC_SetPriority(USARTx_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(USARTx_IRQn);
}

void ConsoleTaskOSInit(void)
{
    osMessageQDef(UARTRxMsgQueue, CONSOLE_MSG_Q_SIZE, char*);
    uartRxMsgQ = osMessageCreate(osMessageQ(UARTRxMsgQueue), NULL);
    
    UartHandle.Instance        = USARTx;
    UartHandle.Init.BaudRate   = 9600;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode       = UART_MODE_TX_RX;
    
    assert_param(HAL_UART_Init(&UartHandle) == HAL_OK);
}

void ConsoleTask(void)
{
    osEvent msgQueueEvent;
    char* rxChars = NULL;
    
    while(1)
    {
        // Put UART into receive mode
        assert_param(HAL_UART_Receive_IT(&UartHandle, (uint8_t *)rxBuff, CONSOLE_MAX_MSG_SIZE) == HAL_OK);
        
        // Sleep until we receive some data
        msgQueueEvent = osMessageGet(uartRxMsgQ, osWaitForever);
        
        if(msgQueueEvent.status == osEventMessage)
        {
            rxChars = (char*)msgQueueEvent.value.p;
            processString(rxChars);
        }
    }
}

void processString(const char* str)
{
    uint8_t len = string_len(str);
    
    switch(str[0])
    {
        case 'h':
            ConsolePrint("h : print help\r\n");
        default:
            ConsolePrint("> ");
            break;
    }
}

uint8_t string_len(const char* str)
{
    uint8_t i = 0;
    
    while(str[i] != '\0' && i < CONSOLE_MAX_MSG_SIZE)
    {
        i++;
    }
    
    return i;
}

void ConsolePrint(const char* text)
{
    uint8_t i = string_len(text);
    
    // While the UART is not ready for TX, spin
    while(!((HAL_UART_GetState(&UartHandle) == HAL_UART_STATE_READY) || (HAL_UART_GetState(&UartHandle) == HAL_UART_STATE_BUSY_RX)))
    {
        osDelay(10);
    }
    
    assert_param(HAL_UART_Transmit_IT(&UartHandle, (uint8_t*)text, i) == HAL_OK);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uh)
{
    osMessagePut(uartRxMsgQ, (uint32_t)uh->pRxBuffPtr, osWaitForever);
}
