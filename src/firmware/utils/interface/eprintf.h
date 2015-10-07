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

#include <stdarg.h>

#ifndef	__EPRINTF_H__
#define __EPRINTF_H__

/**
 * putc function pointer definition
 */
typedef int (*putc_t)(int c);

/**
 * Light printf implementation
 * @param[in] putcf Putchar function to be used by Printf
 * @param[in] fmt Format string
 * @param[in] ... Parameters to print
 * @return the number of character printed
 */
int eprintf(putc_t putcf, char * fmt, ...) 
    __attribute__ (( format(printf, 2, 3) ));

/**
 * Light printf implementation
 * @param[in] putcf Putchar function to be used by Printf
 * @param[in] fmt Format string
 * @param[in] ap Parameters to print
 * @return the number of character printed
 */
int evprintf(putc_t putcf, char * fmt, va_list ap);

#endif //__EPRINTF_H__
