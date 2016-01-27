#include "uart.h"

#define UART_ENABLE(Instance)                   Instance->CR1 |= USART_CR1_UE
#define UART_DISABLE(Instance)                  Instance->CR1 &= ~USART_CR1_UE

char* uartTxBuff = NULL;
uint8_t txSize = 0;
uint8_t txPos = 0;

void UART_CharTX(USART_TypeDef* uart, char c)
{
    uart->TDR = c;
    while((uart->ISR & USART_ISR_TXE) != USART_ISR_TXE);
}

UART_Status UART_ReadyTX(USART_TypeDef* uart)
{
    if(uartTxBuff != NULL || txSize != 0)
    {
        return UART_NOT_READY;
    }
    
    return UART_OK;
}

UART_Status UART_StartRX(USART_TypeDef* uart)
{
    uint32_t ticks = 0;
    
    // Enable USART receive mode
    uart->CR1 |= USART_CR1_RE;
    
    while((uart->ISR & USART_ISR_REACK) != USART_ISR_REACK && ticks < UART_TIMEOUT_TICKS)
    {
        ticks++;
    }
    
    // Enable RX interrupts
    uart->CR1 |= USART_CR1_RXNEIE;
    
    return UART_OK;
}

UART_Status UART_StartTX(USART_TypeDef* uart, char* buff, uint8_t size)
{
    uartTxBuff = buff;
    txSize = size;
    txPos = 0;
    
    UART_ContinueTX(uart);
    
    // Enable TX interrupts
    uart->CR1 |= USART_CR1_TXEIE;
    
    return UART_OK;
}

void UART_ContinueTX(USART_TypeDef* uart)
{
    uint32_t ticks = 0;
    
    while((uart->ISR & USART_ISR_TXE) != USART_ISR_TXE && ticks < UART_TIMEOUT_TICKS)
    {
        ticks++;
    }
    
    if(uartTxBuff == NULL || txSize == 0 || txPos >= txSize)
    {
        return;
    }
    
    uart->TDR = uartTxBuff[txPos++];
    
    // Finished transmitting
    if(txPos >= txSize)
    {
        txPos = 0;
        txSize = 0;
        uartTxBuff = NULL;
        
        // Disable TX interrupts
        uart->CR1 &= (~USART_CR1_TXEIE);
    }
}

#ifdef USE_MY_UART

UART_Status UART_Init(USART_TypeDef* uart)
{
  /* Check the UART handle allocation */
  if(uart == NULL)
  {
    return UART_PARAM_ERROR;
  }
  
  /* Disable the Peripheral */
  UART_DISABLE(uart);
  
  HAL_UART_MspInit(huart);
  
  /* Set the UART Communication parameters */
  UART_SetConfig(huart);
  
  /* In asynchronous mode, the following bits must be kept cleared: 
  - LINEN and CLKEN bits in the USART_CR2 register,
  - SCEN, HDSEL and IREN  bits in the USART_CR3 register.*/
  huart->Instance->CR2 &= ~(USART_CR2_LINEN | USART_CR2_CLKEN); 
  huart->Instance->CR3 &= ~(USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN); 
    
  /* Enable the Peripheral */
  UART_ENABLE(uart);
  
  /* TEACK and/or REACK to check before moving huart->State to Ready */
  return UART_CheckIdleState(uart);
}

UART_Status UART_CheckIdleState(USART_TypeDef *uart)
{
    /* Check if the Transmitter is enabled */
    if((uart->CR1 & USART_CR1_TE) == USART_CR1_TE)
    {
        while((uart->ISR & USART_ISR_TEACK) != USART_ISR_TEACK)
        {
            ;
        }
    }
    /* Check if the Receiver is enabled */
    if((uart->CR1 & USART_CR1_RE) == USART_CR1_RE)
    {
        while((uart->ISR & USART_ISR_REACK) != USART_ISR_REACK)
        {
            ;
        }
    }
  
  return UART_OK;
}

void UART_SetConfig(USART_TypeDef *uart)
{
  uint32_t tmpreg = 0x00000000;
  uint32_t clocksource = 0x00000000;
  uint16_t brrtemp = 0x0000;
  uint16_t usartdiv = 0x0000;
    
  uart->CR1 |= (USART_CR1_TE | USART_CR1_RE);

  /*-------------------------- USART CR2 Configuration -----------------------*/
  /* Configure the UART Stop Bits: Set STOP[13:12] bits according 
   * to huart->Init.StopBits value */
  MODIFY_REG(huart->Instance->CR2, USART_CR2_STOP, huart->Init.StopBits);
  
  /*-------------------------- USART CR3 Configuration -----------------------*/
  /* Configure 
   * - UART HardWare Flow Control: set CTSE and RTSE bits according 
   *   to huart->Init.HwFlowCtl value 
   * - one-bit sampling method versus three samples' majority rule according
   *   to huart->Init.OneBitSampling */
  tmpreg = (uint32_t)huart->Init.HwFlowCtl | huart->Init.OneBitSampling ;
  MODIFY_REG(huart->Instance->CR3, (USART_CR3_RTSE | USART_CR3_CTSE | USART_CR3_ONEBIT), tmpreg);
  
  /*-------------------------- USART BRR Configuration -----------------------*/
  __HAL_UART_GETCLOCKSOURCE(huart, clocksource);
  
  /* Check LPUART instace */
  if(huart->Instance == LPUART1)
  {
    switch (clocksource)
    {
    case UART_CLOCKSOURCE_PCLK1: 
      huart->Instance->BRR = (uint32_t)(__DIV_LPUART(HAL_RCC_GetPCLK1Freq(), huart->Init.BaudRate));
      break;
    case UART_CLOCKSOURCE_HSI: 
      huart->Instance->BRR = (uint32_t)(__DIV_LPUART(HSI_VALUE, huart->Init.BaudRate)); 
      break; 
    case UART_CLOCKSOURCE_SYSCLK:  
      huart->Instance->BRR = (uint32_t)(__DIV_LPUART(HAL_RCC_GetSysClockFreq(), huart->Init.BaudRate));
      break;  
    case UART_CLOCKSOURCE_LSE:
      huart->Instance->BRR = (uint32_t)(__DIV_LPUART(LSE_VALUE, huart->Init.BaudRate)); 
      break;
    default:
      break;
    }
  }
  /* Check the UART Over Sampling 8 to set Baud Rate Register */
  else if (huart->Init.OverSampling == UART_OVERSAMPLING_8)
  { 
    switch (clocksource)
    {
    case UART_CLOCKSOURCE_PCLK1:
      usartdiv = (uint32_t)(__DIV_SAMPLING8(HAL_RCC_GetPCLK1Freq(), huart->Init.BaudRate));
      break;
    case UART_CLOCKSOURCE_PCLK2:
      usartdiv = (uint32_t)(__DIV_SAMPLING8(HAL_RCC_GetPCLK2Freq(), huart->Init.BaudRate));
      break;
    case UART_CLOCKSOURCE_HSI:
      usartdiv = (uint32_t)(__DIV_SAMPLING8(HSI_VALUE, huart->Init.BaudRate)); 
      break;
    case UART_CLOCKSOURCE_SYSCLK:
      huart->Instance->BRR = (uint32_t)(__DIV_SAMPLING8(HAL_RCC_GetSysClockFreq(), huart->Init.BaudRate));
      break;
    case UART_CLOCKSOURCE_LSE:
      usartdiv = (uint32_t)(__DIV_SAMPLING8(LSE_VALUE, huart->Init.BaudRate)); 
      break;
    default:
      break;
    }
    
    brrtemp = usartdiv & 0xFFF0;
    brrtemp |= (uint16_t)((uint16_t)(usartdiv & (uint16_t)0x000F) >> (uint16_t)1);
    huart->Instance->BRR = brrtemp;
  }
  else
  {
    switch (clocksource)
    {
    case UART_CLOCKSOURCE_PCLK1: 
      huart->Instance->BRR = (uint32_t)(__DIV_SAMPLING16(HAL_RCC_GetPCLK1Freq(), huart->Init.BaudRate));
      break;
    case UART_CLOCKSOURCE_PCLK2: 
      huart->Instance->BRR = (uint32_t)(__DIV_SAMPLING16(HAL_RCC_GetPCLK2Freq(), huart->Init.BaudRate));
      break;
    case UART_CLOCKSOURCE_HSI: 
      huart->Instance->BRR = (uint32_t)(__DIV_SAMPLING16(HSI_VALUE, huart->Init.BaudRate)); 
      break; 
    case UART_CLOCKSOURCE_SYSCLK:  
      huart->Instance->BRR = (uint32_t)(__DIV_SAMPLING16(HAL_RCC_GetSysClockFreq(), huart->Init.BaudRate));
      break;  
    case UART_CLOCKSOURCE_LSE:
      huart->Instance->BRR = (uint32_t)(__DIV_SAMPLING16(LSE_VALUE, huart->Init.BaudRate)); 
      break;
    default:
      break;
    }
  }
}

#endif
