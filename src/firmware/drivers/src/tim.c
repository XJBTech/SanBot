#define DEBUG_MODULE "TIM"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "config.h"

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"

#include "rcc.h"
#include "debug.h"

#include "tim.h"

#define PI 3.1415926f

uint8_t TIM_Lock_Bit = 0x00;

move_specfic task_queue[20];
int queue_top = -1;

void timInit ( void )
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE );
	RCC_APB2PeriphClockCmd ( RCC_APB2Periph_AFIO, ENABLE );
	RCC_APB2PeriphClockCmd ( RCC_APB2Periph_TIM1, ENABLE );
	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE );

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	// // TIM1 CH1/2/3
	// GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 |  GPIO_Pin_10;
	// GPIO_Init(GPIOA, &GPIO_InitStructure);

	// TIM2 CH3/4
	// GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	// GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_Init ( GPIOA, &GPIO_InitStructure );
	// GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);

	// TIM3 CH2/3/4
	// GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	// GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init ( GPIOB, &GPIO_InitStructure );
	// GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);

	// TIM4 CH1/2/3/4
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_Init ( GPIOB, &GPIO_InitStructure );

	GPIO_ResetBits ( GPIOA, GPIO_Pin_1 | GPIO_Pin_2 );
	GPIO_ResetBits ( GPIOB, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 );
	// GPIO_ResetBits(GPIOC, GPIO_Pin_8 | GPIO_Pin_9);

	// TIM_DeInit(TIM1);
	// TIM_InternalClockConfig(TIM1);
	TIM_DeInit ( TIM2 );
	TIM_InternalClockConfig ( TIM2 );
	TIM_DeInit ( TIM3 );
	TIM_InternalClockConfig ( TIM3 );
	TIM_DeInit ( TIM4 );
	TIM_InternalClockConfig ( TIM4 );

	TIM_TimeBaseStructInit ( &TIM_TimeBaseStructure );
	TIM_OCStructInit ( &TIM_OCInitStructure );

	//TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
	TIM_TimeBaseStructure.TIM_Period = 1024 - 1;
	//TIM_TimeBaseStructure.TIM_Prescaler = 36 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock / 1000000 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	// TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	TIM_TimeBaseInit ( TIM2, &TIM_TimeBaseStructure );
	TIM_TimeBaseInit ( TIM3, &TIM_TimeBaseStructure );
	TIM_TimeBaseInit ( TIM4, &TIM_TimeBaseStructure );

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;

	// TIM_OC2Init(TIM1, &TIM_OCInitStructure);

	// TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	// TIM_OC3Init(TIM1, &TIM_OCInitStructure);

	// TIM_OC2Init(TIM2, &TIM_OCInitStructure);
	// TIM_OC2Init(TIM3, &TIM_OCInitStructure);

	TIM_OC3Init ( TIM2, &TIM_OCInitStructure );
	TIM_OC4Init ( TIM2, &TIM_OCInitStructure );

	TIM_OC3Init ( TIM3, &TIM_OCInitStructure );
	TIM_OC4Init ( TIM3, &TIM_OCInitStructure );

	// TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	// TIM_OC2Init(TIM4, &TIM_OCInitStructure);

	TIM_OC3Init ( TIM4, &TIM_OCInitStructure );
	TIM_OC4Init ( TIM4, &TIM_OCInitStructure );

	// TIM_Cmd(TIM1, ENABLE);
	// TIM_CtrlPWMOutputs(TIM1, ENABLE);
	TIM_Cmd ( TIM2, ENABLE );
	TIM_CtrlPWMOutputs ( TIM2, ENABLE );
	TIM_Cmd ( TIM3, ENABLE );
	TIM_CtrlPWMOutputs ( TIM3, ENABLE );
	TIM_Cmd ( TIM4, ENABLE );
	TIM_CtrlPWMOutputs ( TIM4, ENABLE );

	DEBUG_PRINT ( "init successfully\n" );
}

uint8_t timSetPulse ( TIM_TypeDef * TIMx, uint8_t channel, uint16_t value )
{
	// TIM_OCInitTypeDef TIM_OCInitStructure;

	// TIM_Cmd(TIMx, DISABLE);
	// TIM_CtrlPWMOutputs(TIMx, DISABLE);

	// TIM_OCStructInit(&TIM_OCInitStructure);

	// TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	// TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	// TIM_OCInitStructure.TIM_Pulse = value;
	// TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;

	switch ( channel )
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
	return 0;

}

uint8_t timMapMartix()
{
	return 0;
}

uint8_t timApplyMartix ( uint16_t * _martix )
{
	timSetPulse ( TIM2, 3, 1023 - _martix[0] );
	timSetPulse ( TIM2, 4, 1023 - _martix[1] );
	timSetPulse ( TIM3, 3, 1023 - _martix[2] );
	timSetPulse ( TIM3, 4, 1023 - _martix[3] );
	timSetPulse ( TIM4, 3, 1023 - _martix[4] );
	timSetPulse ( TIM4, 4, 1023 - _martix[5] );

	return 0;

}

uint8_t timCalculate ( move_specfic * _import )
{
	float cal_temp[3];
	float sum = 0.0f;
	uint8_t i;

	// sum = 0.0f;

	for ( i = 0; i < 3; i++ )
	{
		// DEBUG_PRINT("tp1\n");
		cal_temp[i] = 1.0f * sinf ( ( *_import ).angle + 2.0f * PI / ( float ) TIM_MOTOR_QTY * ( float ) i + TIM_STARTANGLE );

		if(cal_temp[i] > 0.0f)
		{
			sum += cal_temp[i];
		} else {
			sum += -cal_temp[i];
		}

		// DEBUG_PRINT ( "ct = %f, angle = %d\n", cal_temp[i], ( uint16_t ) ( ( ( *_import ).angle + 2.0f * PI / ( float ) TIM_MOTOR_QTY * ( float ) i + TIM_STARTANGLE ) / PI * 180 ) );
		// DEBUG_PRINT("ct1 = %f \n", cal_temp[i]);

		// if ( cal_temp[i] > 1.0f )
		// {
		// 	cal_temp[i]  = 1.0f;
		// }

		// // DEBUG_PRINT("ct1 = %f \n", cal_temp[i]);

		// if ( cal_temp[i] < -1.0f )
		// {
		// 	cal_temp[i]  = -1.0f;
		// }
	}

	DEBUG_PRINT ( "cal_temp = [%f, %f, %f], sum = %f \n", cal_temp[0], cal_temp[1], cal_temp[2], sum );

	for(i = 0; i < 3; i++)
	{
		cal_temp[i] /= sum;
		// DEBUG_PRINT("ct1 = %f \n", cal_temp[i]);

		// time 1.5 for max output power
		cal_temp[i] *= ( float ) ( *_import ).velocity / 1024.0f * 1.5f;

		// DEBUG_PRINT("ct1 = %f \n", cal_temp[i]);

		cal_temp[i] += ( float ) ( *_import ).spin / 1024.0f;

		// DEBUG_PRINT("ct1 = %f \n", cal_temp[i]);

		if ( cal_temp[i] > 0.0f )
		{
			( *_import ).pulse[i * 2] = ( uint16_t ) ( cal_temp[i] * 1024.0f );
			( *_import ).pulse[i * 2 + 1] = 0;
		}
		else
		{
			( *_import ).pulse[i * 2] = 0;
			( *_import ).pulse[i * 2 + 1] = ( uint16_t ) ( - cal_temp[i] * 1024.0f );
		}
	}

	DEBUG_PRINT ( "cal_temp = [%f, %f, %f]\n", cal_temp[0], cal_temp[1], cal_temp[2] );

	return 0;
}

uint8_t timDebugMoveSpecfic ( move_specfic _import )
{
	DEBUG_PRINT ( "add queue v = %d, angle = %f, spin = %d, duration = %d ms, inner timer = %d, martix = [%d,%d,%d,%d,%d,%d]\n",
				  task_queue[queue_top].velocity, task_queue[queue_top].angle, task_queue[queue_top].spin, task_queue[queue_top].duration,
				  task_queue[queue_top].sysTimer,
				  task_queue[queue_top].pulse[0], task_queue[queue_top].pulse[1], task_queue[queue_top].pulse[2],
				  task_queue[queue_top].pulse[3], task_queue[queue_top].pulse[4], task_queue[queue_top].pulse[5] );
	return 0;
}

uint8_t timImportQueue ( move_specfic _import )
{
	// queue_top ++;
	memcpy ( & ( task_queue[queue_top] ), &_import, sizeof ( _import ) );
	task_queue[queue_top].sysTimer = 0;

	// timDebugMoveSpecfic ( task_queue[queue_top] );

	timCalculate ( &task_queue[queue_top] );

	timApplyMartix ( task_queue[queue_top].pulse );

	timDebugMoveSpecfic ( task_queue[queue_top] );

	return 0;
}

uint8_t timLoop ( void )
{
	// check timer

	// if reach the time

	// calculate the rotational speed

	// config the timer

	// reset the next timer target
	return 0;
}
