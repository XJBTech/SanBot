/*
 *  ____    ____    __   __   ______  ______
 * /\  _`\ /\  _`\ /\ \ /\ \ /\__  _\/\  _  \
 * \ \ \/\ \ \ \L\_\ `\`\/'/'\/_/\ \/\ \ \L\ \
 *  \ \ \ \ \ \  _\L`\/ > <     \ \ \ \ \  __ \
 *   \ \ \_\ \ \ \L\ \ \/'/\`\   \ \ \ \ \ \/\ \
 *    \ \____/\ \____/ /\_\\ \_\  \ \_\ \ \_\ \_\
 *     \/___/  \/___/  \/_/ \/_/   \/_/  \/_/\/_/
 *
 * Originally created by Dexta Robotics on Jul 29 2015.
 */

#include "sbn1.dexmo.h"

#ifndef __SBN1_DEXMO_DONGLE_H
#define __SBN1_DEXMO_DONGLE_H

/*==========  Export function  ==========*/

	void SBN1_USART_Handle_Reveived(void);
	void SBN1_Loop(void);

	extern uint8_t SBN1_Recv_Index;

#endif
