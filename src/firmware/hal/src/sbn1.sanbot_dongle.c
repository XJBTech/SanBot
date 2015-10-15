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

#include "sbn1.sanbot_dongle.h"

/*==========  Debug switch  ==========*/


//#define SBN1_DEBUG

#ifdef SBN1_DEBUG

	typedef uint32_t  u32;
	typedef uint16_t u16;
	typedef uint8_t  u8;

	extern uint8_t Debug_Buffer[100];

#else

	// UART struct => UART.h
	extern UART_T g_tUart1;

#endif

/*==========  Send & Recv packet index  ==========*/

uint8_t SBN1_Send_Index = 0x00, SBN1_Recv_Index = 0x00;

/*==========  Buffer defination  ==========*/

#define SBN1_BUFSIZ 40

// Temp buffer
uint8_t Temp_Buffer[SBN1_BUFSIZ];

// Recv Buffer top pointer
uint16_t Recv_Pointer = 0x00;

uint8_t Recv_Buffer[1024],  // Recv buffer
		Send_Buffer[8],     // Send parameter buffer
		Packet_Buffer[SBN1_BUFSIZ]; // Pacoet sendout buffer

/*==========  Task list  ==========*/

#define SBN1_TASK_BUFFER 5

// Task struct
typedef struct
{

	// Task id
	uint16_t _id;

	// decide whether be processed
	uint8_t _enable;

	// Function pointer
	void ( *_action ) ( uint8_t * _register );

	// parameter register
	uint8_t _register[5];

} SBN1_Task_Item;

// task buffer
SBN1_Task_Item SBN1_Task[SBN1_TASK_BUFFER];

// Task list top pointer
uint8_t SBN1_Task_Pointer = 0x00;

/*==========  Function declaration  ==========*/

void SBN1_Clear_Buffer ( uint8_t * _pBuf, uint8_t _length );
void SBN1_Print_Buffer ( uint8_t * _pBuf, uint8_t _length );

void SBN1_USART_Recv ( void );
void SBN1_USART_Send ( uint8_t * _buffer, uint8_t _length );
void SBN1_USART_Handle_Reveived ( void );
void SBN1_USART_Send_Packet ( uint8_t _payload[], uint8_t _length );
void SBN1_USART_Handle_Packet ( uint8_t _payload[], uint8_t _length );

uint8_t SBN1_Create_Task ( uint16_t _id,
						   void ( *_action ) ( uint8_t * _register ), uint8_t _enable, ... );
uint8_t SBN1_Revoke_Task ( uint16_t _id );
void SBN1_Loop ( void );
void SBN1_Func_Probing ( uint8_t * _register );
void SBN1_Func_Search ( uint8_t * _register );
void SBN1_Func_Read ( uint8_t * _register );
void SBN1_Func_Lock ( uint8_t * _register );
void SBN1_Func_SelfCheck ( uint8_t * _register );

/*==========  Buffer  ==========*/

// Clean Buffer
// @param *uint8_t BufferPointer
// @param uint8_t size
void SBN1_Clear_Buffer ( uint8_t * _pBuf, uint8_t _length )
{
	memset ( _pBuf, 0x00, _length );
}

// Print Buffer to system
// @param *uint8_t BufferPointer
// @param uint8_t size
void SBN1_Print_Buffer ( uint8_t * _pBuf, uint8_t _length )
{
	uint8_t i;

	for ( i = 0; i < _length; i++ )
	{
		printf ( "%02X,", _pBuf[i] );
	}

	printf ( "\r\n" );
}

/*==========  USART  ==========*/

// Receive data from USART FIFO
void SBN1_USART_Recv ( void )
{
	#ifdef SBN1_DEBUG
	uint8_t i;
	Recv_Pointer = Debug_Buffer[0x00];

	for ( i = 0; i < Recv_Pointer; i++ )
	{
		Recv_Buffer[i] = Debug_Buffer[i + 1];
	}

	#else

	while ( UartGetChar ( &g_tUart1, &Recv_Buffer[Recv_Pointer] ) &&
			Recv_Pointer < 1024 )
	{
		Recv_Pointer++;
	};

	if ( Recv_Pointer == 1024 )
	{
		Recv_Pointer = 0;
	}

	#endif
}

// Send data through USART
// @param *uint8_t BufferPointer
// @param uint8_t size
void SBN1_USART_Send ( uint8_t * _buffer, uint8_t _length )
{
	#ifdef SBN1_DEBUG
	uint8_t i;

	for ( i = 0; i < _length; i++ )
	{
		printf ( "%02X ", _buffer[i] );
	}

	#else
	UartSend ( &g_tUart1, _buffer, _length );
	#endif
}

// Handle data read from USART
void SBN1_USART_Handle_Reveived ( void )
{
	uint16_t i = 0, _i = 0;
	uint8_t temp;
	// uint8_t _length, _index;

	SBN1_USART_Recv();

	// printf("%d,Buffer=",Recv_Pointer);
	// for(i=0;i<Recv_Pointer;i++)
	// {
	//  printf("%02X ", Recv_Buffer[i]);
	// }
	// printf("\r\n");

	if ( Recv_Pointer > 7 )
	{
		for ( i = 0; i <= Recv_Pointer - 4; )
		{
			if ( Recv_Buffer[i++] != 'S' || Recv_Buffer[i++] != 'B' || Recv_Buffer[i++] != 'N' || Recv_Buffer[i++] != '1' )
			{
				// we didn't get a full header
				continue;
			}
			else
			{
				i -= 4;

				for ( temp = i; temp < Recv_Pointer; temp ++ )
				{
					Recv_Buffer[temp - i] = Recv_Buffer[temp];
				}

				Recv_Pointer -= i;
				break;
			}
		}
	}

	i = 0;

	while ( i < Recv_Pointer && Recv_Pointer > 7 )
	{
		uint8_t index = 0x00, length = 0x00, checksum = 0x00, _checksum;

		// printf("%d,Start Processing, Buffer=",Recv_Pointer);
		// for(temp=0;temp<Recv_Pointer;temp++)
		// {
		//  printf("%02X ", Recv_Buffer[temp]);
		// }
		// printf("\r\n");

		if ( Recv_Buffer[i++] != 'S' || Recv_Buffer[i++] != 'B' || Recv_Buffer[i++] != 'N' || Recv_Buffer[i++] != '1' )
		{
			// we didn't get a full header
			continue;
		}

		_i = i;

		// return when there is no index or chksum
		if ( _i + 2 > Recv_Pointer )
		{
			return;
		}

		// get index from buffer
		index = Recv_Buffer[i];

		// printf("Index  = %02X", index);
		if ( index != SBN1_Recv_Index )
		{
			// incorrect index, clear header
			SBN1_Recv_Index = index + 1;
			// Recv_Pointer = 0;
			memset ( Recv_Buffer, 0x00, _i );
			// printf(" - Incorrect\r\n");
			continue;
		}
		else
		{
			// correct index, updating
			// printf(" - Correct\r\n");

		}

		length = Recv_Buffer[i + 1];

		// printf("Length = %02X", length);
		if ( length + i + 3 > Recv_Pointer )
		{
			// printf(" - Incorrect\r\n");

			// printf("%d,Buffer=",Recv_Pointer);
			// for(temp=0;temp<Recv_Pointer;temp++)
			// {
			//  printf("%02X ", Recv_Buffer[temp]);
			// }
			// printf("\r\n");
			continue;
		}

		// printf(" - Correct\r\n");

		// get checksum from i + 2,  i (should) == 4
		checksum = Recv_Buffer[i + 2];
		_checksum = 0x00;
		i += 3;
		memset ( Packet_Buffer, 0x00, sizeof ( Packet_Buffer ) );

		for ( temp = 0; length > 0; length--, temp++ )
		{
			Packet_Buffer[temp] = Recv_Buffer[i];
			_checksum += Recv_Buffer[i];
			i++;
		}

		if ( checksum == _checksum )
		{
			SBN1_USART_Handle_Packet ( Packet_Buffer, temp );

			SBN1_Recv_Index ++;

			// Recv_Pointer = 0;

			for ( temp = i; temp < Recv_Pointer; temp ++ )
			{
				Recv_Buffer[temp - i] = Recv_Buffer[temp];
			}

			Recv_Pointer -= i;

		}
		else
		{
			memset ( Recv_Buffer, 0x00, _i );
			i = _i;
		}
	}
}

// Send packet
// @param uint8_t operator
// @param *uint8_t payload
// @param uint8_t size
void SBN1_USART_Send_Packet ( uint8_t _payload[], uint8_t _length )
{
	uint8_t _checksum = 0, i;

	for ( i = 0; i < _length; i++ )
	{
		_checksum += _payload[i];
	}

	// Send_Buffer[0x00] = 'S';
	// Send_Buffer[0x01] = 'B';
	// Send_Buffer[0x02] = 'N';
	// Send_Buffer[0x03] = '1';
	strncpy ( ( char * ) Send_Buffer, "SBN1", 4 );

	Send_Buffer[0x04] = SBN1_Send_Index ++;
	Send_Buffer[0x05] = _length;
	Send_Buffer[0x06] = _checksum;

	SBN1_USART_Send ( Send_Buffer, 7 );
	SBN1_USART_Send ( _payload, _length );
}

// Handle / Decode packet
// @param *uint8_t payload
// @param uint8_t size
void SBN1_USART_Handle_Packet ( uint8_t _payload[], uint8_t _length )
{
	// debug
	// uint8_t i;
	// printf("Handle Package\r\n");
	// printf("Length = %d [ ", _length);
	// for(i=0;i<_length;i++)
	// {
	//  printf("%02X ", _payload[i]);
	// }
	// printf("]\r\n");
	// debug

	switch ( _payload[0x00] )
	{
		case SBN1_OP_PROBING_REQUEST:
		{
			SBN1_Func_Probing ( 0 );
			break;
		}

		case SBN1_OP_SEARCH_REQUEST:
		{
			SBN1_Clear_Buffer ( Temp_Buffer, SBN1_BUFSIZ );

			SBN1_Revoke_Task ( 0x0101 );
			SBN1_Create_Task ( 0x0101, SBN1_Func_Search, _payload[0x01], 0x01, 0x01 );

			Temp_Buffer[0x00] = SBN1_OP_SEARCH_RESPONSE;
			Temp_Buffer[0x01] = 0x00;
			Temp_Buffer[0x02] = 0x01;

			SBN1_USART_Send_Packet ( Temp_Buffer, 0x03 );

			break;
		}

		case SBN1_OP_MOVE_REQUEST:
		{
			uint8_t i;

			DEBUG_PRINT ( "received move request to address [%02X]\n", _payload[0x01] );

			SBN1_Clear_Buffer ( Temp_Buffer, SBN1_BUFSIZ );
			// SBN1_Clear_Buffer ( nRF_SendBuffer, SBN1_BUFSIZ );

			Temp_Buffer[0x00] = SBN1_OP_MOVE_RESPONSE;

			nrf24l01ChangeAddress ( _payload[0x01] );

			for ( i = 0; i < _length; i++ )
			{
				nRF_SendBuffer[i] = _payload[i];
			}

			i = nrf24l01TxData ( nRF_SendBuffer );

			// printf("RX_DR = %02X\r\n", i);
			if ( i == TX_DS )
			{
				// // printf("TX_DS\r\n");
				// nRF24L01_RxMode();

				// for ( t = 0; t < 3 && i != RX_DR; t++ )
				// {
				//  i = nRF24L01_RxData ( nRF_ReceiveBuffer );
				// }

				// // printf("RX_DR = %02X\r\n", i);
				// if ( i == RX_DR && nRF_ReceiveBuffer[0x00] == SBN1_OP_LED_RESPONSE )
				// {

				//  // break;
				// }
				Temp_Buffer[0x01] = 0x01;

				DEBUG_PRINT ( "send successfully\n" );
			}

			SBN1_USART_Send_Packet ( Temp_Buffer, 0x03 );

			break;

		}

			// case SBN1_OP_READ_REQUEST:
			// {
			//  uint8_t _address = _payload[0x01], _mode = _payload[0x02];

			//  switch ( _mode )
			//  {
			//      // single
			//      case 0x01:
			//      {
			//          uint8_t temp[1] = {0x00};
			//          temp[0] = _address;

			//          #ifdef SBN1_DEBUG
			//          printf ( "  Single read from %02X\r\n", _address );
			//          #endif
			//          SBN1_Func_Read ( temp );
			//          break;
			//      }

			//      // start contineous
			//      case 0x02:
			//      {
			//          #ifdef SBN1_DEBUG
			//          printf ( "  Start contineous read from %02X\r\n", _address );
			//          #endif

			//          SBN1_Revoke_Task ( 0x0100 | _address );
			//          SBN1_Create_Task ( 0x0100 | _address, SBN1_Func_Read, 0x01, 0x01, _address );

			//          break;
			//      }

			//      // stop
			//      case 0x03:
			//      {
			//          #ifdef SBN1_DEBUG
			//          printf ( "  Stop read from %02X\r\n", _address );
			//          #endif

			//          SBN1_Revoke_Task ( 0x0100 | _address );

			//          break;
			//      }
			//  }

			//  SBN1_Clear_Buffer ( Temp_Buffer, SBN1_BUFSIZ );

			//  Temp_Buffer[0x00] = SBN1_OP_READ_RESPONSE;
			//  Temp_Buffer[0x01] = _address;
			//  Temp_Buffer[0x02] = 0x80;

			//  SBN1_USART_Send_Packet ( Temp_Buffer, 0x03 );

			//  break;
			// }

			// case SBN1_OP_LOCK_REQUEST:
			// {
			//  uint8_t temp[2];
			//  temp[0] = _payload[0x01];
			//  temp[1] = _payload[0x02];

			//  SBN1_Func_Lock ( temp );

			//  break;
			// }

			// case SBN1_OP_SELFCHECK_REQUEST:
			// {
			//  uint8_t temp[1] = {0x00};
			//  temp[0] = _payload[0x01];
			//  SBN1_Func_SelfCheck ( temp );

			//  break;
			// }

			// case SBN1_OP_LED_REQUEST:
			// {
			//  uint8_t i, t;
			//  SBN1_Clear_Buffer ( Temp_Buffer, SBN1_BUFSIZ );

			//  nRF24L01_ChangeAddress ( _payload[0x01] );
			//  nRF_SendBuffer[0] = SBN1_OP_LED_REQUEST;
			//  nRF_SendBuffer[1] = _payload[0x02];

			//  for ( t = 0; t < 3 && i != TX_DS; t++ )
			//  {
			//      i = nRF24L01_TxData ( nRF_SendBuffer );
			//  }

			//  // printf("RX_DR = %02X\r\n", i);

			//  if ( i == TX_DS )
			//  {
			//      // printf("TX_DS\r\n");
			//      nRF24L01_RxMode();

			//      for ( t = 0; t < 3 && i != RX_DR; t++ )
			//      {
			//          i = nRF24L01_RxData ( nRF_ReceiveBuffer );
			//      }

			//      // printf("RX_DR = %02X\r\n", i);
			//      if ( i == RX_DR && nRF_ReceiveBuffer[0x00] == SBN1_OP_LED_RESPONSE )
			//      {

			//          SBN1_USART_Send_Packet ( nRF_ReceiveBuffer, 0x03 );
			//          break;
			//      }
			//  }

			//  Temp_Buffer[0] = SBN1_OP_LED_RESPONSE;
			//  Temp_Buffer[1] = 0x00;
			//  SBN1_USART_Send_Packet ( Temp_Buffer, 0x02 );

			//  break;
			// }
	}
}

/*==========  Task  ==========*/

// Create task / Register task to the loop
// @param uint16_t _id => to identify task
// @param func _action => what to do
// @param uint8_t _enable => whether to processed function during loop
// @param uint8_t ... => parameter register
uint8_t SBN1_Create_Task ( uint16_t _id, void ( *_action ) ( uint8_t * _register ), uint8_t _enable, ... )
{
	va_list argptr;
	uint8_t length, i;

	if ( SBN1_Task_Pointer >= SBN1_TASK_BUFFER )
	{
		return 0;
	}

	for ( i = 0; i < SBN1_Task_Pointer; i++ )
	{
		if ( SBN1_Task[i]._id == _id )
		{
			return 0;
		}
	}

	SBN1_Task[SBN1_Task_Pointer]._id = _id;
	SBN1_Task[SBN1_Task_Pointer]._action = _action;
	SBN1_Task[SBN1_Task_Pointer]._enable = _enable;

	// printf("id = %04X\r\n", SBN1_Task[SBN1_Task_Pointer]._id);

	va_start ( argptr, _enable );
	length = va_arg ( argptr, int );

	for ( i = 0; i < length; i++ )
	{
		SBN1_Task[SBN1_Task_Pointer]._register[i] = va_arg ( argptr, int );
	}

	va_end ( argptr );

	SBN1_Task_Pointer ++;

	return 1;
}

// Revoke the task
// @param uint16_t _id => identification of task
// @return result 1 for success
uint8_t SBN1_Revoke_Task ( uint16_t _id )
{
	uint8_t i, j;

	for ( i = 0; i < SBN1_Task_Pointer; i++ )
	{
		if ( SBN1_Task[i]._id == _id )
		{
			for ( j = i; j < SBN1_Task_Pointer - 1; j++ )
			{
				SBN1_Task[j] = SBN1_Task[j + 1];
			}

			SBN1_Task_Pointer--;

			return 1;
		}
	}

	return 0;
}

// Loop function
void SBN1_Loop ( void )
{
	uint8_t i;

	for ( i = 0; i < SBN1_Task_Pointer; i++ )
	{
		SBN1_Task_Item * _task = & SBN1_Task[i];
		#ifdef SBN1_DEBUG
		// printf("Task [%04X] -> EN = %02X\r\n", (*_task)._id, (*_task)._enable);
		#endif

		if ( ( *_task )._enable && ( *_task )._action )
		{
			#ifdef SBN1_DEBUG
			// printf("  Action => ");
			#endif
			( *_task )._action ( ( *_task )._register );
			#ifdef SBN1_DEBUG
			// printf("\r\n  End . \r\n");
			#endif
		}

		// delay_ms(200);
	}

}

/*==========  Operator Function  ==========*/

// Probing
void SBN1_Func_Probing ( uint8_t * _register )
{
	uint32_t temp0, temp1, temp2;

	#ifdef SBN1_DEBUG
	printf ( "  Probing\r\n" );
	printf ( "Send [ " );
	#endif

	SBN1_Clear_Buffer ( Temp_Buffer, SBN1_BUFSIZ );
	#ifndef SBN1_DEBUG

	Temp_Buffer[0x00] = SBN1_OP_PROBING_RESPONSE;
	Temp_Buffer[0x01] = SBN1_CONST_DEVICE;

	temp0 = * ( __IO uint32_t * ) ( 0x1FFFF7E8 );
	temp1 = * ( __IO uint32_t * ) ( 0x1FFFF7EC );
	temp2 = * ( __IO uint32_t * ) ( 0x1FFFF7F0 );

	Temp_Buffer[0x02] = ( uint8_t ) ( temp0 & 0x000000FF );
	Temp_Buffer[0x03] = ( uint8_t ) ( ( temp0 & 0x0000FF00 ) >> 8 );
	Temp_Buffer[0x04] = ( uint8_t ) ( ( temp0 & 0x00FF0000 ) >> 16 );
	Temp_Buffer[0x05] = ( uint8_t ) ( ( temp0 & 0xFF000000 ) >> 24 );
	Temp_Buffer[0x06] = ( uint8_t ) ( temp1 & 0x000000FF );
	Temp_Buffer[0x07] = ( uint8_t ) ( ( temp1 & 0x0000FF00 ) >> 8 );
	Temp_Buffer[0x08] = ( uint8_t ) ( ( temp1 & 0x00FF0000 ) >> 16 );
	Temp_Buffer[0x09] = ( uint8_t ) ( ( temp1 & 0xFF000000 ) >> 24 );
	Temp_Buffer[0x0A] = ( uint8_t ) ( temp2 & 0x000000FF );
	Temp_Buffer[0x0B] = ( uint8_t ) ( ( temp2 & 0x0000FF00 ) >> 8 );
	Temp_Buffer[0x0C] = ( uint8_t ) ( ( temp2 & 0x00FF0000 ) >> 16 );
	Temp_Buffer[0x0D] = ( uint8_t ) ( ( temp2 & 0xFF000000 ) >> 24 );
	#endif
	SBN1_USART_Send_Packet ( Temp_Buffer, 0x0E );

	#ifdef SBN1_DEBUG
	printf ( "]\r\n" );
	#endif
}

// Search devices
void SBN1_Func_Search ( uint8_t * _register )
{
	uint8_t * address = & _register[0x00], i;

	( *address ) ++;

	if ( ( *address ) == 0x00 )
	{
		( *address ) = 0x01;
	}

	#ifdef SBN1_DEBUG
	// printf("Searching %02X \r\n", (*address));
	#endif

	#ifndef SBN1_DEBUG
	nrf24l01ChangeAddress ( ( *address ) );
	nRF_SendBuffer[0] = SBN1_OP_SEARCH_REQUEST;

	i = nrf24l01TxData ( nRF_SendBuffer );

	// printf("RX_DR = %02X\r\n", i);

	if ( i == TX_DS )
	{
		// printf("TX_DS\r\n");
		nrf24l01RxMode();
		i = nrf24l01RxData ( nRF_ReceiveBuffer );

		// printf("RX_DR = %02X\r\n", i);
		if ( i == RX_DR && nRF_ReceiveBuffer[0x00] == SBN1_OP_SEARCH_RESPONSE )
		{
			// printf("Recv Buffer - [ ");
			// for(i=0;i<SBN1_PAYLOAD_WIDTH;i++)
			// {
			//  printf("%02X ", nRF_ReceiveBuffer[i]);
			// }
			// printf("]\r\n");
			DEBUG_PRINT ( "  Detected Device Address[ %02X ] \n", nRF_ReceiveBuffer[0x01] );
			// switch(nRF_ReceiveBuffer[0x02])
			// {
			//  case SBN1_CONST_DEXMO_V1_L:
			//      printf("DEXMO_V1_L");
			//      break;
			//  case SBN1_CONST_DEXMO_V1_R:
			//      printf("DEXMO_V1_R");
			//      break;
			// }
			// printf(" ] ID[ ");
			// for(i=3;i<=14;i++)
			// {
			//  printf("%02X ", nRF_ReceiveBuffer[i]);
			// }
			// printf("]\r\n");

			SBN1_USART_Send_Packet ( nRF_ReceiveBuffer, 0x0F );
		}
	}



	// nRF_SendBuffer[0x00] = SBN1_OP_SEARCH_RESPONSE;
	// nRF_SendBuffer[0x01] = Get_ChipID();
	// nRF_SendBuffer[0x02] = SBN1_CONST_DEVICE;


	// temp0=*(__IO u32*)(0x1FFFF7E8);
	// temp1=*(__IO u32*)(0x1FFFF7EC);
	// temp2=*(__IO u32*)(0x1FFFF7F0);
	// nRF_SendBuffer[0x03] = (u8)(temp0 & 0x000000FF);
	// nRF_SendBuffer[0x04] = (u8)((temp0 & 0x0000FF00)>>8);
	// nRF_SendBuffer[0x05] = (u8)((temp0 & 0x00FF0000)>>16);
	// nRF_SendBuffer[0x06] = (u8)((temp0 & 0xFF000000)>>24);
	// nRF_SendBuffer[0x07] = (u8)(temp1 & 0x000000FF);
	// nRF_SendBuffer[0x08] = (u8)((temp1 & 0x0000FF00)>>8);
	// nRF_SendBuffer[0x09] = (u8)((temp1 & 0x00FF0000)>>16);
	// nRF_SendBuffer[0x0A] = (u8)((temp1 & 0xFF000000)>>24);
	// nRF_SendBuffer[0x0B] = (u8)(temp2 & 0x000000FF);
	// nRF_SendBuffer[0x0C] = (u8)((temp2 & 0x0000FF00)>>8);
	// nRF_SendBuffer[0x0D] = (u8)((temp2 & 0x00FF0000)>>16);
	// nRF_SendBuffer[0x0E] = (u8)((temp2 & 0xFF000000)>>24);
	//      for(i = 0; i < )

	#endif
}

// // Read
// void SBN1_Func_Read ( uint8_t * _register )
// {
//  uint8_t * address = & _register[0x00], i, p, k = 0;

//  #ifdef SBN1_DEBUG
//  uint8_t i;
//  printf ( "    Read %02X\r\n", ( *address ) );
//  SBN1_Clear_Buffer ( Temp_Buffer, SBN1_BUFSIZ );
//  printf ( "Send [ " );
//  Temp_Buffer[0x00] = SBN1_OP_READ_RESPONSE;
//  Temp_Buffer[0x01] = ( *address );
//  Temp_Buffer[0x02] = 0x2A;

//  for ( i = 3; i < 0x29; i++ )
//  {
//      Temp_Buffer[i] = i - 1;
//  }

//  SBN1_USART_Send_Packet ( Temp_Buffer, 0x28 );
//  printf ( "]\r\n" );
//  #endif

//  #ifndef SBN1_DEBUG

//  // printf("\r\n  Read %02X - \r\n", (*address));

//  SBN1_Clear_Buffer ( Temp_Buffer, 40 );

//  nRF24L01_ChangeAddress ( ( *address ) );
//  nRF_SendBuffer[0x00] = SBN1_OP_READ_REQUEST;

//  // A/M
//  nRF_SendBuffer[0x01] = _register[0x01];

//  i = nRF24L01_TxData ( nRF_SendBuffer );

//  if ( i == TX_DS )
//  {

//      nRF24L01_RxMode();

//      // for(p = 0; p < 3 && (i != RX_DR); p ++)
//      // {
//      // printf(".");
//      // }

//      Temp_Buffer[0x00] = SBN1_OP_READ_RESPONSE;
//      Temp_Buffer[0x02] = 0x80;

//      for ( p = 0; p < 3 && k < 2; p++ )
//      {
//          // delay_us(3);

//          i = nRF24L01_RxData ( nRF_ReceiveBuffer );

//          if ( i == RX_DR && nRF_ReceiveBuffer[0x00] == SBN1_OP_READ_RESPONSE )
//          {
//              // printf("  Success\r\n");

//              // printf("Recv Buffer - [ ");
//              // for(i=0;i<SBN1_PAYLOAD_WIDTH;i++)
//              // {
//              //  printf("%02X ", nRF_ReceiveBuffer[i]);
//              // }
//              // printf("]\r\n");

//              Temp_Buffer[0x01] = nRF_ReceiveBuffer[0x01];
//              Temp_Buffer[0x02] |= nRF_ReceiveBuffer[0x02];
//              Temp_Buffer[0x02] |= nRF_ReceiveBuffer[0x03];

//              switch ( nRF_ReceiveBuffer[0x03] )
//              {
//                  // MPU
//                  case 0x20:
//                      for ( i = 0; i < 16; i++ )
//                      {
//                          Temp_Buffer[0x18 + i] = nRF_ReceiveBuffer[0x04 + i];
//                      }

//                      break;

//                  // ADC
//                  case 0x40:
//                      for ( i = 0; i < 21; i++ )
//                      {
//                          Temp_Buffer[0x03 + i] = nRF_ReceiveBuffer[0x04 + i];
//                      }

//                      break;
//              }

//              k++;
//          }
//      }

//      if ( k > 0 )
//      {
//          SBN1_USART_Send_Packet ( Temp_Buffer, 0x28 );
//      }

//  }
//  else
//  {
//      // printf("  TX failed\r\n");
//  }

//  #endif
// }

// // Lock
// void SBN1_Func_Lock ( uint8_t * _register )
// {
//  uint8_t _address = _register[0x00], _mode = _register[0x01];
//  #ifdef SBN1_DEBUG
//  int i;
//  printf ( "    Lock [ " );

//  for ( i = 0; i < 5; i++ )
//  {
//      printf ( "%d ", ( _mode >> i ) & 0x01 );

//  }

//  printf ( "]\r\n" );

//  SBN1_Clear_Buffer ( Temp_Buffer, SBN1_BUFSIZ );
//  printf ( "Send [ " );
//  Temp_Buffer[0x00] = _address;
//  Temp_Buffer[0x01] = 0x01;
//  SBN1_USART_Send_Packet ( Temp_Buffer, 0x02 );
//  printf ( "]\r\n" );
//  #endif
// }

// // Selfcheck
// void SBN1_Func_SelfCheck ( uint8_t * _register )
// {
//  uint8_t i, *address = & _register[0x00], count;

//  // SBN1_Clear_Buffer(Temp_Buffer, SBN1_BUFSIZ);

//  #ifdef SBN1_DEBUG
//  printf ( "  SelfCheck %02X\r\n", _address );

//  printf ( "Send [ " );
//  Temp_Buffer[0x00] = _address;
//  Temp_Buffer[0x01] = 0x01;
//  SBN1_USART_Send_Packet ( Temp_Buffer, 0x02 );
//  printf ( "]\r\n" );
//  #endif

//  #ifndef SBN1_DEBUG

//  printf ( "  SelfCheck %02X - ", ( *address ) );

//  nRF24L01_ChangeAddress ( ( *address ) );
//  nRF_SendBuffer[0] = SBN1_OP_SELFCHECK_REQUEST;

//  i = nRF24L01_TxData ( nRF_SendBuffer );

//  if ( i == TX_DS )
//  {

//      nRF24L01_RxMode();
//      i = nRF24L01_RxData ( nRF_ReceiveBuffer );

//      // printf("RX_DR = %02X\r\n", i);
//      // printf("Recv Buffer - [ ");
//      // for(i=0;i<SBN1_PAYLOAD_WIDTH;i++)
//      // {
//      //  printf("%02X ", nRF_ReceiveBuffer[i]);
//      // }
//      // printf("]\r\n");
//      if ( i == RX_DR && nRF_ReceiveBuffer[0x00] == SBN1_OP_SELFCEHCK_RESPONSE )
//      {
//          printf ( "  Success\r\n" );
//          SBN1_USART_Send_Packet ( nRF_ReceiveBuffer, 0x03 );
//      }
//      else
//      {
//          printf ( "  No response\r\n" );
//      }
//  }
//  else
//  {
//      printf ( "  TX failed\r\n" );
//  }

//  #endif
// }

/*==========  End  ==========*/

////////////////////////////////////////////////////////////////////////
//
//    Author Rijn, 2015.
//
////////////////////////////////////////////////////////////////////////
