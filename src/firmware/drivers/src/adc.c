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

#define DEBUG_MODULE "ADC"

#include "config.h"

#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"

#include "debug.h"

#include "adc.h"

#define ADC_SampleTime 		ADC_SampleTime_13Cycles5
#define ADC1_DR_Address 	((u32)0x4001244C)

vu16 ADC_ConvertedValue[14];

void dmaInit()
{
	DMA_InitTypeDef DMA_InitStruct;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_DeInit(DMA1_Channel1);
	DMA_InitStruct.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryBaseAddr = (unsigned long)&ADC_ConvertedValue;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;

	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;

	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;

	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_Mode= DMA_Mode_Circular;

	DMA_InitStruct.DMA_Priority = DMA_Priority_High;
	DMA_InitStruct.DMA_BufferSize = 14;
	DMA_Init(DMA1_Channel1,&DMA_InitStruct);

	// Enable DMA1
	DMA_Cmd(DMA1_Channel1, ENABLE);

}


void adcInit()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

#ifdef PLATFORM_REV_DEXMO_A_RIGHT

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

#endif

#ifdef PLATFORM_REV_DEXMO_A_LEFT

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

#endif

	ADC_DeInit(ADC1);

	ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStruct.ADC_NbrOfChannel = 14;
	ADC_InitStruct.ADC_ScanConvMode = ENABLE;
	ADC_Init(ADC1, &ADC_InitStruct);

	ADC_TempSensorVrefintCmd(ENABLE);

/*
          |   |   |   |
          |4  |6  |8  |10
          |   |   |   |
  2 |    /-------------\
   1 \__| 3   5   7   9 nRF->
       0 \-------------/
 */

#ifdef PLATFORM_REV_DEXMO_A_RIGHT

	ADC_RegularChannelConfig(ADC1,  ADC_Channel_0,  1, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1,  ADC_Channel_2,  2, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1,  ADC_Channel_3,  3, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1,  ADC_Channel_4,  4, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1,  ADC_Channel_5,  5, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1,  ADC_Channel_6,  6, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1,  ADC_Channel_8,  7, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1,  ADC_Channel_9,  8, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10,  9, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 10, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 11, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 12, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 13, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 14, ADC_SampleTime);

#endif

#ifdef PLATFORM_REV_DEXMO_A_LEFT

	ADC_RegularChannelConfig(ADC1,  ADC_Channel_0,  1, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1,  ADC_Channel_1,  2, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1,  ADC_Channel_2,  3, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1,  ADC_Channel_3,  4, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1,  ADC_Channel_4,  5, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1,  ADC_Channel_5,  6, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1,  ADC_Channel_6,  7, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1,  ADC_Channel_7,  8, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10,  9, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 10, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 11, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 12, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 13, ADC_SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 14, ADC_SampleTime);

#endif

	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1,ENABLE);

	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	dmaInit();

	DEBUG_PRINT("init successfully\n");
}
