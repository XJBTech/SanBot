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

#include "rcc.h"

#include "debug.h"

#include "nRF24L01.h"

#include "led.h"

// TODO: Implement!
int platformInit(void)
{
	uint8_t i = 0;

	//Low level init: Clock and Interrupt controller
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	// Disable the jtag gpio
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	ledInit();

	ledSet(0, 1);

	uartInit();

	nrf24l01Init();

	nrf24l01SetAddress();
	i = nrf24l01ConnectCheck();

	if(i)
	{
		ledSet(0, 0);
	}


	return 0;
}


