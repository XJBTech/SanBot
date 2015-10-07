// SBN1 protocal test 2
// Searching devices

#include "sys.h"

int main(void)
{
	uint8_t i;
	printf("SBN1 test 2 \r\n");

	Fill_Buffer(0x09, 0x53, 0x42, 0x4E, 0x31, 0x00, 0x02, 0x04, 0x03, 0x01);
	SBN1_Handle_Reveived();

	for(i = 0x00; i < 0xff; i++)
	{
		SBN1_Loop();
	}

	return 0;
}
