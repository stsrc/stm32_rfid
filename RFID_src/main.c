#include <stm32f1xx.h>
#include <stdlib.h>
#include "stupid_delay.h"

/**
 * int SPI1_init() - initialization of SPI1 interface.
 * Function performs pin and SPI interface initalization.
 */

enum SPI1_device {LCD, TOUCH};

int SPI1_init()
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN;
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	GPIOA->CRL |= GPIO_CRL_MODE0_1 | GPIO_CRL_MODE1_1;
	GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_CNF1);
	GPIOC->CRL |= GPIO_CRL_MODE0_1 | GPIO_CRL_MODE1_1 | GPIO_CRL_MODE2_1;
	GPIOC->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_CNF1 | GPIO_CRL_CNF2);
	
	GPIOA->BSRR |= GPIO_BSRR_BS0 | GPIO_BSRR_BS1;
	GPIOC->BSRR |= GPIO_BSRR_BS0 | GPIO_BSRR_BS1 | GPIO_BSRR_BS2;

	SPI1->CR1 |= SPI_CR1_SPE;
	SPI1->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1;
	SPI1->CR1 |= SPI_CR1_MSTR;
	return 0;
}

void SPI1_send_message(const enum SPI1_device dev, const uint8_t msg, 
		       const uint8_t d_cx)
{
	switch(dev) {
	case LCD:
		GPIOA->BSRR |= GPIO_BSRR_BS0;
		if (d_cx)
			GPIOC->BSRR |= GPIO_BSRR_BS1;
		else
			GPIOC->BSRR |= GPIO_BSRR_BR1;
		break;
	case TOUCH:
		GPIOA->BSRR |= GPIO_BSRR_BS1;
		break;
	}
	
	SPI1->DR = msg;
	
	switch(dev) {
	case LCD:
		GPIOA->BSRR |= GPIO_BSRR_BR0;
		break;
	case TOUCH:
		GPIOA->BSRR |= GPIO_BSRR_BR1;
		break;
	}
}

int main(void)
{
	SPI1_init();
	while(1);
	return 0;
}
