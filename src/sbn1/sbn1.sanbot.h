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

#ifdef PLATFORM_DEVICE_SANBOT_DONGLE

	#define SBN1_CONST_DEVICE			SBN1_CONST_SANBOT_DONGLE

#endif

/*==========  Operand  ==========*/

 	// common operand

 	#define SBN1_COMMON_OP_OFFSET		0x00

	// probe if dongle is connected
	#define SBN1_OP_PROBING_REQUEST 	(SBN1_COMMON_OP_OFFSET + 0x01)
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

	// self check
	#define SBN1_OP_SELFCHECK_REQUEST	0x05
	// Address 1byte
	// Dongle = 0x00
	#define SBN1_OP_SELFCEHCK_RESPONSE	0x06
	// Address 1byte | Result 1byte
	// Result 0x00 error
	//        0x01 success

	#define SBN1_OP_RESET_RESPONSE		0x07

 	// specfic operand

 	#define SBN1_SPECFIC_OP_OFFSET		0x80

	// read dexmo status
	#define SBN1_OP_MOVE_REQUEST		(SBN1_SPECFIC_OP_OFFSET + 0x01)
	// Address 1byte | angle float 4byte 0~2pi | velocity uint16 0~1024 | spin uint16 0~1024 | duration ms uint16
	#define SBN1_OP_MOVE_RESPONSE		(SBN1_SPECFIC_OP_OFFSET + 0x02)
	// Address 1byte | result 1byte

	// get device info
	#define SBN1_OP_INFO_REQUEST		0x05
	// Address 1byte
	// Dongle = 0x00
	#define SBN1_OP_INFO_RESPONSE		0x06
	// Address 1byte | Result 1byte | Info
	// Result 0x00 error
	//        0x01 success

    #define SBN1_OP_LED_REQUEST         0x07
    // Address 1byte | 00000 Package x LED xx
    // package = 1 means led 0 will flash when processing package
    #define SBN1_OP_LED_RESPONSE        0x08
    // Address 1byte | Result 1byte | Info
    // Result 0x00 error
    //        0x01 success

#endif
