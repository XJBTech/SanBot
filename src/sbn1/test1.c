// SBN1 protocal test 1
// Probing

#include "sys.h"

int main(void)
{
	printf("SBN1 test 1 \r\n");

	Fill_Buffer(0x08, 0x53, 0x42, 0x4E, 0x31, 0x00, 0x01, 0x01, 0x01);
	SBN1_Handle_Reveived();

	return 0;
}
