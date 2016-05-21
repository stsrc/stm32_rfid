#include "RTC.h"

__IO uint8_t RTC_second_flag = 0;

void RTC_IRQHandler() {
	static uint8_t cnt = 0;
	RTC_second_flag = 1;
	RTC->CRL &= ~RTC_CRL_SECF;
	cnt++;
	if (cnt == 5) {
		RFID_Read();
		cnt = 0;
	}
}

void RTC_SetDate(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, 
		 uint8_t min, uint8_t sec)
{
	uint32_t days = 0;
	uint8_t months = 1;
	uint8_t years = 0;
	uint8_t leap = 0;
	uint32_t temp = 0;
	years = (2016 - year);
	leap = years / 4;
	years -= leap;
	days = years * 356 + leap * 366 + day;
	if (hour >= 24) {
		days++;
		hour -= 24;
	}
	while (months < month) {
		if (months == 1 || months == 3 || months == 5 || months == 7 
		    || months == 8 || months == 10 || months == 12)
			days += 31;
	
		else if (months == 4 || months == 6 || months == 9 || months == 11)
			days += 30;
		
		else if (!years) 
			days += 29;

		else if ((years/4) * 4 == years)
			days += 28;
		else
			days += 29;

		months++;
	}
	temp = days * 60 * 60 * 24 + hour * 60 * 60 + min * 60 + sec;
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


uint8_t RTC_GetDate(uint16_t *year, uint8_t *month, uint8_t *day, 
		    uint8_t *hour, uint8_t *min, uint8_t *sec) {
	uint32_t temp;
	uint8_t ret = 0;
	uint8_t not_leap = 0;
	static uint8_t month_s, day_s, hour_s, min_s, sec_s;
	static uint16_t year_s;
	if (RTC_second_flag) {
		year_s = 0;
		month_s = 0;
		day_s = 0;

		temp = (RTC->CNTH << 16) | RTC->CNTL;

		sec_s = temp % 60;
		temp -= sec_s;

		min_s = temp / 60 % 60;
		temp -= min_s * 60;

		hour_s = (temp / 3600) % 24;
		temp -= hour_s * 3600;
		
		temp = temp / (60 * 60 * 24);

		year_s = 2016;
		while(temp) {
			if (month_s == 0) {
				month_s = 1;
			}
			else if (month_s == 1 || month_s == 3 || month_s == 5 || month_s == 7 || month_s == 8
				 || month_s == 10 || month_s == 12) {
				if (temp > 31) {
					month_s++;
					if (month_s == 13) {
						month_s = 1;
						not_leap++;
						not_leap %= 4;
						year_s++;
					}
					temp -= 31;	
				} else {
					day_s = temp;
					temp = 0;
				}
			}

			else if (month_s == 2) {
				if (not_leap && (temp > 28)) {
					month_s++;
					temp -= 28;
				} else if (not_leap) {
					day_s = temp;
					temp = 0;	
				} else if (!not_leap && (temp > 29)) {
					month_s++;
					temp -= 29;
				} else {
					day_s = temp;
					temp = 0;
				}
			}

			else if (month_s == 4 || month_s == 6 || month_s == 9 || month_s == 11) {
				if (temp > 30) {
					month_s++;
					temp -= 30;
				} else {
					day_s = temp;
					temp = 0;
				}
			}	
		}

		ret = 1;
		RTC_second_flag = 0;
	}
	*year = year_s;
	*month = month_s;
	*day = day_s;
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
