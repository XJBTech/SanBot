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

#include "sbn1.h"

#ifndef __SBN1_DEXMO_H
#define __SBN1_DEXMO_H

/*==========  Define device type  ==========*/

#ifdef PLATFORM_DEVICE_SANBOT_A

	#define SBN1_CONST_DEVICE			SBN1_CONST_SANBOT_A

#endif

/*==========  Operator  ==========*/

	// probe if dongle is connected
	#define SBN1_OP_PROBING_REQUEST 	0x01
	// empty
	#define SBN1_OP_PROBING_RESPONSE	0x02
	// Device 1byte | ID 12byte
	// TEST PACKET [53 42 4E 31 00 01 01 01]

	// search dexmo
	#define SBN1_OP_SEARCH_REQUEST		0x03
	// Switch 1byte
	// Switch Stop  0x00
	// 		  Start 0x01
	#define SBN1_OP_SEARCH_RESPONSE		0x04
	// Address 0x00 | Result 0x01=>success
	// Address 1byte | Device 1byte | ID 12byte

	// read dexmo status
	#define SBN1_OP_READ_REQUEST		0x05
	// Address 1byte | Mode 1byte
	// Mode Single     0x01
	//      Contineous 0x02
	//      Stop       0x03
	// nRF ------------------
	//     OP 1byte | ADC/MPU 1byte
	#define SBN1_OP_READ_RESPONSE		0x06
	// Address 1byte | 0 ADC/MPU X Result X Lock XXXXX | ADC 14*12/8=21byte | MPU 4*4=16byte
	// A/M = 0 => ADC
	// Result = 0 for error

	// self check
	#define SBN1_OP_SELFCHECK_REQUEST	0x09
	// Address 1byte
	// Dongle = 0x00
	#define SBN1_OP_SELFCEHCK_RESPONSE	0x0A
	// Address 1byte | Result 1byte
	// Result 0x00 error
	//        0x01 success

	// get device info
	#define SBN1_OP_INFO_REQUEST		0x0B
	// Address 1byte
	// Dongle = 0x00
	#define SBN1_OP_INFO_RESPONSE		0x0C
	// Address 1byte | Result 1byte | Info
	// Result 0x00 error
	//        0x01 success

    #define SBN1_OP_LED_REQUEST         0x0F
    // Address 1byte | 00000 Package x LED xx
    // package = 1 means led 0 will flash when processing package
    #define SBN1_OP_LED_RESPONSE        0x10
    // Address 1byte | Result 1byte | Info
    // Result 0x00 error
    //        0x01 success

#endif
