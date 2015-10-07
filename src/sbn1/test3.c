// SBN1 protocal test 3
// SelfCheck

#include "sys.h"

int main(void)
{
	uint8_t i;
	printf("SBN1 test 3 \r\n");

	Fill_Buffer(0x09, 0x53, 0x42, 0x4E, 0x31, 0x00, 0x02, 0x09, 0x09, 0x00);
	SBN1_Handle_Reveived();

	for(i = 0x00; i < 0xff; i++)
	{
		SBN1_Loop();
	}

	return 0;
}
