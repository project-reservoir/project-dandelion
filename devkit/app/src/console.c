#include "cmsis_os.h"
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_uart.h"
#include "uart.h"
#include "console.h"
#include "app_header.h"
#include "debug.h"
#include "fw_update.h"
#include "flash.h"
#include "radio.h"
#include "radio_packets.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "power.h"
#include "xprintf.h"
#include "capacitance.h"

// Minimum message size includes the ':' starting character and the \n
#define MIN_INTEL_MESSAGE_LEN   12
#define MAX_INTEL_PAYLOAD       0x10

UART_HandleTypeDef UartHandle;
osMessageQId uartRxMsgQ;

char rxBuff[CONSOLE_MAX_MSG_SIZE];
char txBuff[CONSOLE_MAX_MSG_SIZE];

uint8_t rxBuffPos = 0;
uint8_t txBuffPos = 0;

uint8_t console_task_started = 0;

uint16_t extended_address = 0;
uint16_t current_intel_address = 0;
uint8_t intel_hex_payload[MAX_INTEL_PAYLOAD];

static void processString(char* str);
static uint8_t string_len(char* str);
static void processDebugCommand(char* str, uint8_t len);
static void processRadioCommand(char* str, uint8_t len);
static void processSensorCommand(char* str, uint8_t len);

void consoleTxChar(unsigned char c)
{
    UART_CharTX(UartHandle.Instance, c);
}

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
    
    xdev_out(consoleTxChar);
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
    
    // TODO: check len > 0
    
    xprintf("\r\n");
    
    switch(str[0])
    {
        case 'r':
            if(len >= 2)
            {
                switch(str[1])
                {
                    case 'r':
                        NVIC_SystemReset();
                        break;
                    
                    case 'f':
                        __BKPT(0);
                        break;
                }
            }
            else
            {
                xprintf("Reset commands\n");
                xprintf("rr: reset the microprocessor completely\n");
                xprintf("rf: reset the microprocessor by causing a HardFault\n");
            }
            break;
        case 'u':
            FwUpdateStart();
            for(i = 0; i < FLASH_PAGE_SIZE; i += 4)
            {
                FwUpdateWriteWord(0xDEADBEEF, i);
            }
            FwUpdateEnd();
            break;
        case 'd':
            processDebugCommand(str, len);
            break;
        case 'x':
            processRadioCommand(str, len);
            break;
        case 'v':
            xprintf("DANDELION OS V ");
            xprintf(APP_VERSION_STR);
            xprintf("\r\n");
            xprintf("BUILD DATE: ");
            xprintf(__DATE__);
            xprintf("  :  ");
            xprintf(__TIME__);
            xprintf("\r\n");
            xprintf("RUN REGION: ");
            if(FwUpdateGetCurrentRegion() == MAIN_APP_START)
            {
                xprintf("MAIN APP");
            }
            else
            {
                xprintf("BACKUP APP");
            }
            xprintf("\r\n\r\n");
            break;
        case 's':    
            processSensorCommand(str, len);
            break;
        default:
            xprintf("h : print help\r\n");
            xprintf("v : print version info\r\n");
            xprintf("d : debug information\r\n");
            xprintf("x : radio commands\r\n");
            xprintf("u : perform a fake firmware upgrade\r\n");
            xprintf("r : reset commands\r\n");
            xprintf("s : sensor commands\r\n");
            break;
    }
    
    xprintf("> ");
}

void processSensorCommand(char* str, uint8_t len)
{
    generic_message_t* generic_msg;
    
    if(len >= 2)
    {
        switch(str[1])
        {
            case 't':
                xprintf("Temperature info:\n");
                xprintf("Chip temp: %d c\n", GetChipTemperature());
                return;
            
            case 'b':
                xprintf("Battery voltage: %d mV\n", GetBatteryVoltage());
                return;
            
            case 'c':
            {
                uint32_t cap1, cap2, cap3;
                CapacitanceRead(&cap1, &cap2, &cap3);
                xprintf("Capsense values: %d, %d, %d\n", cap1, cap2, cap3);
                return;
            }
        }
    }
    
    xprintf("Sensor Commands\r\n");
    xprintf("st : print temperature data\r\n");
    xprintf("sb : print battery data\r\n");
    xprintf("sc : acquire and print capsense counts\r\n");
}

void processRadioCommand(char* str, uint8_t len)
{
    generic_message_t* generic_msg;
    
    if(len >= 2)
    {
        switch(str[1])
        {
            case 'p':
            generic_msg = pvPortMalloc(sizeof(generic_message_t));
        
            // TODO: check we didn't run out of RAM (we should catch this in the 
            //       application Malloc failed handler, but just in case)
        
            generic_msg->cmd = PING;
        
            SendToBroadcast((uint8_t*)generic_msg, sizeof(generic_message_t));
            return;
        }
    }
    
    xprintf("Radio Commands\r\n");
    xprintf("xp : send a radio ping packet\r\n");
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
    
    xprintf("Debug commands\r\n");
    xprintf("dd : toggle debug messages\r\n");
    xprintf("di : toggle info messages\r\n");
    xprintf("dw : toggle warn messages\r\n");
    xprintf("de : toggle error messages\r\n");
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
