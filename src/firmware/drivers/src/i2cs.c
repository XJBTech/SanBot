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

#define DEBUG_MODULE "I2CS"

#include "config.h"

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

#include "rcc.h"
#include "debug.h"

#include "i2cs.h"

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2))
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr))
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))

#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08

#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //输出
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //输入

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //输出
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //输入

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //输出
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //输入

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //输出
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //输入

#if defined PLATFORM_DEVICE_SANBOT_A

#define I2CS_GPIO_RCC 	RCC_APB2Periph_GPIOC
#define I2CS_GPIO_PIN 	GPIO_Pin_10 | GPIO_Pin_11
#define I2CS_GPIO_PORT 	GPIOC

#define SDA_In()  {GPIOC->CRH&=0XFFFFF0FF;GPIOC->CRH|=8<<8;}
#define SDA_Out() {GPIOC->CRH&=0XFFFFF0FF;GPIOC->CRH|=3<<8;}

#define i2cs_SCL		PCout(11)
#define i2cs_SDA		PCout(10)
#define i2cs_Read_SDA	PCin(10)

#endif

void i2csInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(I2CS_GPIO_RCC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = I2CS_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(I2CS_GPIO_PORT, &GPIO_InitStructure);

}

uint8_t i2csStart(void)
{
	// DEBUG_PRINT("start\n");
	SDA_Out();
	i2cs_SDA=1;
	delay_us(1);
	i2cs_SCL=1;
	delay_us(5);
    if (!i2cs_Read_SDA)
    {
    	// DEBUG_PRINT("start fail 1\n");
        return 1;
    }
	i2cs_SDA=0;
	delay_us(5);
    if (i2cs_Read_SDA)
	{
		// DEBUG_PRINT("start fail 2\n");
		return 1;
	}
	i2cs_SCL=0;
	delay_us(2);
	return 0;
}

//停止i2cs总线
void i2csStop(void)
{
	SDA_Out();
	i2cs_SCL=0;
	i2cs_SDA=0;
	delay_us(4);
	i2cs_SCL=1;
	delay_us(5);
	i2cs_SDA=1;
	delay_us(4);
}

//等待Ack到来
//返回值：1，接收应答失败
//        0，接收应答成功
uint8_t i2csWaitAck(void)
{
	u16 ucErrTime=0;
	SDA_In();
	i2cs_SDA = 1; delay_us(1);
	i2cs_SCL = 1; delay_us(1);
	while(i2cs_Read_SDA)
	{
		ucErrTime++;
		if(ucErrTime>1000)
		{
			i2csStop();
			return 1;
		}
	}
	i2cs_SCL = 0;
	return 0;
}

//产生ACK应答
void i2csAck(void)
{
	i2cs_SCL=0;
	SDA_Out();
	i2cs_SDA=0;
	delay_us(2);
	i2cs_SCL=1;
	delay_us(2);
	i2cs_SCL=0;
}

//不产生ACK应答
void i2csNAck(void)
{
	i2cs_SCL=0;
	SDA_Out();
	i2cs_SDA=1;
	delay_us(2);
	i2cs_SCL=1;
	delay_us(2);
	i2cs_SCL=0;
}

//i2cs发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void i2csSendByte(uint8_t txd)
{
	u8 t;
	SDA_Out();
	i2cs_SCL=0;
	for(t=0;t<8;t++)
	{
		if((txd&0x80)>>7)
			i2cs_SDA=1;
		else
			i2cs_SDA=0;
		txd<<=1;
		delay_us(2);
		i2cs_SCL=1;
		delay_us(2);
		i2cs_SCL=0;
		delay_us(2);
	}
}

//i2cs读一个字节
//是否发送ack
//1，发送ACK
//0，发送nACK
uint8_t i2csReadByte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_In();
	for(i=0;i<8;i++ )
	{
		i2cs_SCL=0;
		delay_us(2);
		i2cs_SCL=1;
		receive<<=1;
		if(i2cs_Read_SDA)receive++;
		delay_us(1);
	}
	if (!ack)
	    i2csNAck();
	else
	    i2csAck();
	return receive;
}

uint8_t i2csWriteOneByte(uint8_t Address, uint8_t Register, uint8_t Data)
{
	if (i2csStart()) return 0;

	i2csSendByte(Address << 1);

	if (i2csWaitAck()) {
		i2csStop();
		return 0;
	}

	i2csSendByte(Register);
	i2csWaitAck();
	i2csSendByte(Data);
	i2csWaitAck();
	i2csStop();

	return 1;
}

uint8_t i2csProbing(uint8_t address)
{
	// i2cs_SDA = 0;
	// i2cs_SCL = 0;
	// delay_ms(100);
	if (i2csStart()){
		// DEBUG_PRINT("start failed\n");
		return 1;
	}

	i2csSendByte(address << 1);

	if (i2csWaitAck()) {
		i2csStop();
		// DEBUG_PRINT("no ack\n");
		return 1;
	}
	i2csStop();
	return 0;
}

uint8_t i2csReadOneByte(uint8_t Address, uint8_t Register)
{
	return 0;
}


uint8_t i2csWrite(uint8_t slave_addr,  uint8_t reg_addr, uint8_t length, uint8_t *data)
{
    uint8_t i;

    if (i2csStart()) return 1;

    // DEBUG_PRINT("started\r\n");

    i2csSendByte(slave_addr);

    if (i2csWaitAck()) {
        DEBUG_PRINT("no ack\r\n");
        i2csStop();
        return 1;
    }

    // DEBUG_PRINT(" I2C Write Reg [%02X]\r\n",reg_addr);

    i2csSendByte(reg_addr);
    i2csWaitAck();
    for(i = 0; i < length; i++)
    {
        i2csSendByte(*(data + i));
        // DEBUG_PRINT(" I2C Write [%02X]\r\n",*(data + i));
        i2csWaitAck();
    }
    i2csStop();

    return 0;
}

uint8_t i2csRead(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t *data)
{
    uint8_t i;

    if (i2csStart()) return 1;

    i2csSendByte(slave_addr);

    if (i2csWaitAck()) {
        i2csStop();
        return 1;
    }

    // DEBUG_PRINT(" I2C Read Reg [%02X]\r\n", reg_addr);
    i2csSendByte(reg_addr);
    i2csWaitAck();

    if (i2csStart()) return 1;

    i2csSendByte(slave_addr | 1);
    if (i2csWaitAck()) {
        i2csStop();
        return 1;
    }

    for(i = 0; i < length; i++)
    {
        if (i == length -1){
            *(data + i) = i2csReadByte(0);
            // DEBUG_PRINT(" I2C Read NACKed [%02X]\r\n",*(data + i));

        } else {
             *(data + i) = i2csReadByte(1);
            // DEBUG_PRINT(" I2C Read ACKed [%02X]\r\n",*(data + i));

        }
    }

    i2csStop();

    return 0;
}

