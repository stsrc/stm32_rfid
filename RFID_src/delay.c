#include "delay.h"
uint32_t __IO delay_val;

void delay_init(){
	if(SysTick_Config(SystemCoreClock / 100000UL)){
		while(1);
	}
}

void delay_ms(uint32_t value){	//unit of value = [ms]
	delay_val = value*100;
	while(delay_val != 0);
}

/*Cannot get less than 10us because of SystemCoreClock?*/
void delay_us(uint32_t value){	/*multiply value (i.e. value*10us)*/
	delay_val = value;
	while(delay_val != 0);
}

void SysTick_Handler(void){
	if(delay_val != 0) delay_val--;
}
