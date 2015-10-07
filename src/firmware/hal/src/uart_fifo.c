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

#define DEBUG_MODULE "UART_FIFO"

#include <string.h>

/*ST includes */
#include "stm32f10x.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "misc.h"

#include "uart_fifo.h"
#include "config.h"
#include "nvicconf.h"
#include "rcc.h"
#include "debug.h"

#if defined UART1_FIFO_EN
	UART_T g_tUart1;
	static uint8_t g_TxBuf1[UART1_TX_BUF_SIZE];
	static uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];
#endif

#if defined UART2_FIFO_EN
	static UART_T g_tUart2;
	static uint8_t g_TxBuf2[UART2_TX_BUF_SIZE];
	static uint8_t g_RxBuf2[UART2_RX_BUF_SIZE];
#endif

#if defined UART3_FIFO_EN
	static UART_T g_tUart3;
	static uint8_t g_TxBuf3[UART3_TX_BUF_SIZE];
	static uint8_t g_RxBuf3[UART3_RX_BUF_SIZE];
#endif

static void UartIRQ(UART_T *_pUart);
#if defined UART1_FIFO_EN
static void uartInitCom1(uint32_t _uiBaud);
#endif
#if defined UART2_FIFO_EN
static void uartInitCom2(uint32_t _uiBaud);
#endif
#if defined UART3_FIFO_EN
static void uartInitCom3(uint32_t _uiBaud);
#endif

void uartInit(void)
{
	/* Configure the NVIC Preemption Priority Bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

#if defined UART1_FIFO_EN
	uartInitCom1(UART1_BAUD);
#endif

#if defined UART2_FIFO_EN
	uartInitCom2(UART2_BAUD);
#endif

#if defined UART3_FIFO_EN
	uartInitCom3(UART3_BAUD);
#endif

}

/*
*********************************************************************************************************
*	函 数 名: ComToUart
*	功能说明: 将COM端口号转换为UART指针
*	形    参: _ucPort: 端口号(COM1 - COM6)
*	返 回 值: uart指针
*********************************************************************************************************
*/
UART_T *ComToUart(UART_PORT _ucPort)
{
	if (_ucPort == COM1)
	{
		#if defined UART1_FIFO_EN
			return &g_tUart1;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM2)
	{
		#if defined UART2_FIFO_EN
			return &g_tUart2;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == COM3)
	{
		#if defined UART3_FIFO_EN
			return &g_tUart3;
		#else
			return 0;
		#endif
	}
	else
	{
		/* 不做任何处理 */
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: comSendBuf
*	功能说明: 向串口发送一组数据。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(COM1 - COM6)
*			  _ucaBuf: 待发送的数据缓冲区
*			  _usLen : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void comSendBuf(UART_PORT _ucPort, uint8_t *_ucaBuf, uint16_t _usLen)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	if (pUart->SendBefor != 0)
	{
		pUart->SendBefor();		/* 如果是RS485通信，可以在这个函数中将RS485设置为发送模式 */
	}

	UartSend(pUart, _ucaBuf, _usLen);
}

/*
*********************************************************************************************************
*	函 数 名: comSendChar
*	功能说明: 向串口发送1个字节。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(COM1 - COM6)
*			  _ucByte: 待发送的数据
*	返 回 值: 无
*********************************************************************************************************
*/
void comSendChar(UART_PORT _ucPort, uint8_t _ucByte)
{
	comSendBuf(_ucPort, &_ucByte, 1);
}

/*
*********************************************************************************************************
*	函 数 名: comGetChar
*	功能说明: 从串口缓冲区读取1字节，非阻塞。无论有无数据均立即返回
*	形    参: _ucPort: 端口号(COM1 - COM6)
*			  _pByte: 接收到的数据存放在这个地址
*	返 回 值: 0 表示无数据, 1 表示读取到有效字节
*********************************************************************************************************
*/
uint8_t comGetChar(UART_PORT _ucPort, uint8_t *_pByte)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return 0;
	}

	return UartGetChar(pUart, _pByte);
}

/*
*********************************************************************************************************
*	函 数 名: comClearTxFifo
*	功能说明: 清零串口发送缓冲区
*	形    参: _ucPort: 端口号(COM1 - COM6)
*	返 回 值: 无
*********************************************************************************************************
*/
void comClearTxFifo(UART_PORT _ucPort)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	pUart->usTxWrite = 0;
	pUart->usTxRead = 0;
	pUart->usTxCount = 0;
}

/*
*********************************************************************************************************
*	函 数 名: comClearRxFifo
*	功能说明: 清零串口接收缓冲区
*	形    参: _ucPort: 端口号(COM1 - COM6)
*	返 回 值: 无
*********************************************************************************************************
*/
void comClearRxFifo(UART_PORT _ucPort)
{
	UART_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}

	pUart->usRxWrite = 0;
	pUart->usRxRead = 0;
	pUart->usRxCount = 0;
}

#if defined UART1_FIFO_EN
void uartInitCom1(uint32_t _uiBaud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	USART_Cmd(USART1, DISABLE);

	g_tUart1.uart = USART1;
	g_tUart1.pTxBuf = g_TxBuf1;
	g_tUart1.pRxBuf = g_RxBuf1;
	g_tUart1.usTxBufSize = UART1_TX_BUF_SIZE;
	g_tUart1.usRxBufSize = UART1_RX_BUF_SIZE;
	g_tUart1.usTxWrite = 0;
	g_tUart1.usTxRead = 0;
	g_tUart1.usRxWrite = 0;
	g_tUart1.usRxRead = 0;
	g_tUart1.usRxCount = 0;
	g_tUart1.usTxCount = 0;
	g_tUart1.SendBefor = 0;
	g_tUart1.SendOver = 0;
	g_tUart1.ReciveNew = 0;

#if !defined UART1_REMAP
	/* TX = PA9   RX = PA10 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#else

#endif

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_InitStructure.USART_BaudRate = _uiBaud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART1, ENABLE);

	USART_ClearFlag(USART1, USART_FLAG_TC);

	{
		NVIC_InitTypeDef NVIC_InitStructure;

		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}


}
#endif
#if defined UART2_FIFO_EN
void uartInitCom2(uint32_t _uiBaud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	USART_Cmd(USART2, DISABLE);

	g_tUart2.uart = USART2;						/* STM32 串口设备 */
	g_tUart2.pTxBuf = g_TxBuf2;					/* 发送缓冲区指针 */
	g_tUart2.pRxBuf = g_RxBuf2;					/* 接收缓冲区指针 */
	g_tUart2.usTxBufSize = UART2_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart2.usRxBufSize = UART2_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart2.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart2.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart2.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart2.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart2.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart2.usTxCount = 0;						/* 待发送的数据个数 */
	g_tUart2.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart2.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_tUart2.ReciveNew = 0;						/* 接收到新数据后的回调函数 */

#if !defined UART2_REMAP
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	/* 复用模式 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	/* Configure USART2 Rx (PA.03) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART2 Tx (PA.02) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#else
#endif

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	USART_InitStructure.USART_BaudRate = _uiBaud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART2, ENABLE);

	USART_ClearFlag(USART2, USART_FLAG_TC);

	{
		NVIC_InitTypeDef NVIC_InitStructure;

		/* 使能串口2中断 */
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
}
#endif
#if defined UART3_FIFO_EN
void uartInitCom3(uint32_t _uiBaud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	USART_Cmd(USART3, DISABLE);

	g_tUart3.uart = USART3;
	g_tUart3.pTxBuf = g_TxBuf3;
	g_tUart3.pRxBuf = g_RxBuf3;
	g_tUart3.usTxBufSize = UART3_TX_BUF_SIZE;
	g_tUart3.usRxBufSize = UART3_RX_BUF_SIZE;
	g_tUart3.usTxWrite = 0;
	g_tUart3.usTxRead = 0;
	g_tUart3.usRxWrite = 0;
	g_tUart3.usRxRead = 0;
	g_tUart3.usRxCount = 0;
	g_tUart3.usTxCount = 0;
	g_tUart3.SendBefor = 0;
	g_tUart3.SendOver = 0;
	g_tUart3.ReciveNew = 0;

#if !defined UART3_REMAP

#else
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);
#endif

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	USART_InitStructure.USART_BaudRate = UART3_BAUD;	/* 波特率 */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART3, ENABLE);

	USART_ClearFlag(USART3, USART_FLAG_TC); 

	{
		NVIC_InitTypeDef NVIC_InitStructure;

		/* 使能串口3中断t */
		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
}
#endif

/*
*********************************************************************************************************
*	函 数 名: UartSend
*	功能说明: 填写数据到UART发送缓冲区,并启动发送中断。中断处理函数发送完毕后，自动关闭发送中断
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen)
{
	uint16_t i;

	for (i = 0; i < _usLen; i++)
	{
		/* 如果发送缓冲区已经满了，则等待缓冲区空 */
	#if 0
		/*
			在调试GPRS例程时，下面的代码出现死机，while 死循环
			原因： 发送第1个字节时 _pUart->usTxWrite = 1；_pUart->usTxRead = 0;
			将导致while(1) 无法退出
		*/
		while (1)
		{
			uint16_t usRead;

			DISABLE_INT();
			usRead = _pUart->usTxRead;
			ENABLE_INT();

			if (++usRead >= _pUart->usTxBufSize)
			{
				usRead = 0;
			}

			if (usRead != _pUart->usTxWrite)
			{
				break;
			}
		}
	#else
		while (1)
		{
			uint16_t usCount;

			DISABLE_INT();
			usCount = _pUart->usTxCount;
			ENABLE_INT();

			if (usCount < _pUart->usTxBufSize)
			{
				break;
			}
		}
	#endif

		/* 将新数据填入发送缓冲区 */
		_pUart->pTxBuf[_pUart->usTxWrite] = _ucaBuf[i];

		DISABLE_INT();
		if (++_pUart->usTxWrite >= _pUart->usTxBufSize)
		{
			_pUart->usTxWrite = 0;
		}
		_pUart->usTxCount++;
		ENABLE_INT();
	}

	USART_ITConfig(_pUart->uart, USART_IT_TXE, ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: UartGetChar
*	功能说明: 从串口接收缓冲区读取1字节数据 （用于主程序调用）
*	形    参: _pUart : 串口设备
*			  _pByte : 存放读取数据的指针
*	返 回 值: 0 表示无数据  1表示读取到数据
*********************************************************************************************************
*/
uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte)
{
	uint16_t usCount;

	/* usRxWrite 变量在中断函数中被改写，主程序读取该变量时，必须进行临界区保护 */
	DISABLE_INT();
	usCount = _pUart->usRxCount;
	ENABLE_INT();

	/* 如果读和写索引相同，则返回0 */
	//if (_pUart->usRxRead == usRxWrite)
	if (usCount == 0)	/* 已经没有数据 */
	{
		return 0;
	}
	else
	{
		*_pByte = _pUart->pRxBuf[_pUart->usRxRead];		/* 从串口接收FIFO取1个数据 */

		/* 改写FIFO读索引 */
		DISABLE_INT();
		if (++_pUart->usRxRead >= _pUart->usRxBufSize)
		{
			_pUart->usRxRead = 0;
		}
		_pUart->usRxCount--;
		ENABLE_INT();
		return 1;
	}
}

/*
*********************************************************************************************************
*	函 数 名: UartIRQ
*	功能说明: 供中断服务程序调用，通用串口中断处理函数
*	形    参: _pUart : 串口设备
*	返 回 值: 无
*********************************************************************************************************
*/
static void UartIRQ(UART_T *_pUart)
{

	if (USART_GetITStatus(_pUart->uart, USART_IT_RXNE) != RESET)
	{

		_pUart->pRxBuf[_pUart->usRxWrite] = USART_ReceiveData(_pUart->uart);
		if (++_pUart->usRxWrite >= _pUart->usRxBufSize)
		{
			_pUart->usRxWrite = 0;
		}
		if (_pUart->usRxCount < _pUart->usRxBufSize)
		{
			_pUart->usRxCount++;
		}

		if (_pUart->ReciveNew)
		{
			_pUart->ReciveNew();
		}
	}

	/* 处理发送缓冲区空中断 */
	if (USART_GetITStatus(_pUart->uart, USART_IT_TXE) != RESET)
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* 发送缓冲区的数据已取完时， 禁止发送缓冲区空中断 （注意：此时最后1个数据还未真正发送完毕）*/
			USART_ITConfig(_pUart->uart, USART_IT_TXE, DISABLE);

			/* 使能数据发送完毕中断 */
			USART_ITConfig(_pUart->uart, USART_IT_TC, ENABLE);
		}
		else
		{
			/* 从发送FIFO取1个字节写入串口发送数据寄存器 */
			USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}

	}
	/* 数据bit位全部发送完毕的中断 */
	else if (USART_GetITStatus(_pUart->uart, USART_IT_TC) != RESET)
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* 如果发送FIFO的数据全部发送完毕，禁止数据发送完毕中断 */
			USART_ITConfig(_pUart->uart, USART_IT_TC, DISABLE);

			/* 回调函数, 一般用来处理RS485通信，将RS485芯片设置为接收模式，避免抢占总线 */
			if (_pUart->SendOver)
			{
				_pUart->SendOver();
			}
		}
		else
		{
			/* 正常情况下，不会进入此分支 */

			/* 如果发送FIFO的数据还未完毕，则从发送FIFO取1个数据写入发送数据寄存器 */
			USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}
	}
}

int uartPutchar(int ch)
{
    comSendChar(UART_DEBUG_COM, ch);

    return (unsigned char)ch;
}

#if defined UART1_FIFO_EN
void __attribute__((used)) USART1_IRQHandler(void)
{
	UartIRQ(&g_tUart1);
}
#endif

#if defined UART2_FIFO_EN
void __attribute__((used)) USART2_IRQHandler(void)
{
	UartIRQ(&g_tUart2);
}
#endif

#if defined UART3_FIFO_EN
void __attribute__((used)) USART3_IRQHandler(void)
{
	UartIRQ(&g_tUart3);
}
#endif

