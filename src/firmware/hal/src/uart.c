/*
 *  ____    ____    __   __   ______  ______
 * /\  _`\ /\  _`\ /\ \ /\ \ /\__  _\/\  _  \
 * \ \ \/\ \ \ \L\_\ `\`\/'/'\/_/\ \/\ \ \L\ \
 *  \ \ \ \ \ \  _\L`\/ > <     \ \ \ \ \  __ \
 *   \ \ \_\ \ \ \L\ \ \/'/\`\   \ \ \ \ \ \/\ \
 *    \ \____/\ \____/ /\_\\ \_\  \ \_\ \ \_\ \_\
 *     \/___/  \/___/  \/_/ \/_/   \/_/  \/_/\/_/
 *
 * Originally created by Dexta Robotics.
 * Copyright <C> Dexta Robotics, 2015.
 * All rights reserved.
 */

#define DEBUG_MODULE "UART"

#include <string.h>

/*ST includes */
#include "stm32f10x.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "misc.h"

#include "uart.h"
#include "config.h"
#include "nvicconf.h"
#include "debug.h"

#if defined PLATFORM_DEVICE_SANBOT_A 

  #define UART_TYPE       USART1
  #define UART_PERIF      RCC_APB2Periph_USART1

  #define UART_DMA_IRQ    DMA1_Channel4_IRQn
  #define UART_DMA_IT_TC  DMA_IT_TC
  #define UART_DMA_CH     DMA1_Channel4

  #define UART_GPIO_PERIF RCC_APB2Periph_GPIOA
  #define UART_GPIO_PORT  GPIOA
  #define UART_GPIO_TX    GPIO_Pin_9
  #define UART_GPIO_RX    GPIO_Pin_10

  #define UART_NVIC_IRQn  USART1_IRQn

#endif

// #define UART_TYPE       USART1
// #define UART_PERIF      RCC_APB2Periph_USART1

// #define UART_DMA_IRQ    DMA1_Channel4_IRQn
// #define UART_DMA_IT_TC  DMA_IT_TC
// #define UART_DMA_CH     DMA1_Channel4

// #define UART_GPIO_PERIF RCC_APB2Periph_GPIOA
// #define UART_GPIO_PORT  GPIOA
// #define UART_GPIO_TX    GPIO_Pin_9
// #define UART_GPIO_RX    GPIO_Pin_10

// #define UART_NVIC_IRQn USART1_IRQn


static uint8_t outBuffer[64];
static DMA_InitTypeDef DMA_InitStructureShare;

#define CCR_ENABLE_SET  ((uint32_t)0x00000001)

void uartRxTask(void *param);

void uartDmaInit(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  DMA_InitStructureShare.DMA_PeripheralBaseAddr = (uint32_t)&UART_TYPE->DR;
  DMA_InitStructureShare.DMA_MemoryBaseAddr = (uint32_t)outBuffer;
  DMA_InitStructureShare.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructureShare.DMA_BufferSize = 0;
  DMA_InitStructureShare.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructureShare.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructureShare.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructureShare.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructureShare.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructureShare.DMA_Priority = DMA_Priority_High;
  DMA_InitStructureShare.DMA_M2M = DMA_M2M_Disable;

  NVIC_InitStructure.NVIC_IRQChannel = UART_DMA_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_UART_PRI;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void uartInit(void)
{

  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable GPIO and USART clock */
  RCC_APB2PeriphClockCmd(UART_GPIO_PERIF, ENABLE);

#if defined PLATFORM_DEVICE_SANBOT_A
  RCC_APB2PeriphClockCmd(UART_PERIF, ENABLE);
#endif

  /* Configure USART Rx as input floating */
  GPIO_InitStructure.GPIO_Pin   = UART_GPIO_RX;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_Init(UART_GPIO_PORT, &GPIO_InitStructure);
/* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin   = UART_GPIO_TX;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
  GPIO_Init(UART_GPIO_PORT, &GPIO_InitStructure);

  USART_InitStructure.USART_BaudRate            = 115200;
  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;

  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;
  USART_InitStructure.USART_Parity              = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_Init(UART_TYPE, &USART_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = UART_NVIC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_ITConfig(UART_TYPE, USART_IT_RXNE, ENABLE);

  USART_Cmd(UART_TYPE, ENABLE);

  // uartDmaInit();

  DEBUG_PRINT("init successfully\n");
}

void uartRxTask(void *param)
{

}

void uartSendData(uint32_t size, uint8_t* data)
{
  uint32_t i;

  for(i = 0; i < size; i++)
  {
    while (!(UART_TYPE->SR & USART_FLAG_TXE));
    UART_TYPE->DR = (data[i] & 0xFF);
  }
}

int uartPutchar(int ch)
{
    uartSendData(1, (uint8_t *)&ch);

    return (unsigned char)ch;
}

void uartSendDataDma(uint32_t size, uint8_t* data)
{
    memcpy(outBuffer, data, size);
    DMA_InitStructureShare.DMA_BufferSize = size;
    // Wait for DMA to be free
    while(UART_DMA_CH->CCR & CCR_ENABLE_SET);
    DMA_Init(UART_DMA_CH, &DMA_InitStructureShare);
    // Enable the Transfer Complete interrupt
    DMA_ITConfig(UART_DMA_CH, DMA_IT_TC, ENABLE);
    USART_DMACmd(UART_TYPE, USART_DMAReq_Tx, ENABLE);
    DMA_Cmd(UART_DMA_CH, ENABLE);
}

void uartIsr(void)
{

}

void uartDmaIsr(void)
{

}

#if defined PLATFORM_REV_DEXMO_A_RIGHT

void __attribute__((used)) USART3_IRQHandler(void)
{
  uartIsr();
}

void __attribute__((used)) DMA1_Channel6_IRQHandler(void)
{
#if defined(UART_OUTPUT_TRACE_DATA) || defined(ADC_OUTPUT_RAW_DATA)
  uartDmaIsr();
#endif
}

#elif defined PLATFORM_REV_DEXMO_A_LEFT || defined PLATFORM_REV_DEXMO_DONGLE_A

void __attribute__((used)) USART1_IRQHandler(void)
{
  uartIsr();
}

void __attribute__((used)) DMA1_Channel4_IRQHandler(void)
{
#if defined(UART_OUTPUT_TRACE_DATA) || defined(ADC_OUTPUT_RAW_DATA)
  uartDmaIsr();
#endif
}

#endif

