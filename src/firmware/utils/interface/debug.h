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
#include "uart_fifo.h"
#include "eprintf.h"

#ifdef DEBUG_MODULE
#define DEBUG_FMT(fmt) DEBUG_MODULE ": " fmt
#endif

#if defined(DEBUG_PRINT_ON_UART)
  #ifndef ENABLE_UART
    #error "Need to define ENABLE_UART to use DEBUG_PRINT_ON_UART"
  #endif
  #define DEBUG_PRINT(fmt, ...) uartPrintf(DEBUG_FMT(fmt), ##__VA_ARGS__)
#endif


#ifdef TEST_PRINTS
  #define TEST_AND_PRINT(e, msgOK, msgFail)\
    if(e) { DEBUG_PRINT(msgOK); } else { DEBUG_PRINT(msgFail); }
  #define FAIL_PRINT(msg) DEBUG_PRINT(msg)
#else
  #define TEST_AND_PRINT(e, msgOK, msgFail)
  #define FAIL_PRINT(msg)
#endif

