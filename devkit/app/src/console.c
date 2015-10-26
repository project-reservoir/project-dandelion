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
static void ParseIntelHex(char* str, uint8_t len);
static uint8_t HexToNibble(char ch);

// Convert 1 hex char into a Nibble. Fills the lower 4 bits of the byte
static uint8_t HexToNibble(char ch)
{
    uint8_t nib = 0;
    
    switch(ch)
    {
        case '0':
            nib = 0u;
            break;
        
        case '1':
            nib = 1u;
            break;
        
        case '2':
            nib = 2u;
            break;
        
        case '3':
            nib = 3u;
            break;
        
        case '4':
            nib = 4u;
            break;
        
        case '5':
            nib = 5u;
            break;
        
        case '6':
            nib = 6u;
            break;
        
        case '7':
            nib = 7u;
            break;
        
        case '8':
            nib = 8u;
            break;
        
        case '9':
            nib = 9u;
            break;
        case 'A':
        case 'a':
            nib = 10u;
            break;
        
        case 'B':
        case 'b':
            nib = 11u;
            break;
        
        case 'C':
        case 'c':
            nib = 12u;
            break;
        
        case 'D':
        case 'd':
            nib = 13u;
            break;
        
        case 'E':
        case 'e':
            nib = 14u;
            break;
        
        case 'F':
        case 'f':
            nib = 15u;
            break;
        
        default:
            DEBUG("Invalid HEX char in HexToNib\r\n");
            break;
    }
    
    return nib;
}

static void ParseIntelHex(char* str, uint8_t len)
{
    uint8_t i = 0;
    uint8_t command = 0;
    uint8_t byte_count = 0;
    uint16_t address = 0;
    uint8_t checksum = 0;
    uint8_t sent_check = 0;
    
    if(len < MIN_INTEL_MESSAGE_LEN)
    {
        WARN("Malformed Intel Hex Msg\r\n");
        return;
    }
    
    if(str[0] != ':')
    {
        WARN("Message is not Intel Hex Msg\r\n");
        return;
    }
    
    // Form the byte count
    byte_count = (HexToNibble(str[1]) << 4) | HexToNibble(str[2]);
    checksum = byte_count;
    
    // Form the address
    address = (HexToNibble(str[3]) << 12) | (HexToNibble(str[4]) << 8) | (HexToNibble(str[5]) << 4) | HexToNibble(str[6]);
    checksum += ((HexToNibble(str[3]) << 4) | HexToNibble(str[4]));
    checksum += ((HexToNibble(str[5]) << 4) | HexToNibble(str[6]));
    
    // Form the command
    command = (HexToNibble(str[7]) << 4) | HexToNibble(str[8]);
    checksum += command;
    
    for(i = 0; i < byte_count && i < MAX_INTEL_PAYLOAD; i++)
    {
        intel_hex_payload[i] = (HexToNibble(str[9 + (i*2)]) << 4) | HexToNibble(str[10 + (i*2)]);
        checksum += intel_hex_payload[i];
    }
    
    checksum = ~checksum;
    checksum += 0x01;
    
    sent_check = (uint32_t)((HexToNibble(str[(byte_count * 2) + 9]) << 4) | HexToNibble(str[(byte_count * 2) + 10]));
    
    if(checksum != sent_check)
    {
        ERR("Intel Hex Checksum fail\r\n");
        return;
    }
    
    switch(command)
    {
        case 0:
            if((byte_count % 4) != 0)
            {
                WARN("Intel HEX Cmd 0 err: byte count !mod 4\r\n");
                break;
            }
            
            // Allow the first write to be to any address, initialize for future writes
            if(current_intel_address == 0)
            {
                current_intel_address = address;
            }
            else if(current_intel_address != address)
            {
                ERR("Addresses are not sequential\r\n");
                return;
            }
            
            current_intel_address += byte_count;
            
            for(i = 0; i < byte_count; i += 4)
            {
                FwUpdateWriteWord((intel_hex_payload[i + 3] << 24) | (intel_hex_payload[i + 2] << 16) | (intel_hex_payload[i + 1] << 8) | (intel_hex_payload[i]));
            }
            break;
        
        case 1:
            if(byte_count != 0)
            {
                WARN("Intel HEX Cmd 1 err: byte count != 0\r\n");
                break;
            }
            extended_address = 0;
            FwUpdateEnd();
            break;
        
        case 4:
            if(byte_count != 2)
            {
                WARN("Intel HEX Cmd 4 err: byte count != 2\r\n");
                break;
            }
            extended_address = (intel_hex_payload[0] << 8) | intel_hex_payload[1];
            FwUpdateStart();
            break;
        
        case 5:
            if(byte_count != 4)
            {
                WARN("Intel HEX Cmd 5 err: byte count != 4\r\n");
                break;
            }
            break;
        
        default:
            WARN("Unknown Intel HEX cmd\r\n");
            break;
    }
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
    
    ConsolePrint("\r\n");
    
    switch(str[0])
    {
        case ':':
            ParseIntelHex(str, len);
            break;
        case 'r':
            if(len >= 2)
            {
                if(str[1] == 'r')
                {
                    NVIC_SystemReset();
                }
            }
            else
            {
                ConsolePrint("Reset commands\r\n");
                ConsolePrint("rr: reset the microprocessor completely\r\n");
            }
            break;
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
        case 'x':
            processRadioCommand(str, len);
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
            ConsolePrint("x : radio commands\r\n");
            ConsolePrint("u : perform a fake firmware upgrade\r\n");
            ConsolePrint("r : reset commands\r\n");
            break;
    }
    
    ConsolePrint("> ");
}

void processRadioCommand(char* str, uint8_t len)
{
    radio_message_t* generic_msg;
    
    if(len >= 2)
    {
        switch(str[1])
        {
            case 'p':
            generic_msg = pvPortMalloc(sizeof(radio_message_t));
        
            // TODO: check we didn't run out of RAM (we should catch this in the 
            //       application Malloc failed handler, but just in case)
        
            generic_msg->ping.cmd = PING;
        
            SendToBroadcast((uint8_t*)generic_msg, sizeof(radio_message_t));
            return;
        }
    }
    
    ConsolePrint("Radio Commands\r\n");
    ConsolePrint("xp : send a radio ping packet\r\n");
}

void processSensorCommand(char* str, uint8_t len)
{
    radio_message_t* generic_msg;
    uint32_t ms = 0;
    
    if(len >= 2)
    {
        switch(str[1])
        {
            case 'r':
            if(str[2] == ' ')
            {
                ms = atoi(&str[3]);
                return;
            }
        }
    }
    
    ConsolePrint("Sensor Commands\r\n");
    ConsolePrint("sr <val> : set polling rate in miliseconds\r\n");
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
