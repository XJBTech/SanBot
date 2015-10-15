#define DEBUG_MODULE "NRF"

#include <string.h>

#include "config.h"

#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_rcc.h"

#include "rcc.h"
#include "debug.h"
#include "nrf24l01.h"

#if defined PLATFORM_DEVICE_SANBOT_A

#define nRF24L01_DEBUG_SPI	"SPI1\n"
#define nRF24L01_SPI 		SPI1
#define nRF24L01_SPI1

#define nRF24L01_SPI_BaudRatePrescaler	SPI_BaudRatePrescaler_16

#define nRF24L01_DMA_DR_ADDR	SPI1_DR_Addr
#define nRF24L01_DMA_RX_ADDR	SPI1_Rx_Buff
#define nRF24L01_DMA_TX_ADDR	SPI1_Tx_Buff

#define nRF24L01_RCC_SPI  RCC_APB2Periph_SPI1
#define nRF24L01_RCC_ALL  RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC

#define nRF24L01_PORT_SPI	GPIOA
#define nRF24L01_PIN_SCK	GPIO_Pin_5
#define nRF24L01_PIN_MISO	GPIO_Pin_6
#define nRF24L01_PIN_MOSI	GPIO_Pin_7

#define nRF24L01_PORT_CE   GPIOB
#define nRF24L01_PIN_CE    GPIO_Pin_12

#define nRF24L01_PORT_CSN  GPIOA
#define nRF24L01_PIN_CSN   GPIO_Pin_4

#define nRF24L01_PORT_IRQ  GPIOB
#define nRF24L01_PIN_IRQ   GPIO_Pin_11


#elif defined PLATFORM_DEVICE_SANBOT_DONGLE

#define nRF24L01_DEBUG_SPI	"SPI1\n"
#define nRF24L01_SPI 		SPI1
#define nRF24L01_SPI1

#define nRF24L01_SPI_BaudRatePrescaler	SPI_BaudRatePrescaler_16

#define nRF24L01_DMA_DR_ADDR	SPI1_DR_Addr
#define nRF24L01_DMA_RX_ADDR	SPI1_Rx_Buff
#define nRF24L01_DMA_TX_ADDR	SPI1_Tx_Buff

#define nRF24L01_RCC_SPI  	RCC_APB2Periph_SPI1
#define nRF24L01_RCC_ALL  	(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB)

#define nRF24L01_PORT_CE  	GPIOB
#define nRF24L01_PIN_CE   	GPIO_Pin_1

#define nRF24L01_PORT_CSN 	GPIOB
#define nRF24L01_PIN_CSN  	GPIO_Pin_0

#define nRF24L01_PORT_IRQ 	GPIOA
#define nRF24L01_PIN_IRQ  	GPIO_Pin_4

#define nRF24L01_PORT_SPI	GPIOA
#define nRF24L01_PIN_SCK	GPIO_Pin_5
#define nRF24L01_PIN_MISO	GPIO_Pin_6
#define nRF24L01_PIN_MOSI	GPIO_Pin_7

#elif defined PLATFORM_DEVICE_SANBOT_REMOTE

#define nRF24L01_DEBUG_SPI	"SPI2\n"
#define nRF24L01_SPI 		SPI2
#define nRF24L01_SPI2

#define nRF24L01_SPI_BaudRatePrescaler	SPI_BaudRatePrescaler_16

#define nRF24L01_DMA_DR_ADDR	SPI2_DR_Addr
#define nRF24L01_DMA_RX_ADDR	SPI2_Rx_Buff
#define nRF24L01_DMA_TX_ADDR	SPI2_Tx_Buff

#define nRF24L01_RCC_SPI  RCC_APB1Periph_SPI2
#define nRF24L01_RCC_ALL  RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC

#define nRF24L01_PORT_SPI	GPIOB
#define nRF24L01_PIN_SCK	GPIO_Pin_13
#define nRF24L01_PIN_MISO	GPIO_Pin_14
#define nRF24L01_PIN_MOSI	GPIO_Pin_15

#define nRF24L01_PORT_CE   GPIOB
#define nRF24L01_PIN_CE    GPIO_Pin_5

#define nRF24L01_PORT_CSN  GPIOB
#define nRF24L01_PIN_CSN   GPIO_Pin_4

#define nRF24L01_PORT_IRQ  GPIOB
#define nRF24L01_PIN_IRQ   GPIO_Pin_12

#endif


#define nRF24L01_CSN_1()	GPIO_SetBits(nRF24L01_PORT_CSN, nRF24L01_PIN_CSN)
#define nRF24L01_CSN_0()	GPIO_ResetBits(nRF24L01_PORT_CSN, nRF24L01_PIN_CSN)
#define nRF24L01_CE_1()		GPIO_SetBits(nRF24L01_PORT_CE, nRF24L01_PIN_CE)
#define nRF24L01_CE_0()		GPIO_ResetBits(nRF24L01_PORT_CE, nRF24L01_PIN_CE)
#define nRF24L01_IRQ_Read()	  (nRF24L01_PORT_IRQ->IDR & nRF24L01_PIN_IRQ)

// #define nRF24L01_IRQ_EXTI_LINE                    EXTI_Line4
// #define nRF24L01_IRQ_EXTI_PORT_SOURCE             GPIO_PortSourceGPIOA
// #define nRF24L01_IRQ_EXTI_PIN_SOURCE              GPIO_PinSource4
// #define nRF24L01_IRQ_EDGE                         EXTI_Trigger_Falling
// #define nRF24L01_IRQ_EXTI_IRQn                    EXTI15_10_IRQn

// #define nRF24L01_IRQ_EXTI_PREEMPTION_PRIORITY     14
// #define nRF24L01_IRQ_EXTI_SUB_PRIORITY            0


/* 发射端和接收端地址 */
uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x34,0x43,0x10,0x10,0x00};
uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0x34,0x43,0x10,0x10,0x00};

uint8_t nRF_Address = 0;

uint8_t nRF_ReceiveBuffer[SBN1_PAYLOAD_WIDTH], nRF_SendBuffer[SBN1_PAYLOAD_WIDTH];

#define SPI_BUFFER_SIZE 512

uint8_t SPI1_Tx_Buff[SPI_BUFFER_SIZE];
uint8_t SPI1_Rx_Buff[SPI_BUFFER_SIZE];

uint8_t nrf24l01SetAddress(void)
{
	uint32_t temp0, temp1, temp2;
	uint8_t _id = TX_ADDRESS[TX_ADR_WIDTH - 1];

	if(_id == 0x00)
	{

		temp0=*(__IO u32*)(0x1FFFF7E8);
		temp1=*(__IO u32*)(0x1FFFF7EC);
		temp2=*(__IO u32*)(0x1FFFF7F0);
		_id += (u8)(temp0 & 0x000000FF);
		_id += (u8)((temp0 & 0x0000FF00)>>8);
		_id += (u8)((temp0 & 0x00FF0000)>>16);
		_id += (u8)((temp0 & 0xFF000000)>>24);
		_id += (u8)(temp1 & 0x000000FF);
		_id += (u8)((temp1 & 0x0000FF00)>>8);
		_id += (u8)((temp1 & 0x00FF0000)>>16);
		_id += (u8)((temp1 & 0xFF000000)>>24);
		_id += (u8)(temp2 & 0x000000FF);
		_id += (u8)((temp2 & 0x0000FF00)>>8);
		_id += (u8)((temp2 & 0x00FF0000)>>16);
		_id += (u8)((temp2 & 0xFF000000)>>24);

		if(_id == 0x00)
		{
			_id ++;
		}

		nRF_Address = _id;
		TX_ADDRESS[TX_ADR_WIDTH-1] = _id;
		RX_ADDRESS[RX_ADR_WIDTH-1] = _id;

	}

	DEBUG_PRINT("my address is [%02X]\n", _id);

	return _id;
}


// void nrf24l01DmaConfiguration(void)
// {
// 	DMA_InitTypeDef  DMA_InitStructure;


// 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);


// 	DMA_DeInit(DMA1_Channel4);
// 	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SPI1_DR_Addr;
// 	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SPI1_Rx_Buff;
// 	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
// 	DMA_InitStructure.DMA_BufferSize = SPI_BUFFER_SIZE;
// 	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
// 	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
// 	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
// 	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
// 	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
// 	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
// 	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
// 	DMA_Init(DMA1_Channel4, &DMA_InitStructure);


// 	DMA_DeInit(DMA1_Channel5);
// 	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SPI1_DR_Addr;
// 	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SPI1_Tx_Buff;
// 	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
// 	DMA_InitStructure.DMA_BufferSize = SPI_BUFFER_SIZE;
// 	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
// 	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
// 	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
// 	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
// 	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
// 	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
// 	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
// 	DMA_Init(DMA1_Channel5, &DMA_InitStructure);


// 	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);

// 	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);

// }

void nrf24l01SpiConfiguration(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* 配置SPI硬件参数 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* 数据方向：2线全双工 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* STM32的SPI工作模式 ：主机模式 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* 数据位长度 ： 8位 */

	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;

	SPI_InitStructure.SPI_BaudRatePrescaler = nRF24L01_SPI_BaudRatePrescaler;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* 数据位传输次序：高位先传 */
	SPI_InitStructure.SPI_CRCPolynomial = 7;			/* CRC多项式寄存器，复位后为7。本例程不用 */
	SPI_Init(nRF24L01_SPI, &SPI_InitStructure);

	SPI_Cmd(nRF24L01_SPI, ENABLE);				/* 使能SPI  */
}

// void EXTIX_Init(void)
// {

//  GPIOC->CRL&=0XFF0FFFFF;// PC5输入
//  GPIOC->CRL|=0X00800000;  //上拉/下拉输入模式

//  GPIOC->IDR|=1<<5;    //PC5默认上拉

//  Ex_NVIC_Config(GPIO_C,5,FTIR); //<b>由上升沿改为下降沿沿触发</b>

// MY_NVIC_Init(2,2,EXTI0_IRQChannel,2);    //抢占2，子优先级2，组2
// }

// void nrf24l01InterruptInit(void)
// {

// 	EXTI_InitTypeDef EXTI_InitStructure;

// 	GPIO_EXTILineConfig(nRF24L01_IRQ_EXTI_PORT_SOURCE, nRF24L01_IRQ_EXTI_PIN_SOURCE);

// 	/* Configure MPU9150int EXTI line */
// 	EXTI_InitStructure.EXTI_Line = nRF24L01_IRQ_EXTI_LINE;
// 	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
// 	EXTI_InitStructure.EXTI_Trigger = nRF24L01_IRQ_EDGE;
// 	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
// 	EXTI_Init(&EXTI_InitStructure);
  
// }

// void nrf24l01InterruptCmd(uint8_t NewState)
// {
//   NVIC_InitTypeDef NVIC_InitStructure;

//   /* Enable and set MPU9150int EXTI Interrupt priority */
//   NVIC_InitStructure.NVIC_IRQChannel = nRF24L01_IRQ_EXTI_IRQn;
//   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = nRF24L01_IRQ_EXTI_PREEMPTION_PRIORITY;

//   NVIC_InitStructure.NVIC_IRQChannelSubPriority = nRF24L01_IRQ_EXTI_SUB_PRIORITY;
//   NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
//   NVIC_Init(&NVIC_InitStructure);
// }

void nrf24l01Init(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;

#if defined nRF24L01_SPI1
	RCC_APB2PeriphClockCmd(nRF24L01_RCC_SPI, ENABLE);
#else
	RCC_APB1PeriphClockCmd(nRF24L01_RCC_SPI, ENABLE);
#endif
	
	RCC_APB2PeriphClockCmd(nRF24L01_RCC_ALL | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_InitStructure.GPIO_Pin = nRF24L01_PIN_CE;
	GPIO_Init(nRF24L01_PORT_CE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = nRF24L01_PIN_CSN;
	GPIO_Init(nRF24L01_PORT_CSN, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;

	GPIO_InitStructure.GPIO_Pin = nRF24L01_PIN_IRQ;
	GPIO_Init(nRF24L01_PORT_IRQ, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

	GPIO_InitStructure.GPIO_Pin = nRF24L01_PIN_SCK | nRF24L01_PIN_MOSI;
	GPIO_Init(nRF24L01_PORT_SPI, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

	GPIO_InitStructure.GPIO_Pin = nRF24L01_PIN_MISO;
	GPIO_Init(nRF24L01_PORT_SPI, &GPIO_InitStructure);

	/* 配置 SPI1工作模式 */
	nrf24l01SpiConfiguration();
	// nrf24l01Interrupt_Init();
	// nrf24l01Interrupt_Cmd(DISABLE);
	// nrf24l01DMA_Configuration();

	/* 用于拉高csn引脚，NRF进入空闲状态 */
	nRF24L01_CE_0();
	nRF24L01_CSN_1();

	DEBUG_PRINT("init successfully.\n");
}

// 向NRF读/写一字节数据

static uint8_t nrf24l01WriteReadByte(uint8_t dat)
{
	/* 当SPI发送缓冲器非空时等待 */
	while (SPI_I2S_GetFlagStatus(nRF24L01_SPI, SPI_I2S_FLAG_TXE) == RESET);

	/* 通过SPI发送一字节数据 */
	SPI_I2S_SendData(nRF24L01_SPI, dat);

	/* 当SPI接收缓冲器为空时等待 */
	while (SPI_I2S_GetFlagStatus(nRF24L01_SPI, SPI_I2S_FLAG_RXNE) == RESET);

	/* 通过SPI接收一字节数据 */
	return SPI_I2S_ReceiveData(nRF24L01_SPI);
}

// 用于向nRF24L01特定的寄存器写入数据

static uint8_t nrf24l01WriteReg(uint8_t _ucRegAddr, uint8_t _usValue)
{
	uint8_t ucStatus;

	nRF24L01_CE_0();

	/* 置低CSN，使能SPI传输 */
	nRF24L01_CSN_0();

	/* 发送命令及寄存器号 */
	ucStatus = nrf24l01WriteReadByte(_ucRegAddr);

	 /* 向寄存器写入数据 */
	nrf24l01WriteReadByte(_usValue);

	/* CSN拉高，完成 */
	nRF24L01_CSN_1();

	/* 返回状态寄存器的值 */
	return(ucStatus);
}

// 用于从nRF24L01特定的寄存器读出数据

static uint8_t nrf24l01ReadReg(uint8_t _ucRegAddr)
{
 	uint8_t usValue;

	nRF24L01_CE_0();

	/* 置低CSN，使能SPI传输 */
 	nRF24L01_CSN_0();

  	 /* 发送寄存器号 */
	nrf24l01WriteReadByte(_ucRegAddr);

	 /* 读取寄存器的值 */
	usValue = nrf24l01WriteReadByte(NOP);

   	/*CSN拉高，完成*/
	nRF24L01_CSN_1();

	return usValue;
}

// 用于从nRF24L01的寄存器中读出一串数据

static uint8_t nrf24l01ReadBuf(uint8_t _ucRegAddr, uint8_t *_pBuf, uint8_t _ucLen)
{
 	uint8_t ucStatus, i;

	nRF24L01_CE_0();

	/* 置低CSN，使能SPI传输 */
	nRF24L01_CSN_0();

	/*发送寄存器号*/
	ucStatus = nrf24l01WriteReadByte(_ucRegAddr);

 	/*读取缓冲区数据*/
	for(i = 0; i < _ucLen; i++)
	{
		_pBuf[i] = nrf24l01WriteReadByte(NOP);
	}

	/* CSN拉高，完成 */
	nRF24L01_CSN_1();

 	return ucStatus;
}

// 向nRF24L01的寄存器中写入一串数据

static uint8_t nrf24l01WriteBuf(uint8_t _ucRegAddr, uint8_t *_pBuf, uint8_t _ucLen)
{
	 uint8_t ucStatus,i;

	 nRF24L01_CE_0();

   	 /*置低CSN，使能SPI传输*/
	 nRF24L01_CSN_0();

	 /*发送寄存器号*/
  	 ucStatus = nrf24l01WriteReadByte(_ucRegAddr);

  	  /*向缓冲区写入数据*/
	 for(i = 0; i < _ucLen; i++)
	{
		nrf24l01WriteReadByte(*_pBuf++);
	}

	/*CSN拉高，完成*/
	nRF24L01_CSN_1();

  	return (ucStatus);
}

// 设置nRF24L01工作在接收模式

void nrf24l01RxMode(void)
{
	nRF24L01_CE_0();
	nrf24l01WriteBuf(NRF_WRITE_REG+RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);  /* 写RX节点地址 */
	nrf24l01WriteReg(NRF_WRITE_REG+EN_AA, 0x01);               			/* 使能通道0的自动应答 */
	nrf24l01WriteReg(NRF_WRITE_REG+EN_RXADDR, 0x01);           			/* 使能通道0的接收地址 */
	nrf24l01WriteReg(NRF_WRITE_REG+RF_CH, CHANNEL);             			/* 设置RF通信频率 */
	nrf24l01WriteReg(NRF_WRITE_REG+RX_PW_P0, RX_PLOAD_WIDTH);  			/* 选择通道0的有效数据宽度 */
	nrf24l01WriteReg(NRF_WRITE_REG+RF_SETUP, 0x0f); /* 设置TX发射参数,0db增益,2Mbps,低噪声增益开启 */
	nrf24l01WriteReg(NRF_WRITE_REG+CONFIG, 0x0f);   /* 配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 */

	/*CE拉高，进入接收模式*/
	nRF24L01_CE_1();
	delay_us(130);
}

void nrf24l01ChangeAddress(uint8_t Address)
{
	TX_ADDRESS[TX_ADR_WIDTH-1] = Address;
	RX_ADDRESS[RX_ADR_WIDTH-1] = Address;

	nRF24L01_CE_0();
	nrf24l01WriteBuf(NRF_WRITE_REG+TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);    /* 写TX节点地址 */
	nrf24l01WriteBuf(NRF_WRITE_REG+RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); /* 设置TX节点地址,主要为了使能ACK */
	nrf24l01WriteReg(NRF_WRITE_REG+EN_AA, 0x01);     /* 使能通道0的自动应答 */
	nrf24l01WriteReg(NRF_WRITE_REG+EN_RXADDR, 0x01); /* 使能通道0的接收地址 */
	nrf24l01WriteReg(NRF_WRITE_REG+SETUP_RETR, 0x05);/* 设置自动重发间隔时间:250us + 86us;最大自动重发次数:10次 */
	nrf24l01WriteReg(NRF_WRITE_REG+RF_CH, CHANNEL);   /* 设置RF通道为CHANAL */
	nrf24l01WriteReg(NRF_WRITE_REG+RF_SETUP, 0x0f);  /* 设置TX发射参数,0db增益,2Mbps,低噪声增益开启 */
	nrf24l01WriteReg(NRF_WRITE_REG+CONFIG, 0x0e);    /* 配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,发射模式,开启所有中断 */

	/*CE拉高，进入发送模式*/
	// nRF24L01_CE_1();
	// delay_us(100);  /* CE要拉高一段时间才进入发送模式，时间大于10us */
}

// 设置nRF24L01工作在发送模式

void nrf24l01TxMode(void)
{
	nRF24L01_CE_0();
	nrf24l01WriteBuf(NRF_WRITE_REG+TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);    /* 写TX节点地址 */
	nrf24l01WriteBuf(NRF_WRITE_REG+RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); /* 设置TX节点地址,主要为了使能ACK */
	nrf24l01WriteReg(NRF_WRITE_REG+EN_AA, 0x01);     /* 使能通道0的自动应答 */
	nrf24l01WriteReg(NRF_WRITE_REG+EN_RXADDR, 0x01); /* 使能通道0的接收地址 */
	nrf24l01WriteReg(NRF_WRITE_REG+SETUP_RETR, 0x05);/* 设置自动重发间隔时间:250us + 86us;最大自动重发次数:10次 */
	nrf24l01WriteReg(NRF_WRITE_REG+RF_CH, CHANNEL);   /* 设置RF通道为CHANAL */
	nrf24l01WriteReg(NRF_WRITE_REG+RF_SETUP, 0x0f);  /* 设置TX发射参数,0db增益,2Mbps,低噪声增益开启 */
	nrf24l01WriteReg(NRF_WRITE_REG+CONFIG, 0x0e);    /* 配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,发射模式,开启所有中断 */

	/*CE拉高，进入发送模式*/
	// nRF24L01_CE_1();
	// delay_us(100);  /* CE要拉高一段时间才进入发送模式，时间大于10us */
}

// Check

uint8_t nrf24l01ConnectCheck(void)
{
	uint8_t ucBuf[TX_ADR_WIDTH];

	// return 1;

	// for(i = 0; i < 255; i ++)
	// {
	// 	ucBuf[0] = 0xFF;
	// 	nrf24l01ReadBuf(TX_ADDR + i, ucBuf, 1);
	// 	if((ucBuf[0] != 0xFF) && (ucBuf[0] != 0x00))
	// 	{
	// 		break;
	// 	}
	// }

	// if(i<255)
	// {
	// 	return 1;
	// } else {
	// 	return 0;
	// }

	/*写入TX_ADR_WIDTH个字节的地址.  */
	nrf24l01WriteBuf(NRF_WRITE_REG+TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);

	/*读出写入的地址 */
	nrf24l01ReadBuf(TX_ADDR, ucBuf, TX_ADR_WIDTH);

	/* debug */
	// DEBUG_PRINT("ucBuf[0] = %d, TX_ADDRESS[0] = %d\r\n", ucBuf[0], TX_ADDRESS[0]);
	// DEBUG_PRINT("ucBuf[1] = %d, TX_ADDRESS[1] = %d\r\n", ucBuf[1], TX_ADDRESS[1]);
	// DEBUG_PRINT("ucBuf[2] = %d, TX_ADDRESS[2] = %d\r\n", ucBuf[2], TX_ADDRESS[2]);
	// DEBUG_PRINT("ucBuf[3] = %d, TX_ADDRESS[3] = %d\r\n", ucBuf[3], TX_ADDRESS[3]);
	// DEBUG_PRINT("ucBuf[4] = %d, TX_ADDRESS[4] = %d\r\n", ucBuf[4], TX_ADDRESS[4]);

	/* 比较写入的数据和读出的是否相同 */
	if(strncmp((char *)TX_ADDRESS, (char *)ucBuf, TX_ADR_WIDTH) == 0)
	{
		DEBUG_PRINT("connection success\n");
		return 1;  /* 相同 */
	}
	else
	{
		DEBUG_PRINT("connection failed\n");
		return 0;  /* 不相同 */
	}
}

// 向nRF24L01的发送缓冲区中写入数据

uint8_t nrf24l01TxData(uint8_t *_pTxBuf)
{
	uint8_t ucState = 0;
	uint32_t ulCount = 0;

	nrf24l01WriteReg(NRF_WRITE_REG+STATUS, ucState);
	nrf24l01WriteReg(FLUSH_TX,NOP);

	/*CE为低，进入待机模式1*/
	nRF24L01_CE_0();

	/*写数据到_pTxBuf最大32个字节*/
	nrf24l01WriteBuf(WR_TX_PLOAD, _pTxBuf, TX_PLOAD_WIDTH);

	/*CE为高，_pTxBuf非空，发送数据包 */
	nRF24L01_CE_1();

	delay_us(100);

	while((nRF24L01_IRQ_Read() != 0) && (ulCount < 4096))
	{
		// printf("ulCount - %d", ulCount);
		ulCount++;
	}

	if(ulCount >= 4096)
	{
		nrf24l01WriteReg(NRF_WRITE_REG+STATUS, ucState);
		nrf24l01WriteReg(FLUSH_TX,NOP);
		return 0;
	}

	/* 读取状态寄存器的值 */
	ucState = nrf24l01ReadReg(STATUS);

	/*清除TX_DS或MAX_RT中断标志*/
	nrf24l01WriteReg(NRF_WRITE_REG+STATUS, ucState);

	nrf24l01WriteReg(FLUSH_TX,NOP);      /* 清除TX FIFO寄存器 */

	 /*判断中断类型*/
	/* 达到最大重发次数 */
	if(ucState & MAX_RT)
	{
		return MAX_RT;
	}
	/* 发送完成 */
	else if(ucState & TX_DS)
	{
		return TX_DS;
	}
	/* 其他原因发送失败 */
	else
	{
		return 0;
	}
}

// 用于从nRF24L01的接收缓冲区中读出数据

uint8_t nrf24l01RxData(uint8_t *_pRxBuf)
{
	uint8_t ucState;
	uint32_t ulCount = 0;

	// nrf24l01ReadBuf(RD_RX_PLOAD, _pRxBuf, RX_PLOAD_WIDTH); /* 读取数据 */
	// nrf24l01WriteReg(FLUSH_RX, NOP); 

	nRF24L01_CE_1();	  /* 进入接收状态 */

	while((nRF24L01_IRQ_Read() != 0) && (ulCount < 4096))
	{
		// printf("ulCount - %d", ulCount);
		ulCount++;
	}

	if(ulCount >= 4096)
	{
		// nrf24l01ReadBuf(RD_RX_PLOAD, _pRxBuf, RX_PLOAD_WIDTH); /* 读取数据 */
		nrf24l01WriteReg(FLUSH_RX, NOP); 
		return 0;
	}

	nRF24L01_CE_0();  	 /* 进入待机状态 */

	/*读取status寄存器的值  */
	ucState = nrf24l01ReadReg(STATUS);

	/* 清除中断标志*/
	nrf24l01WriteReg(NRF_WRITE_REG+STATUS, ucState);

	/*判断是否接收到数据*/
	if(ucState & RX_DR)    /* 接收到数据 */
	{
		nrf24l01ReadBuf(RD_RX_PLOAD, _pRxBuf, RX_PLOAD_WIDTH); /* 读取数据 */
		nrf24l01WriteReg(FLUSH_RX, NOP);                       /* 清除RX FIFO寄存器 */
		return RX_DR;
	}
	else
    {
		return 0;   /* 没收到任何数据 */
	}
}

// void EXTI12_IRQHandler(void)
// {
//     if(EXTI_GetITStatus(EXTI_Line12) != RESET)
//     {
//         //Led_RW_OFF();
//         // GPIO_ResetBits(GPIOC, GPIO_Pin_4 | GPIO_Pin_5);
//         Set_LED(0,1);
//         delay_ms(100);
//         Set_LED(0,0);
//         /* Clear the EXTI line 9 pending bit */
//         EXTI_ClearITPendingBit(EXTI_Line12);
//     }
// }
