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

#define DEBUG_MODULE "PLATFORM"

/* Project includes */
#include "config.h"

/* ST includes */
#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

#include "rcc.h"

#include "debug.h"

// #include "adc.h"

#include "tim.h"

#include "nRF24L01.h"
#include "sbn1.sanbot_a.h"

#include "mpu9150.h"

#include "led.h"

void wakeupDriver()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE );
	RCC_APB2PeriphClockCmd ( RCC_APB2Periph_AFIO, ENABLE );

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init ( GPIOA, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_10;
	GPIO_Init ( GPIOB, &GPIO_InitStructure );

	GPIO_SetBits ( GPIOA, GPIO_Pin_1 );
	GPIO_SetBits ( GPIOB, GPIO_Pin_5 | GPIO_Pin_10 );
}

// TODO: Implement!
int platformInit ( void )
{
	uint8_t i = 0;
	int checksum = 0;

	//Low level init: Clock and Interrupt controller
	NVIC_PriorityGroupConfig ( NVIC_PriorityGroup_4 );

	// Disable the jtag gpio
	GPIO_PinRemapConfig ( GPIO_Remap_SWJ_JTAGDisable, ENABLE );

	ledInit();

	ledSet ( 0, 0 );
	ledSet ( 1, 0 );
	ledSet ( 2, 0 );

	delay_ms ( 500 );
	ledSet ( 0, 1 );
	delay_ms ( 500 );
	ledSet ( 1, 1 );
	delay_ms ( 500 );
	ledSet ( 2, 1 );

	delay_ms ( 500 );

	uartInit();

	DEBUG_PRINT ( "Too young too simple, sometimes naive.\n" );
	DEBUG_PRINT ( "I'm a journalist from Hongkong.\n" );
	DEBUG_PRINT ( "I could run very fast.\n" );
	DEBUG_PRINT ( "------------------------------\n" );

	DEBUG_PRINT ( "uart init successfully.\n" );

	ledSet ( 0, 0 );

	// adcInit();

	DEBUG_PRINT ( "test motor.\n" );

	timInit();

	DEBUG_PRINT ( "waking up driver.\n" );

	wakeupDriver();

	timSetPulse ( TIM2, 3, 0 );
	timSetPulse ( TIM3, 3, 0 );
	timSetPulse ( TIM4, 3, 0 );
	timSetPulse ( TIM2, 4, 0 );
	timSetPulse ( TIM3, 4, 0 );
	timSetPulse ( TIM4, 4, 0 );

	delay_ms ( 500 );

	timSetPulse ( TIM2, 4, 500 );
	timSetPulse ( TIM3, 4, 500 );
	timSetPulse ( TIM4, 4, 500 );

	delay_ms ( 500 );

	timSetPulse ( TIM2, 3, 0 );
	timSetPulse ( TIM3, 3, 0 );
	timSetPulse ( TIM4, 3, 0 );
	timSetPulse ( TIM2, 4, 0 );
	timSetPulse ( TIM3, 4, 0 );
	timSetPulse ( TIM4, 4, 0 );

	delay_ms ( 500 );

	timSetPulse ( TIM2, 3, 500 );
	timSetPulse ( TIM3, 3, 500 );
	timSetPulse ( TIM4, 3, 500 );

	delay_ms ( 500 );

	timSetPulse ( TIM2, 3, 0 );
	timSetPulse ( TIM3, 3, 0 );
	timSetPulse ( TIM4, 3, 0 );
	timSetPulse ( TIM2, 4, 0 );
	timSetPulse ( TIM3, 4, 0 );
	timSetPulse ( TIM4, 4, 0 );

	ledSet ( 1, 0 );

	delay_ms ( 200 );

	nrf24l01Init();

	i = nrf24l01ConnectCheck();

	nrf24l01SetAddress();
	// i = nrf24l01ConnectCheck();

	if ( i == 1 )
	{
		checksum --;
		ledSet ( 2, 0 );
	}

	delay_ms ( 1000 );

	// mpu9150Init();
	// i = mpu9150Status();

	// if(!i)
	// {
	//  checksum --;
	//  ledSet(1, 0);
	// }

	if ( checksum > 0 )
	{
		delay_ms ( 1000 );

		timSetPulse ( TIM2, 3, 0 );
		timSetPulse ( TIM3, 3, 0 );
		timSetPulse ( TIM4, 3, 0 );
		timSetPulse ( TIM2, 4, 0 );
		timSetPulse ( TIM3, 4, 0 );
		timSetPulse ( TIM4, 4, 0 );
		// timSetPulse(TIM2, 4, 999);
		// timSetPulse(TIM3, 4, 999);
		// timSetPulse(TIM4, 4, 999);

		while ( 1 )
		{

			ledSet ( 0, 1 );
			ledSet ( 1, 1 );
			ledSet ( 2, 1 );
			delay_ms ( 100 );
			ledSet ( 0, 0 );
			ledSet ( 1, 0 );
			ledSet ( 2, 0 );
			delay_ms ( 100 );
		}
	}

	ledSet ( 0, 0 );
	ledSet ( 1, 0 );
	ledSet ( 2, 0 );

	delay_ms ( 500 );

	ledSet ( 0, 1 );
	ledSet ( 1, 1 );
	ledSet ( 2, 1 );

	delay_ms ( 500 );

	ledSet ( 0, 0 );
	ledSet ( 1, 0 );
	ledSet ( 2, 0 );

	return 0;
}