#include <stdio.h>
#include <stdlib.h>

#include "tools.h"

int main(void)
{
	uint8_t value[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11};
	char buffer[100];
	mem_to_string(buffer, 100, value, 16, true, true);
	printf("%s\n", buffer);
}