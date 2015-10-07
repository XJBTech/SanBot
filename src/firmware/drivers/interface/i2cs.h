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

#ifndef __I2CS_H
#define __I2CS_H

#include "config.h"

typedef struct
{
  uint8_t*        pbBuffer;        /*!<The address of the buffer from/to which the transfer should start */

  __IO uint32_t   wNumData;        /*!<Number of data to be transferred for the current transaction */

  uint32_t        wAddr1;          /*!<Contains the Target device Address (optional)*/

  uint32_t        wAddr2;          /*!<Contains the Register/Physical Address into the device (optional) */

} CPAL_TransferTypeDef;

void i2csInit(void);
uint8_t i2csStart(void);
void i2csStop(void);
uint8_t i2csWaitAck(void);
void i2csAck(void);
void i2csNAck(void);
void i2csSendByte(uint8_t txd);
uint8_t i2csReadByte(unsigned char ack);
uint8_t i2csWriteOneByte(uint8_t Address, uint8_t Register, uint8_t Data);
uint8_t i2csReadOneByte(uint8_t Address, uint8_t Register);
uint8_t i2csProbing(uint8_t address);
uint8_t i2csWrite(uint8_t slave_addr,  uint8_t reg_addr, uint8_t length, uint8_t *data);
uint8_t i2csRead(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t *data);

#endif
