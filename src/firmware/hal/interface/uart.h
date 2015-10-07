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

#ifndef UART_H_
#define UART_H_

#include <stdbool.h>
#include "platform.h"
#include "eprintf.h"

void uartInit(void);
void uartSendData(uint32_t size, uint8_t* data);
int uartPutchar(int ch);
#define uartPrintf(FMT, ...) eprintf(uartPutchar, FMT, ## __VA_ARGS__)
void uartSendDataDma(uint32_t size, uint8_t* data);
void uartIsr(void);
void uartDmaIsr(void);

#endif /* UART_H_ */

