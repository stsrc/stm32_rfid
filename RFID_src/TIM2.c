#include "TIM2.h"

static __IO uint8_t clear_lcd = 0;
static __IO uint8_t turn_off_lcd = 0;
static __IO uint8_t turn_on_RFID = 0;


void TIM2_IRQHandler(void)
{
	if(TIM2->SR & TIM_SR_UIF){
		TIM2->SR &= ~TIM_SR_UIF;
			
		if (clear_lcd) {
			clear_lcd--;
			if (!clear_lcd) 
				TM_ILI9341_DrawFilledRectangle(0, 100, 239, 
						319, ILI9341_COLOR_BLACK);	
		}
	
		if (turn_off_lcd) {
			turn_off_lcd--;
			if (!turn_off_lcd)
				TM_ILI9341_DisplayOff();
			else if (turn_off_lcd == 1)
				xpt2046_InterruptOn();
		}

		if (turn_on_RFID) {
			turn_on_RFID--;
			if (!turn_on_RFID)
				RFID_Read();
		}
	}
}

void TIM2_Init() 
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	/*update request source only from over/under-flow, or DMA*/
	TIM2->CR1 |= TIM_CR1_URS;
	/*Prescaler = 54000 => Fclock = 500Hz;*/
	TIM2->PSC = 54000;
	/*Overflow after 1s*/
	TIM2->ARR = 500;
	/*NVIC updating*/
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	/*Update interrupt enabled*/
	TIM2->DIER |= TIM_DIER_UIE;
	/*Counter enabled*/
	TIM2->CR1 |= TIM_CR1_CEN;
}

void TIM2_ClearLCDAfterTimeInterval(uint8_t sec) 
{
	clear_lcd = sec;
}

void TIM2_TurnOffLCDAfterTimeInterval(uint8_t sec) 
{
	turn_off_lcd = sec;
}

void TIM2_TurnOnRFIDAfterTimeInterval(uint8_t sec)
{
	turn_on_RFID = sec;
}
