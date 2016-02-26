#include <stm32f1xx.h>
#include <stdlib.h>
#include "tm_stm32f1_ili9341.h"

int main(void)
{
	TM_ILI9341_InitLCD();
	while(1);
	return 0;
}
