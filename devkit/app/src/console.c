#include "cmsis_os.h"
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_uart.h"
#include "uart.h"
#include "console.h"
#include "app_header.h"
#include "debug.h"
#include "fw_update.h"
#include "flash.h"
#include <string.h>

UART_HandleTypeDef UartHandle;
osMessageQId uartRxMsgQ;

char rxBuff[CONSOLE_MAX_MSG_SIZE];
char txBuff[CONSOLE_MAX_MSG_SIZE];

uint8_t rxBuffPos = 0;
uint8_t txBuffPos = 0;

uint8_t console_task_started = 0;

static void processString(char* str);
static uint8_t string_len(char* str);
static void processDebugCommand(char* str, uint8_t len);

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
    UartHandle.Init.BaudRate   = 115200;
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
        UART_StartRX(USARTx);
        
        console_task_started = 1;
        
        // Sleep until we receive some data
        msgQueueEvent = osMessageGet(uartRxMsgQ, osWaitForever);
        
        if(msgQueueEvent.status == osEventMessage)
        {
            rxChars = (char*)msgQueueEvent.value.p;
            processString(rxChars);
        }
        
        memset(rxBuff, 0, CONSOLE_MAX_MSG_SIZE);
        rxBuffPos = 0;
    }
}

void processString(char* str)
{
    uint8_t len = string_len(str);
    uint8_t i;
    
    ConsolePrint("\r\n");
    
    switch(str[0])
    {
        case 'u':
            FwUpdateStart();
            for(i = 0; i < FLASH_PAGE_SIZE; i++)
            {
                FwUpdateWriteWord(0xDEADBEEF);
            }
            FwUpdateEnd();
            break;
        case 'd':
            processDebugCommand(str, len);
            break;
        case 'v':
            ConsolePrint("DANDELION OS V ");
            ConsolePrint(APP_VERSION_STR);
            ConsolePrint("\r\n");
            ConsolePrint("BUILD DATE: ");
            ConsolePrint(__DATE__);
            ConsolePrint("  :  ");
            ConsolePrint(__TIME__);
            ConsolePrint("\r\n");
            ConsolePrint("RUN REGION: ");
            if(FwUpdateGetCurrentRegion() == MAIN_APP_START)
            {
                ConsolePrint("MAIN APP");
            }
            else
            {
                ConsolePrint("BACKUP APP");
            }
            ConsolePrint("\r\n\r\n");
            
            break;
        default:
            ConsolePrint("h : print help\r\n");
            ConsolePrint("v : print version info\r\n");
            ConsolePrint("d : debug information\r\n");
            ConsolePrint("u : perform a fake firmware upgrade using 0xDEADBEEF as the payload\r\n");
            break;
    }
    
    ConsolePrint("> ");
}

void processDebugCommand(char* str, uint8_t len)
{
    if(len >= 2)
    {
        if(str[1] == 'd')
        {
            ToggleDebug();
            return;
        }
        else if(str[1] == 'i')
        {
            ToggleInfo();
            return;
        }
        else if(str[1] == 'w')
        {
            ToggleWarn();
            return;
        }
        else if(str[1] == 'e')
        {
            ToggleError();
            return;
        }
    }
    
    ConsolePrint("Debug commands\r\n");
    ConsolePrint("dd : toggle debug messages\r\n");
    ConsolePrint("di : toggle info messages\r\n");
    ConsolePrint("dw : toggle warn messages\r\n");
    ConsolePrint("de : toggle error messages\r\n");
}

uint8_t string_len(char* str)
{
    uint8_t i = 0;
    
    while(str[i] != '\0' && i < CONSOLE_MAX_MSG_SIZE)
    {
        i++;
    }
    
    return i;
}

void ConsolePrint(char* text)
{
    uint8_t i = string_len(text);
    while(!console_task_started)
    {
        osDelay(10);
    }
    
    // While the UART is not ready for TX, spin
    while(UART_ReadyTX(UartHandle.Instance) != UART_OK)
    {
        osDelay(10);
    }
    
    UART_StartTX(UartHandle.Instance, text, i);
}

void ConsoleGetChar(char c)
{
    // Silently drop chars if buffer is full: OS is dealing with the message
    if(rxBuffPos >= CONSOLE_MAX_MSG_SIZE)
    {
        return;
    }
    
    switch(c)
    {
        case '\r':
        case '\n':
            osMessagePut(uartRxMsgQ, (uint32_t)rxBuff, osWaitForever);
            break;
        
        default:
            rxBuff[rxBuffPos++] = c;
    }
    
    if(rxBuffPos >= CONSOLE_MAX_MSG_SIZE)
    {
        osMessagePut(uartRxMsgQ, (uint32_t)rxBuff, osWaitForever);
    }
    
    UART_CharTX(UartHandle.Instance, c);
}
