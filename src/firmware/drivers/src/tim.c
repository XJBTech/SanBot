#define DEBUG_MODULE "TIM"

#include "config.h"

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"

#include "debug.h"

uint8_t TIM_Lock_Bit = 0x00;

void timInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	// // TIM1 CH1/2/3
	// GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 |  GPIO_Pin_10;
	// GPIO_Init(GPIOA, &GPIO_InitStructure);

	// TIM2 CH3/4
	// GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	// GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);

	// TIM3 CH2/3/4
	// GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	// GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);

	// TIM4 CH1/2/3/4
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOA, GPIO_Pin_1 | GPIO_Pin_2);
	GPIO_ResetBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9);
	// GPIO_ResetBits(GPIOC, GPIO_Pin_8 | GPIO_Pin_9);

	// TIM_DeInit(TIM1);
	// TIM_InternalClockConfig(TIM1);
	TIM_DeInit(TIM2);
	TIM_InternalClockConfig(TIM2);
	TIM_DeInit(TIM3);
	TIM_InternalClockConfig(TIM3);
	TIM_DeInit(TIM4);
	TIM_InternalClockConfig(TIM4);

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_OCStructInit(&TIM_OCInitStructure);

	//TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
	//TIM_TimeBaseStructure.TIM_Prescaler = 36 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock / 1000000 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	// TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;

	// TIM_OC2Init(TIM1, &TIM_OCInitStructure);

	// TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	// TIM_OC3Init(TIM1, &TIM_OCInitStructure);

	// TIM_OC2Init(TIM2, &TIM_OCInitStructure);
	// TIM_OC2Init(TIM3, &TIM_OCInitStructure);

	TIM_OC3Init(TIM2, &TIM_OCInitStructure);
	TIM_OC4Init(TIM2, &TIM_OCInitStructure);

	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);

	// TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	// TIM_OC2Init(TIM4, &TIM_OCInitStructure);

	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
	TIM_OC4Init(TIM4, &TIM_OCInitStructure);

	// TIM_Cmd(TIM1, ENABLE);
	// TIM_CtrlPWMOutputs(TIM1, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
	TIM_CtrlPWMOutputs(TIM2, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
	TIM_CtrlPWMOutputs(TIM3, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
	TIM_CtrlPWMOutputs(TIM4, ENABLE);

	DEBUG_PRINT("init successfully\n");
}

uint8_t timSetPulse(TIM_TypeDef* TIMx, uint8_t channel, uint16_t value)
{
	TIM_OCInitTypeDef TIM_OCInitStructure;

	// TIM_Cmd(TIMx, DISABLE);
	// TIM_CtrlPWMOutputs(TIMx, DISABLE);

	TIM_OCStructInit(&TIM_OCInitStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = value;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;

	switch(channel)
	{
		case 1:
			TIMx->CCR1 = value;
			// TIM_OC1Init(TIMx, &TIM_OCInitStructure);
			break;
		case 2:
			TIMx->CCR2 = value;
			// TIM_OC2Init(TIMx, &TIM_OCInitStructure);
			break;
		case 3:
			TIMx->CCR3 = value;
			// TIM_OC3Init(TIMx, &TIM_OCInitStructure);
			break;
		case 4:
			TIMx->CCR4 = value;
			// TIM_OC4Init(TIMx, &TIM_OCInitStructure);
			break;
	};

	// TIM_Cmd(TIMx, ENABLE);
	// TIM_CtrlPWMOutputs(TIMx, ENABLE);
}

uint8_t TIM_Lock(uint8_t _RW, uint8_t _bit)
{
	if(_RW == 1)
	{
		TIM_Lock_Bit = _bit;
	}

	// printf("Lock[ ");

	// if(TIM_Lock_Bit & 0x01)
	// {
	// 	printf("On ");
	// } else {
	// 	printf("Off ");
	// }

	// if(TIM_Lock_Bit & 0x02)
	// {
	// 	printf("On ");
	// } else {
	// 	printf("Off ");
	// }

	// if(TIM_Lock_Bit & 0x04)
	// {
	// 	printf("On ");
	// } else {
	// 	printf("Off ");
	// }

	// if(TIM_Lock_Bit & 0x08)
	// {
	// 	printf("On ");
	// } else {
	// 	printf("Off ");
	// }

	// if(TIM_Lock_Bit & 0x10)
	// {
	// 	printf("On ");
	// } else {
	// 	printf("Off ");
	// }

	// printf("]\r\n");

	return TIM_Lock_Bit;
}
