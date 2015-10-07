// SBN1 protocal test 2
// read devices

#include "sys.h"

int main(void)
{
	uint8_t i;
	printf("SBN1 test 5 \r\n");

	// lock 0xAB 01000
	Fill_Buffer(0x0A, 0x53, 0x42, 0x4E, 0x31, 0x00, 0x03, 0xB4, 0x07, 0xAB, 0x02);
	SBN1_Handle_Reveived();

	// lock 0xAB 00111
	Fill_Buffer(0x0A, 0x53, 0x42, 0x4E, 0x31, 0x01, 0x03, 0xCE, 0x07, 0xAB, 0x1C);
	SBN1_Handle_Reveived();

	return 0;
}