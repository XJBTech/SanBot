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

#define DEBUG_MODULE "SBN1"

#include <stdio.h>
#include <string.h>

#include "config.h"

#include "rcc.h"
#include "debug.h"
#include "adc.h"
#include "mpu9150.h"
#include "nrf24l01.h"
#include "led.h"

#include "sbn1.sanbot_a.h"

extern uint8_t nrf_led;

void sbn1ClearBuffer(uint8_t *_pBuf)
{
	memset(_pBuf, 0x00, TX_PLOAD_WIDTH * sizeof(uint8_t));
}

void sbn1PrintBuffer(uint8_t *_pBuf)
{
	uint8_t i;
	DEBUG_PRINT("Send Buffer - [ ");
	for(i = 0; i < SBN1_PAYLOAD_WIDTH; i++)
	{
		DEBUG_PRINT("%02X ", _pBuf[i]);
	}
	DEBUG_PRINT("]\r\n");
}

void sbn1HandleReceived(void)
{
	uint8_t _op = nRF_ReceiveBuffer[0x00];

	// DEBUG_PRINT("Recv Buffer - [ ");
	// for(i=0;i<SBN1_PAYLOAD_WIDTH;i++)
	// {
	// 	DEBUG_PRINT("%02X ", nRF_ReceiveBuffer[i]);
	// }
	// DEBUG_PRINT("]\r\n");

	switch(_op)
	{
		case SBN1_OP_SEARCH_REQUEST:
		{
			uint32_t temp0,temp1,temp2;

			sbn1ClearBuffer(nRF_SendBuffer);

			nRF_SendBuffer[0x00] = SBN1_OP_SEARCH_RESPONSE;
			nRF_SendBuffer[0x01] = nRF_Address;
			nRF_SendBuffer[0x02] = SBN1_CONST_DEVICE;


			temp0=*(__IO u32*)(0x1FFFF7E8);
			temp1=*(__IO u32*)(0x1FFFF7EC);
			temp2=*(__IO u32*)(0x1FFFF7F0);
			nRF_SendBuffer[0x03] = (u8)(temp0 & 0x000000FF);
			nRF_SendBuffer[0x04] = (u8)((temp0 & 0x0000FF00)>>8);
			nRF_SendBuffer[0x05] = (u8)((temp0 & 0x00FF0000)>>16);
			nRF_SendBuffer[0x06] = (u8)((temp0 & 0xFF000000)>>24);
			nRF_SendBuffer[0x07] = (u8)(temp1 & 0x000000FF);
			nRF_SendBuffer[0x08] = (u8)((temp1 & 0x0000FF00)>>8);
			nRF_SendBuffer[0x09] = (u8)((temp1 & 0x00FF0000)>>16);
			nRF_SendBuffer[0x0A] = (u8)((temp1 & 0xFF000000)>>24);
			nRF_SendBuffer[0x0B] = (u8)(temp2 & 0x000000FF);
			nRF_SendBuffer[0x0C] = (u8)((temp2 & 0x0000FF00)>>8);
			nRF_SendBuffer[0x0D] = (u8)((temp2 & 0x00FF0000)>>16);
			nRF_SendBuffer[0x0E] = (u8)((temp2 & 0xFF000000)>>24);

		    nrf24l01TxMode();
		    nrf24l01TxData(nRF_SendBuffer);

		    // sbn1PrintBuffer(nRF_SendBuffer);
			break;
		}

		case SBN1_OP_READ_REQUEST:
		{
			uint32_t i, pointer;
			uint8_t _buffer[42];

			// _AM = nRF_ReceiveBuffer[0x01];

			// DEBUG_PRINT("  Read A/M = %02X\r\n", _AM);

			sbn1ClearBuffer(nRF_SendBuffer);

			nRF_SendBuffer[0x00] = SBN1_OP_READ_RESPONSE;
			nRF_SendBuffer[0x01] = nRF_Address;
			nRF_SendBuffer[0x02] = 0x00;
			// nRF_SendBuffer[0x02] = TIM_Lock(0, 0);

			// ADC
			nRF_SendBuffer[0x03] = 0x40;

			pointer = 0x00;
			for(i = 0; i < 14; i++){
				_buffer[pointer++] = ADC_ConvertedValue[i] >> 8;
				_buffer[pointer++] = ADC_ConvertedValue[i] >> 4 & 0xF;
				_buffer[pointer++] = ADC_ConvertedValue[i] & 0xF;
			}
			for(i = 0; i < 21; i++)
			{
				nRF_SendBuffer[0x04 + i] = (_buffer[i*2] << 4) | _buffer[i*2+1];
			}

			nrf24l01TxMode();
			nrf24l01TxData(nRF_SendBuffer);
			// sbn1PrintBuffer(nRF_SendBuffer);

			// MPU
			nRF_SendBuffer[0x03] = 0x20;

			pointer = 0x04;
			for(i = 0; i < 4; i++){
				nRF_SendBuffer[pointer++] = mpu_data.quat[i] >> 24;
				nRF_SendBuffer[pointer++] = mpu_data.quat[i] >> 16 & 0xFF;
				nRF_SendBuffer[pointer++] = mpu_data.quat[i] >> 8 & 0xFF;
				nRF_SendBuffer[pointer++] = mpu_data.quat[i] & 0xFF;
			}
			// for(i = 0; i < 3; i++){
				// nRF_SendBuffer[pointer++] = compass[i];
			// }

			nrf24l01TxMode();
			nrf24l01TxData(nRF_SendBuffer);

			// sbn1PrintBuffer(nRF_SendBuffer);

			break;
		}

		case SBN1_OP_SELFCHECK_REQUEST:
		{
			// uint8_t _result = 0x01, _i;

			DEBUG_PRINT("  Self Check\r\n");

			sbn1ClearBuffer(nRF_SendBuffer);

			nRF_SendBuffer[0x00] = SBN1_OP_SELFCEHCK_RESPONSE;
			// nRF_SendBuffer[0x01] = Get_ChipID();

			// _result &= nrf24l01ConnectCheck();

			// DEBUG_PRINT("MPU_Address => %02X\r\n",MPU_Address);

			// if(!I2C_Write_One_Byte(MPU_Address, 0x00, 0x00))
			// {
			// 	_result = 0x00;
			// }

			// nRF_SendBuffer[0x02] = _result;

		    nrf24l01TxMode();
		    nrf24l01TxData(nRF_SendBuffer);

			// sbn1PrintBuffer(nRF_SendBuffer);

			break;
		}

		case SBN1_OP_LED_REQUEST:
		{
			uint8_t led = nRF_ReceiveBuffer[0x01];

			ledSet(0, led & 0x01);
			ledSet(1, (led >> 1) & 0x01);
			nrf_led = (led >> 2) & 0x01;

			sbn1ClearBuffer(nRF_SendBuffer);

			nRF_SendBuffer[0x00] = SBN1_OP_LED_RESPONSE;
			nRF_SendBuffer[0x01] = nRF_Address;
			nRF_SendBuffer[0x02] = 0x01;

		    nrf24l01TxMode();
		    nrf24l01TxData(nRF_SendBuffer);

			break;
		}
	}
}
