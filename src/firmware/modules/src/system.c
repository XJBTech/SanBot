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

#define DEBUG_MODULE "SYSTEM"

#include <stdbool.h>
#include <stdio.h>
#include "config.h"
#include "system.h"

#include "debug.h"
#include "rcc.h"
#include "led.h"

/* Private variable */
static uint8_t isInit;

// This must be the first module to be initialized!
void systemInit(void)
{
	if(isInit)
		return;

	rccInit();
	timerInit();

	isInit = 1;
}
