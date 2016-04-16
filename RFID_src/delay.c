#include "delay.h"
static uint32_t __IO delay_val;

void delay_init(){
	if(SysTick_Config(SystemCoreClock / 1000UL)){
		while(1);
	}
}

void delay_ms(uint32_t value){
	delay_val = value;
	while(delay_val != 0);
}

void SysTick_Handler(void){
	if(delay_val != 0) delay_val--;
	HAL_IncTick();
}
