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

#include <stdbool.h>
#include "platform.h"
#include "eprintf.h"

#ifndef __USART_FIFO_H__
#define __USART_FIFO_H__

#if defined PLATFORM_DEVICE_SANBOT_A

#define	UART1_FIFO_EN
// #define UART1_REMAP
#define UART1_BAUD		115200
#define UART_DEBUG_COM 	COM1

#endif

#if defined PLATFORM_DEVICE_SANBOT_DONGLE

#define	UART1_FIFO_EN
// #define UART1_REMAP
#define UART1_BAUD		230400
#define UART_DEBUG_COM 	COM1

#endif

#if defined PLATFORM_DEVICE_SANBOT_REMOTE

#define	UART1_FIFO_EN
// #define UART1_REMAP
#define UART1_BAUD		115200
#define UART_DEBUG_COM 	COM1

#endif

typedef enum
{
	COM1 = 0,
	COM2 = 1,
	COM3 = 2
}UART_PORT;

#if defined UART1_FIFO_EN
#if !defined UART1_BAUD
#define UART1_BAUD			115200
#endif
#define UART1_TX_BUF_SIZE	1*1024
#define UART1_RX_BUF_SIZE	1*1024
#endif

#if defined UART2_FIFO_EN
#if !defined UART2_BAUD
#define UART2_BAUD			9600
#endif
#define UART2_TX_BUF_SIZE	1*1024
#define UART2_RX_BUF_SIZE	1*1024
#endif

#if defined UART3_FIFO_EN
#if !defined UART3_BAUD
#define UART3_BAUD			9600
#endif
#define UART3_TX_BUF_SIZE	1*1024
#define UART3_RX_BUF_SIZE	1*1024
#endif

typedef struct
{
	USART_TypeDef *uart;		/* STM32内部串口设备指针 */
	uint8_t *pTxBuf;			/* 发送缓冲区 */
	uint8_t *pRxBuf;			/* 接收缓冲区 */
	uint16_t usTxBufSize;		/* 发送缓冲区大小 */
	uint16_t usRxBufSize;		/* 接收缓冲区大小 */
	uint16_t usTxWrite;			/* 发送缓冲区写指针 */
	uint16_t usTxRead;			/* 发送缓冲区读指针 */
	uint16_t usTxCount;			/* 等待发送的数据个数 */

	uint16_t usRxWrite;			/* 接收缓冲区写指针 */
	uint16_t usRxRead;			/* 接收缓冲区读指针 */

	uint16_t usRxCount;			/* 还未读取的新数据个数 */

	void (*SendBefor)(void); 	/* 开始发送之前的回调函数指针（主要用于RS485切换到发送模式） */
	void (*SendOver)(void); 	/* 发送完毕的回调函数指针（主要用于RS485将发送模式切换为接收模式） */
	void (*ReciveNew)(void);	/* 串口收到数据的回调函数指针 */
}UART_T;

void uartInit(void);

int uartPutchar(int ch);

void comSendBuf(UART_PORT _ucPort, uint8_t *_ucaBuf, uint16_t _usLen);
void comSendChar(UART_PORT _ucPort, uint8_t _ucByte);
uint8_t comGetChar(UART_PORT _ucPort, uint8_t *_pByte);

uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte);
void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen);

void comClearTxFifo(UART_PORT _ucPort);
void comClearRxFifo(UART_PORT _ucPort);

#define uartPrintf(FMT, ...) eprintf(uartPutchar, FMT, ## __VA_ARGS__)

extern UART_T g_tUart1;

#endif
