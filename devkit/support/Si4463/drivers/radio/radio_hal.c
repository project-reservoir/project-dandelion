/*!
 * File:
 *  radio_hal.c
 *
 * Description:
 *  This file contains RADIO HAL.
 *
 * Silicon Laboratories Confidential
 * Copyright 2011 Silicon Laboratories, Inc.
 */

                /* ======================================= *
                 *              I N C L U D E              *
                 * ======================================= */

#include "..\..\bsp.h"
#include "radio.h"

                /* ======================================= *
                 *          D E F I N I T I O N S          *
                 * ======================================= */

                /* ======================================= *
                 *     G L O B A L   V A R I A B L E S     *
                 * ======================================= */
extern SPI_HandleTypeDef SpiHandle;
                /* ======================================= *
                 *      L O C A L   F U N C T I O N S      *
                 * ======================================= */

                /* ======================================= *
                 *     P U B L I C   F U N C T I O N S     *
                 * ======================================= */

void radio_hal_AssertShutdown(void)
{
    HAL_GPIO_WritePin(RADIO_SDL_GPIO_PORT, RADIO_SDL_PIN, GPIO_PIN_SET);
}

void radio_hal_DeassertShutdown(void)
{
	HAL_GPIO_WritePin(RADIO_SDL_GPIO_PORT, RADIO_SDL_PIN, GPIO_PIN_RESET);
}

void radio_hal_ClearNsel(void)
{
	// TODO: select the chip using the NSEL pin on the SPI hardware
    //RF_NSEL = 0;
}

void radio_hal_SetNsel(void)
{
	// TODO: de-select the chip using the NSEL pin on the SPI hardware
    //RF_NSEL = 1;
}

BIT radio_hal_NirqLevel(void)
{
	//TODO: return the value of the pin connected to the radio NIRQ pin
    return HAL_GPIO_ReadPin(RADIO_NIRQ_GPIO_PORT, RADIO_NIRQ_PIN);
}

void radio_hal_SpiWriteByte(U8 byteToWrite)
{
    uint8_t byte_buff[1];
    
    byte_buff[0] = byteToWrite;
    
    HAL_SPI_Transmit(&SpiHandle, byte_buff, 1, 1000);
}

U8 radio_hal_SpiReadByte(void)
{
    uint8_t byte[1];
    uint8_t dummy_byte[1];
    dummy_byte[0] = 0xFF;
    
    HAL_SPI_TransmitReceive(&SpiHandle, dummy_byte, byte, 1, 1000);
    return byte[0];
}

void radio_hal_SpiWriteData(U8 byteCount, U8* pData)
{
    HAL_SPI_Transmit(&SpiHandle, pData, byteCount, 1000);
}

void radio_hal_SpiReadData(U8 byteCount, U8* pData)
{
    HAL_SPI_Receive(&SpiHandle, pData, byteCount, 1000);
}

#ifdef RADIO_DRIVER_EXTENDED_SUPPORT
BIT radio_hal_Gpio0Level(void)
{
  BIT retVal = FALSE;

#ifdef SILABS_PLATFORM_DKMB
  retVal = FALSE;
#endif
#ifdef SILABS_PLATFORM_UDP
  retVal = EZRP_RX_DOUT;
#endif
#if (defined SILABS_PLATFORM_RFSTICK) || (defined SILABS_PLATFORM_LCDBB)
  retVal = RF_GPIO0;
#endif
#if (defined SILABS_PLATFORM_WMB930)
  retVal = FALSE;
#endif
#if defined (SILABS_PLATFORM_WMB912)
  #ifdef SILABS_IO_WITH_EXTENDER
    //TODO
    retVal = FALSE;
  #endif
#endif

  return retVal;
}

BIT radio_hal_Gpio1Level(void)
{
  BIT retVal = FALSE;

#ifdef SILABS_PLATFORM_DKMB
  retVal = FSK_CLK_OUT;
#endif
#ifdef SILABS_PLATFORM_UDP
  retVal = FALSE; //No Pop
#endif
#if (defined SILABS_PLATFORM_RFSTICK) || (defined SILABS_PLATFORM_LCDBB) || (defined SILABS_PLATFORM_WMB930)
  retVal = RF_GPIO1;
#endif
#if defined (SILABS_PLATFORM_WMB912)
  #ifdef SILABS_IO_WITH_EXTENDER
    //TODO
    retVal = FALSE;
  #endif
#endif

  return retVal;
}

BIT radio_hal_Gpio2Level(void)
{
  BIT retVal = FALSE;

#ifdef SILABS_PLATFORM_DKMB
  retVal = DATA_NFFS;
#endif
#ifdef SILABS_PLATFORM_UDP
  retVal = FALSE; //No Pop
#endif
#if (defined SILABS_PLATFORM_RFSTICK) || (defined SILABS_PLATFORM_LCDBB) || (defined SILABS_PLATFORM_WMB930)
  retVal = RF_GPIO2;
#endif
#if defined (SILABS_PLATFORM_WMB912)
  #ifdef SILABS_IO_WITH_EXTENDER
    //TODO
    retVal = FALSE;
  #endif
#endif

  return retVal;
}

BIT radio_hal_Gpio3Level(void)
{
  BIT retVal = FALSE;

#if (defined SILABS_PLATFORM_RFSTICK) || (defined SILABS_PLATFORM_LCDBB) || (defined SILABS_PLATFORM_WMB930)
  retVal = RF_GPIO3;
#elif defined (SILABS_PLATFORM_WMB912)
  #ifdef SILABS_IO_WITH_EXTENDER
    //TODO
    retVal = FALSE;
  #endif
#endif

  return retVal;
}

#endif
