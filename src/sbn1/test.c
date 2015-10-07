#include "sys.h"

uint8_t Debug_Buffer[100];

uint8_t Fill_Buffer(int _length, ...)
{
	va_list argptr;
	uint8_t i;

	Debug_Buffer[0x00] = _length;

	va_start(argptr, _length);
	for(i = 0; i < _length; i++)
	{
		Debug_Buffer[i+1] = va_arg(argptr, int);
	}
	va_end(argptr);

	return 0;
}