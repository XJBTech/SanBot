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
#include <math.h>

#include "config.h"

#include "rcc.h"
#include "debug.h"
#include "adc.h"
#include "mpu9150.h"
#include "nrf24l01.h"
#include "led.h"

#include "sbn1.sanbot_a.h"

#define M_PI_F ((float) M_PI)

extern uint8_t nrf_led;

void sbn1ClearBuffer ( uint8_t * _pBuf )
{
	memset ( _pBuf, 0x00, TX_PLOAD_WIDTH * sizeof ( uint8_t ) );
}

void sbn1PrintBuffer ( uint8_t * _pBuf )
{
	uint8_t i;
	DEBUG_PRINT ( "Send Buffer - [ " );

	for ( i = 0; i < SBN1_PAYLOAD_WIDTH; i++ )
	{
		DEBUG_PRINT ( "%02X ", _pBuf[i] );
	}

	DEBUG_PRINT ( "]\r\n" );
}

void sbn1HandleReceived ( void )
{

	uint32_t i, v;
	// uint8_t _buffer[42];
	unsigned char * p;

	float q0;
	float q1;
	float q2;
	float q3;

	float yaw , pitch, roll;
	float gx, gy, gz; // estimated gravity direction

	q1 = ( float ) ( mpu_data.quat[0] ) / 1073741824.0f;
	q2 = ( float ) ( mpu_data.quat[1] ) / 1073741824.0f;
	q3 = ( float ) ( mpu_data.quat[2] ) / 1073741824.0f;
	q0 = ( float ) ( mpu_data.quat[3] ) / 1073741824.0f;

	gx = 2 * ( q1 * q3 - q0 * q2 );
	gy = 2 * ( q0 * q1 + q2 * q3 );
	gz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

	// if ( gx > 1 )
	// {
	//  gx = 1;
	// }

	// if ( gx < -1 )
	// {
	//  gx = -1;
	// }

	yaw = atan2f ( 2 * ( q0 * q3 + q1 * q2 ), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3 ) * 180 / M_PI_F;
	pitch = asinf ( gx ) * 180 / M_PI_F; //Pitch seems to be inverted
	roll = atan2f ( gy, gz ) * 180 / M_PI_F;

	// DEBUG_PRINT ( "Q => [%f,%f,%f,%f]\n", q0, q1, q2, q3 );

	// DEBUG_PRINT ( "yaw, pitch, roll => [%10.5f, %10.5f, %10.5f], gx = %f\n", yaw, pitch, roll, gx );

	// _AM = nRF_ReceiveBuffer[0x01];

	// DEBUG_PRINT("  Read A/M = %02X\r\n", _AM);

	sbn1ClearBuffer ( nRF_SendBuffer );

	nRF_SendBuffer[0x00] = 0x81;
	nRF_SendBuffer[0x01] = 0x58;

	/// Cartesian coordinate System
	//ea.m_fRoll  = atan2(2 * (w * x + y * z) , 1 - 2 * (x * x + y * y));
	//ea.m_fPitch = asin(2 * (w * y - z * x));
	//ea.m_fYaw   = atan2(2 * (w * z + x * y) , 1 - 2 * (y * y + z * z));


	roll += 180.0f;
	roll = roll / 180.0f * M_PI_F;
	p = ( unsigned char * ) ( &roll );

	for ( i = 0 ; i < 4; i++ )
	{
		nRF_SendBuffer[0x02 + i] = p[i];
	}

	if ( yaw < 0.0f )
	{
		yaw = -yaw;
	}

	v = ( int ) ( yaw / 180.0f * 1024.0f );

	nRF_SendBuffer[0x06] = v >> 8;
	nRF_SendBuffer[0x07] = v & 0xFF;

	DEBUG_PRINT ( "v = %d, angle = %f, payload = [%02X %02X %02X %02X %02X %02X]\n",
				  v, roll, nRF_SendBuffer[0x02], nRF_SendBuffer[0x03], nRF_SendBuffer[0x04], nRF_SendBuffer[0x05], nRF_SendBuffer[0x06], nRF_SendBuffer[0x07] );

	// 53 42 4E 31 01 0C C1 81 41 00 00 00 00 00 FF 00 00 00 00

	// MPU
	// nRF_SendBuffer[0x03] = 0x20;

	// pointer = 0x04;

	// for ( i = 0; i < 4; i++ )
	// {
	//  nRF_SendBuffer[pointer++] = mpu_data.quat[i] >> 24;
	//  nRF_SendBuffer[pointer++] = mpu_data.quat[i] >> 16 & 0xFF;
	//  nRF_SendBuffer[pointer++] = mpu_data.quat[i] >> 8 & 0xFF;
	//  nRF_SendBuffer[pointer++] = mpu_data.quat[i] & 0xFF;
	// }

	// for(i = 0; i < 3; i++){
	// nRF_SendBuffer[pointer++] = compass[i];
	// }

	nrf24l01ChangeAddress ( 0x58 );
	nrf24l01TxData ( nRF_SendBuffer );

	// sbn1PrintBuffer(nRF_SendBuffer);

}
