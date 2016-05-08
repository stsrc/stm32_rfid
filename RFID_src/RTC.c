#include "RTC.h"

__IO uint8_t RTC_second_flag = 0;

void RTC_IRQHandler() {
	RTC_second_flag = 1;
	RTC->CRL &= ~RTC_CRL_SECF;	
}

void RTC_SetTime(uint8_t hour, uint8_t min, uint8_t sec) {
	uint32_t temp = hour * 3600 + min * 60 + sec;
	/* Entering configuration mode */
	RTC->CRL |= RTC_CRL_CNF;
	/* Waiting for entry to configuration mode */
	while(!(RTC->CRL & RTC_CRL_RTOFF));

	RTC->CNTH = (uint16_t)(temp >> 16);
	RTC->CNTL = (uint16_t)(temp);
	/* Leaving configuration mode */
	RTC->CRL &= ~RTC_CRL_CNF;
	
	/* Waiting for end of write operation to CRL*/
	while(!(RTC->CRL & RTC_CRL_RTOFF)); 
}

uint8_t RTC_GetTime(uint8_t *hour, uint8_t *min, uint8_t *sec) {
	uint32_t temp;
	uint8_t ret = 0;
	static uint8_t hour_s, min_s, sec_s;
	if (RTC_second_flag) {
		temp = (RTC->CNTH << 16) | RTC->CNTL;
		hour_s = (temp / 3600) % 24;
		min_s = temp / 60 % 60;
		sec_s = temp % 60;
		ret = 1;
		RTC_second_flag = 0;
	}
	*hour = hour_s;
	*min = min_s;
	*sec = sec_s;
	return ret;
}

HAL_StatusTypeDef RTC_Init()
{
	/*
	 * After reset access to RTC registers is disabled, so
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
	/* Second interrupt enable */
	RTC->CRH = RTC_CRH_SECIE;
	/* RTC prescaler reload value high */
	RTC->PRLH = 0;
	/* RTC prescaler reload value low */
	RTC->PRLL = 0x7FFF;
	/* Leaving configuration mode */
	RTC->CRL &= ~RTC_CRL_CNF;
	/* Waiting for end of write operation */
	while(!(RTC->CRL & RTC_CRL_RTOFF)); 
	HAL_NVIC_EnableIRQ(RTC_IRQn);
	return HAL_OK;
}
