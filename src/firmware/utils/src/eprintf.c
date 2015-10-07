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

#include "eprintf.h"

#include <stdarg.h>
#include <ctype.h>

static const char digit[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
                             'A', 'B', 'C', 'D', 'E', 'F'};

int get_int_len (int value)
{
  int l=1;
  while(value>9)
  {
    l++;
    value/=10;
  }
  return l;
}

int power(int a, int b)
{
  int i;
  int x = a;

  for (i = 1; i < b; i++)
  {
    x *= a;
  }

  return x;
}

static int itoa(putc_t putcf, int num, int base, int precision)
{
  long long int i = 1;
  int len = 0;
  unsigned int n = num;
  int numLenght = get_int_len(num);
  int fillWithZero = 0;

  if (num == 0)
  {
    putcf('0');
    return 1;
  }

  if (num < 0  && base == 10)
  {
    n = -num;
    putcf('-');
  }

  if (numLenght < precision)
  {
    fillWithZero = precision -numLenght;
    while (fillWithZero>0)
    {
      putcf('0');
      len++;
      fillWithZero--;
    }
  }

  while (n / i)
  i*=base;

  while (i /= base)
  {
    putcf(digit[(n / i) % base]);
    len++;
  }
  
  return len;
}

int evprintf(putc_t putcf, char * fmt, va_list ap)
{
  int len=0;
  float num;
  char* str;
  int precision;

  while (*fmt)
  {
    precision = 6;
    if (*fmt == '%')
    {
      while (!isalpha((unsigned) * ++fmt))//TODO: Implement basic print length handling!
      {
        if (*fmt == '.')
        {
          if (isdigit((unsigned)*++fmt))
            precision = *fmt - '0';
        }
      }
      switch (*fmt++)
      {
        case 'i':
        case 'd':
          len += itoa(putcf, va_arg(ap, int), 10 , 0);
          break;
        case 'x':
        case 'X':
          len += itoa(putcf, va_arg(ap, int), 16 , 0);
          break;
        case 'f':
          num = va_arg(ap, double);
          if(num<0)
          {
            putcf('-');
            num = -num;
            len++;
          }
          len += itoa(putcf, (int)num, 10, 0);
          putcf('.'); len++;
          len += itoa(putcf, (num - (int)num) * power(10,precision), 10, precision);
          break;
        case 's':
          str = va_arg( ap, char* );
          while(*str)
          {
            putcf(*str++);
            len++;
          }
          break;
        default:
          break;
      }
    }
    else
    {
      putcf(*fmt++);
      len++;
    }
  }
  
  return len;
}

int eprintf(putc_t putcf, char * fmt, ...)
{
  va_list ap;
  int len;

  va_start(ap, fmt);
  len = evprintf(putcf, fmt, ap);
  va_end(ap);

  return len;
}
