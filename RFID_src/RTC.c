#include "RTC.h"

__IO uint8_t RTC_second_flag = 0;

void RTC_IRQHandler() {
	RTC_second_flag = 1;
	RTC->CRL &= ~RTC_CRL_SECF;
}

static void RTC_set_time(uint8_t hour, uint8_t min, uint8_t sec) {
	uint32_t temp = hour * 3600 + min * 60 + sec;
	RTC->CNTH = (uint16_t)(temp >> 16);
	RTC->CNTL = (uint16_t)(temp);
}

void RTC_get_time(uint8_t *hour, uint8_t *min, uint8_t *sec) {
	uint32_t temp = RTC->CNTH + RTC->CNTL;
	*hour = temp / 3600;
	temp = temp - 3600 * *hour;
	*min = temp / 60;
	temp = temp - 60 * *min;
	*sec = temp;
}

HAL_StatusTypeDef RTC_Init()
{
	/*
	 * After reset access to RTC registers is deisable, so
	 * there is need to update RCC register. Look to datasheet
	 */
	RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
	PWR->CR |= PWR_CR_DBP;
	// Setting clock to RTC. Using LSE clock (low speed external)
	RCC->BDCR |= RCC_BDCR_RTCSEL_0;
	RCC->BDCR &= RCC_BDCR_RTCSEL_1;
	// Enabling LSE oscillator
	RCC->BDCR |= RCC_BDCR_LSEON;
	// Waiting for stable oscillator
	while(!(RCC->BDCR & RCC_BDCR_LSERDY));
	// Enabling RTC clock;
	RCC->BDCR |= RCC_BDCR_RTCEN;	
	/* Synchronizing APB1 bus with RTC (read datasheet)*/
	while(!(RTC->CRL & RTC_CRL_RSF));
	/* Waiting for end of write operation */
	while(!(RTC->CRL & RTC_CRL_RTOFF));
	/* Entering configuration mode */
	RTC->CRL |= RTC_CRL_CNF;

	/*Writing to registers*/
	
	/* Second interrupt enable */
	RTC->CRH = RTC_CRH_SECIE;
	/* RTC prescaler reload value high */
	RTC->PRLH = 0;
	/* RTC prescaler reload value low */
	RTC->PRLL = 0x7FFF;
	
	RTC_set_time(11, 11, 11);

	/* Leaving configuration mode */
	RTC->CRL &= ~RTC_CRL_CNF;
	/* Waiting for end of write operation */
	while(!(RTC->CRL & RTC_CRL_RTOFF)); 
	HAL_NVIC_EnableIRQ(RTC_IRQn);
	return HAL_OK;
}
