#include "radio.h"
#include "radio_config.h"
#include "cmsis_os.h"
#include "si446x_api_lib.h"
#include "si446x_cmd.h"
#include "spi.h"
#include "radio_packets.h"
#include "fw_update.h"

extern void si446x_fifo_info(uint8_t FIFO);
extern void si446x_read_rx_fifo( uint8_t numBytes, uint8_t* pRxData );
extern void si446x_write_tx_fifo( uint8_t numBytes, uint8_t* pData );

extern unsigned portBASE_TYPE makeFreeRtosPriority(osPriority priority);

// Global variables
osMessageQId radioTxMsgQ;
osMessageQId radioRxMsgQ;
osMessageQId radioWakeupMsgQ;

SPI_HandleTypeDef SpiHandle;

uint8_t Radio_Configuration_Data_Array[] = RADIO_CONFIGURATION_DATA_ARRAY;
tRadioConfiguration RadioConfiguration = RADIO_CONFIGURATION_DATA;
tRadioConfiguration* pRadioConfiguration = &RadioConfiguration;

uint8_t customRadioPacket[RADIO_MAX_PACKET_LENGTH];

// Local variables 
//static uint8_t txBuff[BUFFSIZE];
static uint8_t rxBuff[BUFFSIZE];

static RadioTaskState radioTaskState = SEARCHING;
static NetworkInfo    network;
static bool           inhibitTx      = false;

// Local function prototypes
static void RadioLinkManagementTask(void);
static uint8_t SendRadioConfig(void);
static void    Radio_StartTx_Variable_Packet(uint8_t channel, uint8_t *pioRadioPacket, uint8_t length);
static void    SignalRadioTXNeeded(void);
static void    Radio_StartRX(uint8_t channel);

// Global function implementations
void RadioTaskOSInit(void)
{
    osMessageQDef(RadioTxMsgQueue, RADIO_MSG_QUEUE_SIZE, RadioMessage*);
    osMessageQDef(RadioRxMsgQueue, RADIO_MSG_QUEUE_SIZE, RadioMessage*);
    osMessageQDef(RadioWakeupMsgQueue, RADIO_MSG_QUEUE_SIZE, RadioTaskWakeupReason);
    
    radioTxMsgQ = osMessageCreate(osMessageQ(RadioTxMsgQueue), NULL);
    radioRxMsgQ = osMessageCreate(osMessageQ(RadioRxMsgQueue), NULL);
    radioWakeupMsgQ = osMessageCreate(osMessageQ(RadioWakeupMsgQueue), NULL);
    
    assert_param(radioTxMsgQ != NULL);
    assert_param(radioRxMsgQ != NULL);
    assert_param(radioWakeupMsgQ != NULL);
    
    SpiHandle.Instance               = SPIx;
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
    SpiHandle.Init.CRCPolynomial     = 7;
    SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.NSS               = SPI_NSS_SOFT;
    SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
    SpiHandle.Init.Mode              = SPI_MODE_MASTER;
    
    assert_param(HAL_SPI_Init(&SpiHandle) == HAL_OK);
    
    SPI2->CR1 |= SPI_CR1_SPE;
}

void RadioTaskHwInit(void)
{   
    GPIO_InitTypeDef  GPIO_InitStruct;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    SPIx_SCK_GPIO_CLK_ENABLE();
    SPIx_MISO_GPIO_CLK_ENABLE();
    SPIx_MOSI_GPIO_CLK_ENABLE();
    /* Enable SPI clock */
    SPIx_CLK_ENABLE(); 

    /*##-2- Configure peripheral GPIO ##########################################*/  
    /* SPI SCK GPIO pin configuration  */
    GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = SPIx_SCK_AF;

    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MISO GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MISO_PIN;
    GPIO_InitStruct.Alternate = SPIx_MISO_AF;

    HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MOSI GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
    GPIO_InitStruct.Alternate = SPIx_MOSI_AF;

    HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin        = SPIx_NSS_PIN;
    GPIO_InitStruct.Pull       = GPIO_PULLDOWN;
    GPIO_InitStruct.Mode       = GPIO_MODE_OUTPUT_PP;   
    GPIO_InitStruct.Speed      = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate  = 0x00;

    HAL_GPIO_Init(SPIx_NSS_GPIO_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin        = RADIO_SDL_PIN;
    GPIO_InitStruct.Pull       = GPIO_PULLDOWN;
    GPIO_InitStruct.Mode       = GPIO_MODE_OUTPUT_PP;   
    GPIO_InitStruct.Speed      = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate  = 0x00;
    
    HAL_GPIO_Init(RADIO_SDL_GPIO_PORT, &GPIO_InitStruct);
    
    // Configure Radio NIRQ
    GPIO_InitStruct.Pin        = RADIO_NIRQ_PIN;
    GPIO_InitStruct.Pull       = GPIO_NOPULL;
    GPIO_InitStruct.Mode       = GPIO_MODE_IT_FALLING;   
    GPIO_InitStruct.Speed      = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate  = 0x00;
    
    HAL_GPIO_Init(RADIO_NIRQ_GPIO_PORT, &GPIO_InitStruct);
    
    // Configure Radio GP1 pin
    GPIO_InitStruct.Pin        = RADIO_GP1_PIN;
    GPIO_InitStruct.Pull       = GPIO_NOPULL;
    GPIO_InitStruct.Mode       = GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate  = 0x00;
    
    HAL_GPIO_Init(RADIO_GP1_GPIO_PORT, &GPIO_InitStruct);
    
    // Initially de-select SPI devices
    HAL_GPIO_WritePin(SPIx_NSS_GPIO_PORT, SPIx_NSS_PIN, GPIO_PIN_SET);
}

void RadioTask(void)
{
    uint8_t radioConfigured = 0;
    osEvent msgQueueEvent;
    RadioMessage* msg;
    
    generic_message_t* generic_msg;
    
    // Configure radio
    while(!radioConfigured)
    {
        if(SendRadioConfig() == SI446X_SUCCESS)
        {
            // Verify the radio actually came up:
            si446x_part_info();
            
            #ifdef DEVKIT
            if(Si446xCmd.PART_INFO.PART != 0x4463)
            #elif DANDELION
            if(Si446xCmd.PART_INFO.PART != 0x4468)
            #else
            #error "Please select a device type"
            #endif
            {
                ERR("Radio did not return correct part number!\n");
            }
            
            radioConfigured = 1;
        }
        else
        {
            // TODO: we should delay for a while before trying to reprogram the radio.
            // TODO: consider entering an ultra-low power mode if radio config fails?
            //       We can't send sensor readings without the radio, so performing
            //       sensor polling is probably a waste of power
            __BKPT(0);
        }
    }
    
    // Now that the radio has been configured, enable radio interrupts
    HAL_NVIC_SetPriority(NIRQ_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(NIRQ_IRQn);
    
    xTaskCreate(RadioLinkManagementTask,
                "RadioLinkManagementTask",
                configMINIMAL_STACK_SIZE,
                NULL,
                makeFreeRtosPriority(osPriorityLow),
                NULL);
    
    while(1)
    {               
        // Pend on the message queue that will wakeup the radio task. 
        // This can come from a TX event or an IRQ event
        msgQueueEvent = osMessageGet(radioWakeupMsgQ, osWaitForever);
        
        if(msgQueueEvent.status == osEventMessage)
        {
            if(msgQueueEvent.value.v == RADIO_IRQ_DETECTED)
            {
                RadioTaskHandleIRQ();
            }
            else if(msgQueueEvent.value.v == RADIO_TX_NEEDED)
            {
                // Wait on the transmit queue
                // TODO: this should timeout, since it should have a message in it already if we got this far
                msgQueueEvent = osMessageGet(radioTxMsgQ, osWaitForever);
                
                if(msgQueueEvent.status == osEventMessage)
                {
                    msg = (RadioMessage*)(msgQueueEvent.value.p);
                    ((generic_message_t*)(msg->pData))->dst = msg->dest;
                    ((generic_message_t*)(msg->pData))->src = RadioGetMACAddress();
                    
                    // Transmit the packet to the radio hardware
                    Radio_StartTx_Variable_Packet(pRadioConfiguration->Radio_ChannelNumber, msg->pData, msg->size);
                    
                    // Free the data
                    vPortFree(msg->pData);
                    vPortFree(msg);
                }
            }
        }
        else
        {
            // Message wasn't received OK? Just wait for another one... something went wrong with the OS.
            continue;
        }
    }
}

void RadioLinkManagementTask(void)
{   
    generic_message_t* generic_msg;
    
    while(1)
    {               
        switch(radioTaskState)
        {
            // Try and join the network. If success, enter CONNECTED state, else return to LOOKING_FOR_BASE_STATION
            case CONNECTING:
                DEBUG("Connecting to base station...\r\n");
                osDelay(10000);
                break;
            
            // Perform regular radio duties. If we loose connection, enter the CONNECTING_TO_BASE_STATION state
            case CONNECTED:
                // TODO: occasionally send PING packets to the host: if we don't get a reply, go to the SEARCHING state
                osDelay(10000);
                break;
            
            // Send ANNOUNCE packets every 20 seconds and see if we get a reply
            case SEARCHING:
                DEBUG("Sending ANNOUNCE packet...\r\n");
                generic_msg = pvPortMalloc(sizeof(generic_message_t));
                
                // TODO: check we didn't run out of RAM (we should catch this in the 
                //       application Malloc failed handler, but just in case)
            
                generic_msg->cmd = ANNOUNCE;
                generic_msg->dst = 0xFFFFFFFF;
                generic_msg->src = RadioGetMACAddress();
                
                SendToBroadcast((uint8_t*)generic_msg, sizeof(generic_message_t));
                
                osDelay(10000);
                break;
        }
    }
}

// The pointer passed into this function should have been allocated using 
// the osAlloc() routine: it will be freed using the osFree routine
void SendToDevice(uint8_t* data, uint8_t size, uint32_t mac)
{
    if(!inhibitTx)
    {
        RadioMessage* message = pvPortMalloc(sizeof(RadioMessage));
        message->pData = data;
        message->size = size;
        message->dest = mac;

        // TODO: Check that this actually succeeded, and don't block forever.
        //       Might need to return a status to indicate to calling function that we failed to send message
        osMessagePut(radioTxMsgQ, (uint32_t)message, osWaitForever);

        SignalRadioTXNeeded();
    }
    else
    {
        vPortFree(data);
    }
}

// The pointer passed into this function should have been allocated using 
// the osAlloc() routine: it will be freed using the osFree routine
void SendToBaseStation(uint8_t* data, uint8_t size)
{
    if(!inhibitTx)
    {
        RadioMessage* message = pvPortMalloc(sizeof(RadioMessage));
        message->pData = data;
        message->size = size;
        message->dest = network.baseStationMac; // TODO: what if we disconnect from the network and connect to a new base station when there are messages in the queue? This would result in packet loss
        
        // TODO: Check that this actually succeeded, and don't block forever.
        //       Might need to return a status to indicate to calling function that we failed to send message
        if(osMessagePut(radioTxMsgQ, (uint32_t)message, osWaitForever) == osOK)
        {
            SignalRadioTXNeeded();
        }
        else
        {
            DEBUG("SendToBaseStation failed due to message queue error\r\n");
        }
    }
    else
    {
        vPortFree(data);
    }
}

// The pointer passed into this function should have been allocated using 
// the pvPortMalloc() routine: it will be freed using the osFree routine
void SendToBroadcast(uint8_t* data, uint8_t size)
{
    RadioMessage* message = pvPortMalloc(sizeof(RadioMessage));
    message->pData = data;
    message->size = size;
    message->dest = 0xFFFFFFFF; // Send to broadcast address
    
    // TODO: Check that this actually succeeded, and don't block forever.
    //       Might need to return a status to indicate to calling function that we failed to send message
    if(osMessagePut(radioTxMsgQ, (uint32_t)message, 0) == osOK)
    {
        SignalRadioTXNeeded();
    }
    else
    {
        DEBUG("SendToBroadcast failed due to message queue error\r\n");
    }
}

// Local function implementations

uint8_t SendRadioConfig(void)
{
    uint8_t retVal;
    
    si446x_reset();
    
    /* Wait until reset timeout or Reset IT signal */
    //for (wDelay = 0; wDelay < pRadioConfiguration->Radio_Delay_Cnt_After_Reset; wDelay++);
    osDelay(100);
    
    taskENTER_CRITICAL();
    retVal = si446x_configuration_init(pRadioConfiguration->Radio_ConfigurationArray);
    taskEXIT_CRITICAL();
    
    return retVal;
}

void SignalRadioIRQ(void)
{
    // Wakeup the radio task by putting a message on it's "wakeup" queue.
    // TODO: Check that this actually succeeded, and don't block forever.
    //       Might need to return a status to indicate to calling function that we failed to send message
    osMessagePut(radioWakeupMsgQ, RADIO_IRQ_DETECTED, osWaitForever);
}

void SignalRadioTXNeeded(void)
{
    // Wakeup the radio task by putting a message on it's "wakeup" queue.
    // TODO: Check that this actually succeeded, and don't block forever.
    //       Might need to return a status to indicate to calling function that we failed to send message
    osMessagePut(radioWakeupMsgQ, RADIO_TX_NEEDED, osWaitForever);
}

void Radio_StartTx_Variable_Packet(uint8_t channel, uint8_t *pioRadioPacket, uint8_t length)
{
  /* Leave RX state */
  si446x_change_state(SI446X_CMD_CHANGE_STATE_ARG_NEXT_STATE1_NEW_STATE_ENUM_READY);

  /* Read ITs, clear pending ones */
  si446x_get_int_status(0u, 0u, 0u);

  /* Reset the Tx Fifo */
  si446x_fifo_info(SI446X_CMD_FIFO_INFO_ARG_FIFO_TX_BIT);

  /* Fill the TX fifo with datas */
  si446x_write_tx_fifo(length, pioRadioPacket);

  /* Start sending packet, channel 0, START immediately */
   si446x_start_tx(channel, 0x30, length);
}

void Radio_StartRX(uint8_t channel)
{
    // Read ITs, clear pending ones
    si446x_get_int_status(0u, 0u, 0u);

    // Reset the FIFO
    si446x_fifo_info(SI446X_CMD_FIFO_INFO_ARG_FIFO_RX_BIT);

    /* Start Receiving packet, channel 0, START immediately, Packet n bytes long */
    si446x_start_rx(channel, 0u, RadioConfiguration.Radio_PacketLength,
                      SI446X_CMD_START_RX_ARG_NEXT_STATE1_RXTIMEOUT_STATE_ENUM_NOCHANGE,
                      SI446X_CMD_START_RX_ARG_NEXT_STATE2_RXVALID_STATE_ENUM_READY,
                      SI446X_CMD_START_RX_ARG_NEXT_STATE3_RXINVALID_STATE_ENUM_RX );
}

void RadioTaskHandleIRQ(void)
{
    uint8_t                         phInt = 0;
    uint8_t                         chipInt = 0;
    uint8_t                         modemInt = 0;
    
    // Get rid of annoying warnings
    (void)modemInt;
    (void)chipInt;
    
    // Get the interrupts from the radio: clear them all
    si446x_get_int_status(0u, 0u, 0u);
    
    // Only process status flags if they triggered this interrupt
    phInt = Si446xCmd.GET_INT_STATUS.PH_PEND; // & Si446xCmd.GET_INT_STATUS.PH_STATUS;
    chipInt = Si446xCmd.GET_INT_STATUS.CHIP_PEND; // & Si446xCmd.GET_INT_STATUS.CHIP_STATUS;
    modemInt = Si446xCmd.GET_INT_STATUS.MODEM_PEND; // & Si446xCmd.GET_INT_STATUS.MODEM_STATUS;      
    
    // Read the RX buffer in case there was something there
    si446x_read_rx_fifo(RadioConfiguration.Radio_PacketLength, rxBuff);
    
    // Start RX operations
    Radio_StartRX(pRadioConfiguration->Radio_ChannelNumber);
    
    // PACKET_SENT
    if(phInt & PACKET_SENT)
    {
        // TODO: Packet was transmitted, move to the "wait for ACK" state
        DEBUG("Packet TX completed event\r\n");
    }
    
    // PACKET_RX
    if(phInt & PACKET_RX)
    {
        DEBUG("Packet RX event\r\n");
        
        generic_message_t* message = (generic_message_t*)rxBuff;
        generic_message_t* generic_msg;
        
        if(message->dst == RadioGetMACAddress() || message->dst == 0xFFFFFFFF)
        {
            switch(message->cmd)
            {
                // The base station has requested info from us: reply with it
                case DEVICE_INFO:
                    // TODO: assemble device info struct and place on radio TX queue
                    break;
                
                case SENSOR_MSG:
                    // TODO: the base station wants a sensor update ASAP
                    break;
                
                case FW_UPD_START:
                    // TODO: add a timer to automatically re-enable TX after 30 minutes
                    inhibitTx = true;
                    FwUpdateStart();
                    break;
                
                case FW_UPD_END:
                    inhibitTx = false;
                    FwUpdateEnd();
                    break;
                    
                case FW_UPD_PAYLOAD:
                    for(uint8_t i = 0; i < NUM_FW_UPDATE_PAYLOAD_WORDS; i++)
                    {
                        FwUpdateWriteWord(message->payload.fw_update_data.payload[i], message->payload.fw_update_data.offset + (i * 4));
                    }
                    break;
                    
                case PING:
                    INFO("PING\r\n");
                    generic_msg = pvPortMalloc(sizeof(generic_message_t));
                
                    // TODO: check we didn't run out of RAM (we should catch this in the 
                    //       application Malloc failed handler, but just in case)
                
                    generic_msg->cmd = PONG;
                    generic_msg->dst = message->src;
                    generic_msg->src = RadioGetMACAddress();
                    
                    SendToDevice((uint8_t*)generic_msg, sizeof(generic_message_t), message->src);
                    break;
                
                case PONG:
                    INFO("PONG\r\n");
                    break;
                
                case JOIN:
                    INFO("Received JOIN message. Connecting to base station...\r\n");
                    radioTaskState = CONNECTED;
                    network.baseStationMac = message->src;
                    break;
                
                case RSSI:
                    DEBUG("Received RSSI message. Sending RSSI values...\r\n");
                
                    si446x_get_modem_status(0xFF);
                
                    generic_msg = pvPortMalloc(sizeof(generic_message_t));
                
                    // TODO: check we didn't run out of RAM (we should catch this in the 
                    //       application Malloc failed handler, but just in case)
                    
                    generic_msg->cmd = RSSI;
                    generic_msg->dst = message->src;
                    generic_msg->src = RadioGetMACAddress();
                    generic_msg->payload.rssi_message.curr_rssi  = Si446xCmd.GET_MODEM_STATUS.CURR_RSSI;
                    generic_msg->payload.rssi_message.latch_rssi = Si446xCmd.GET_MODEM_STATUS.LATCH_RSSI;
                    generic_msg->payload.rssi_message.ant1_rssi  = Si446xCmd.GET_MODEM_STATUS.ANT1_RSSI;
                    generic_msg->payload.rssi_message.ant2_rssi  = Si446xCmd.GET_MODEM_STATUS.ANT2_RSSI;
                
                    SendToDevice((uint8_t*)generic_msg, sizeof(generic_message_t), message->src);
                    break;
                
                case SENSOR_CMD:
                    SensorsCmd(message->payload.sensor_cmd);
                    break;
            }
            // TODO: send ACK
        }
    }
    
    // CRC_ERROR
    if(phInt & CRC_ERROR)
    {
        // TODO: Recevied a garbled packet. Reply with a NAK
    }
     
    // Other interesting interrupts:
    //  CHIP_READY
    //  LOW_BATT
    //  CMD_ERROR
    //  FIFO_UNDERFLOW_OVERFLOW_ERROR
    //  FILTER_MATCH 
    //  SYNC_DETECT
    //  PREAMBLE_DETECT
    //  RSSI
    //  RSSI_JUMP
    //  INVALID_SYNC
    //  TX_FIFO_ALMOST_EMPTY
    //  RX_FIFO_ALMOST_FULL
}

uint32_t RadioGetMACAddress(void)
{
    return *((uint32_t*)0x1FF80050);
}