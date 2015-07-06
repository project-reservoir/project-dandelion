#include "radio.h"
#include "radio_config.h"
#include "cmsis_os.h"
#include "si446x_api_lib.h"

#define RADIO_MAX_PACKET_LENGTH     64u

// Typedefs
typedef enum RadioTaskState_t {
    CONNECTED,
    CONNECTING,
    SEARCHING
} RadioTaskState;

typedef struct NetworkInfo_t {
    uint32_t baseStationMac;
} NetworkInfo;

typedef struct RadioMessage_t {
    uint8_t* pData;
    uint8_t  size;
    uint32_t dest;
} RadioMessage;

typedef struct
{
    uint8_t   *Radio_ConfigurationArray;

    uint8_t   Radio_ChannelNumber;
    uint8_t   Radio_PacketLength;
    uint8_t   Radio_State_After_Power_Up;

    uint16_t  Radio_Delay_Cnt_After_Reset;

    uint8_t   Radio_CustomPayload[RADIO_MAX_PACKET_LENGTH];
} tRadioConfiguration;

// Global variables
osMessageQId radioMsgQ;
SPI_HandleTypeDef SpiHandle;

uint8_t Radio_Configuration_Data_Array[] = RADIO_CONFIGURATION_DATA_ARRAY;

tRadioConfiguration RadioConfiguration = RADIO_CONFIGURATION_DATA;

tRadioConfiguration* pRadioConfiguration = &RadioConfiguration;

uint8_t customRadioPacket[RADIO_MAX_PACKET_LENGTH];

// Local variables 
static uint8_t txBuff[BUFFSIZE];
static uint8_t rxBuff[BUFFSIZE];

static RadioTaskState radioTaskState = SEARCHING;
static NetworkInfo    network;

// Local function prototypes
static void SendRadioConfig(void);

// Global function implementations
void RadioTaskOSInit(void)
{
    osMessageQDef(RadioMsgQueue, RADIO_MSG_QUEUE_SIZE, RadioMessage*);
    
    radioMsgQ = osMessageCreate(osMessageQ(RadioMsgQueue), NULL);
    
    // TODO: check message queue was created OK
    
    SpiHandle.Instance               = SPIx;
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SpiHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;
    SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
    SpiHandle.Init.CRCPolynomial     = 7;
    SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.NSS               = SPI_NSS_HARD_OUTPUT;
    SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
    SpiHandle.Init.Mode              = SPI_MODE_MASTER;
    
    assert_param(HAL_SPI_Init(&SpiHandle) == HAL_OK);
}

void RadioTaskHwInit(void)
{
/*    GPIO_InitTypeDef  GPIO_InitStruct;

    SPIx_SCK_GPIO_CLK_ENABLE();
    SPIx_MISO_GPIO_CLK_ENABLE();
    SPIx_MOSI_GPIO_CLK_ENABLE();
  
    SPIx_CLK_ENABLE();
    
    __SYSCFG_CLK_ENABLE();

    GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = SPIx_SCK_AF;

    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SPIx_MISO_PIN;
    GPIO_InitStruct.Alternate = SPIx_MISO_AF;

    HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
    GPIO_InitStruct.Alternate = SPIx_MOSI_AF;

    HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = SPIx_NSEL_PIN;
    GPIO_InitStruct.Alternate = SPIx_NSEL_AF;

    HAL_GPIO_Init(SPIx_NSEL_GPIO_PORT, &GPIO_InitStruct);

    // Configure Radio SDL
    GPIO_InitStruct.Pin        = RADIO_SDL_PIN;
    GPIO_InitStruct.Pull       = GPIO_PULLDOWN;
    GPIO_InitStruct.Mode       = GPIO_MODE_OUTPUT_PP;   
    GPIO_InitStruct.Speed      = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate  = 0x00;
    
    HAL_GPIO_Init(RADIO_SDL_GPIO_PORT, &GPIO_InitStruct);
    
    // Configure Radio NIRQ
    GPIO_InitStruct.Pin        = RADIO_NIRQ_PIN;
    GPIO_InitStruct.Pull       = GPIO_NOPULL;
    GPIO_InitStruct.Mode       = GPIO_MODE_IT_RISING;   
    GPIO_InitStruct.Speed      = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate  = 0x00;
    
    HAL_GPIO_Init(RADIO_NIRQ_GPIO_PORT, &GPIO_InitStruct);
    
    // NVIC for SPI
    //HAL_NVIC_SetPriority(SPIx_IRQn, 0, 1);
    //HAL_NVIC_EnableIRQ(SPIx_IRQn);
    
    // Configure NVIC for NIRQ line
    //HAL_NVIC_SetPriority(NIRQ_IRQn, 0, 1);
    //HAL_NVIC_EnableIRQ(NIRQ_IRQn);*/
    
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
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
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

    /*##-3- Configure the NVIC for SPI #########################################*/
    /* NVIC for SPI */
    HAL_NVIC_SetPriority(SPIx_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(SPIx_IRQn);
}

void RadioTask(void)
{
    osEvent msgQueueEvent;
    RadioMessage* msg;
    // Load radio config from flash    
    
    //Configure radio
    SendRadioConfig();
    
    while(1)
    {
        switch(radioTaskState)
        {
            // Try and join the network. If success, enter CONNECTED state, else return to LOOKING_FOR_BASE_STATION
            case CONNECTING:
                
                break;
            
            // Perform regular radio duties. If we loose connection, enter the CONNECTING_TO_BASE_STATION state
            case CONNECTED:
                // Wait on the transmit queue
                msgQueueEvent = osMessageGet(radioMsgQ, 0);
                if(msgQueueEvent.status == osOK)
                {
                    msg = (RadioMessage*)(msgQueueEvent.value.p);
                    
                    // Transmit the packet to the radio hardware
                    
                    // Free the data
                    vPortFree(msg->pData);
                    vPortFree(msg);
                }            
                break;
            
            // Send ANNOUNCE packets every 20 seconds and see if we get a reply
            case SEARCHING:
                // Send message
                // Wait for reply
                // If network found, store details and move to -> CONNECTING
                break;
        }
        osDelay(1000);
    }
}

// The pointer passed into this function should have been allocated using 
// the osAlloc() routine: it will be freed using the osFree routine
void SendToBaseStation(uint8_t* data, uint8_t size)
{
    RadioMessage* message = pvPortMalloc(sizeof(RadioMessage));
    message->pData = data;
    message->size = size;
    message->dest = network.baseStationMac; // TODO: what if we disconnect from the network and connect to a new base station when there are messages in the queue? This would result in packet loss
    
    // TODO: Check that this actually succeeded, and don't block forever.
    //       Might need to return a status to indicate to calling function that we failed to send message
    osMessagePut (radioMsgQ, (uint32_t)message, 0);
}

// The pointer passed into this function should have been allocated using 
// the osAlloc() routine: it will be freed using the osFree routine
void SendToBroadcast(uint8_t* data, uint8_t size)
{
    RadioMessage* message = pvPortMalloc(sizeof(RadioMessage));
    message->pData = data;
    message->size = size;
    message->dest = 0xFFFFFFFF; // Send to broadcast address
    
    // TODO: Check that this actually succeeded, and don't block forever.
    //       Might need to return a status to indicate to calling function that we failed to send message
    osMessagePut (radioMsgQ, (uint32_t)message, 0);
}

// Local function implementations

void SendRadioConfig(void)
{
    uint16_t wDelay = 0;
    
    si446x_reset();
    
    /* Wait until reset timeout or Reset IT signal */
    for (wDelay = 0; wDelay < pRadioConfiguration->Radio_Delay_Cnt_After_Reset; wDelay++);
    
    si446x_configuration_init(pRadioConfiguration->Radio_ConfigurationArray);
}
