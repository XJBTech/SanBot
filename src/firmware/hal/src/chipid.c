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

#include "config.h"

#include "chipid.h"

uint8_t chipidGet(void)
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
		// TX_ADDRESS[TX_ADR_WIDTH-1] = _id;
		// RX_ADDRESS[RX_ADR_WIDTH-1] = _id;

		if(_id == 0x00)
		{
			_id ++;
		}

	}
	return _id;
}
