#include "RTC.h"

__IO uint8_t RTC_second_flag = 0;
extern __IO uint8_t esp8266_second_flag;

void RTC_IRQHandler() {
	static uint8_t test = 0;
	test++;
	RTC_second_flag = 1;
	if (test == 40) {
		esp8266_second_flag = 1;
		test = 0;
	}

	RTC->CRL &= ~RTC_CRL_SECF;
}

static uint8_t RTC_YearIsLeap(uint16_t year)
{
	if (((year / 4) * 4 == year) && (year % 400 == 0))
		return 1;
	else if (((year / 4) * 4 == year) && (year % 100 != 0))
		return 1;

	return 0;
}

static uint8_t RTC_DaysInMonth(uint16_t year, uint8_t month)
{
	if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8
			 || month == 10 || month == 12) 
		return 31;
	else if (RTC_YearIsLeap(year) && month == 2)
		return 29;
	else if (month == 2)
		return 28;
	else
		return 30;

	return 0;
}

void RTC_SetDate(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, 
		 uint8_t min, uint8_t sec)
{
	uint32_t days = 0;
	uint8_t months = 1;
	uint32_t years = 0;
	uint32_t temp = 0;
	uint32_t cnt = 2016;

	years = year - cnt;

	for (uint32_t i = 0; i < years; i++) {
		if (RTC_YearIsLeap(cnt))
			days += 366;
		else
			days += 365;
		
		cnt++;
	}

	days += day;

	if (hour >= 24) {
		days++;
		hour -= 24;
	}

	while (months < month) {
		days += RTC_DaysInMonth(cnt, months);
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

static void RTC_CalculateDate(uint32_t temp, uint8_t *month_s, uint8_t *day_s, 
			 uint8_t *hour_s, uint8_t *min_s, uint8_t *sec_s, 
			 uint16_t *year_s)
{
	uint8_t not_leap = 0;
	*year_s = 0;
	*month_s = 0;
	*day_s = 0;
	*sec_s = temp % 60;
	temp -= *sec_s;

	*min_s = (temp / 60) % 60;
	temp -= *min_s * 60;

	*hour_s = (temp / 3600) % 24;
	temp -= *hour_s * 3600;
		
	temp = temp / (60 * 60 * 24);

	*year_s = 2016;
	while(temp) {
		if (*month_s == 0) {
			*month_s = 1;
		}
		else if (*month_s == 1 || *month_s == 3 || *month_s == 5 || *month_s == 7 || *month_s == 8
			 || *month_s == 10 || *month_s == 12) {
			if (temp > 31) {
				*month_s = *month_s + 1;
				if (*month_s == 13) {
					*month_s = 1;
					not_leap++;
					not_leap %= 4;
					year_s++;
				}
				temp -= 31;	
			} else {
				*day_s = temp;
				temp = 0;
			}
		}

		else if (*month_s == 2) {
			if (not_leap && (temp > 28)) {
			*month_s = *month_s + 1;
				temp -= 28;
			} else if (not_leap) {
				*day_s = temp;
				temp = 0;	
			} else if (!not_leap && (temp > 29)) {
				*month_s = *month_s + 1;
				temp -= 29;
			} else {
				*day_s = temp;
				temp = 0;
			}
		}

		else if (*month_s == 4 || *month_s == 6 || *month_s == 9 || *month_s == 11) {
			if (temp > 30) {
				*month_s = *month_s + 1;
				temp -= 30;
			} else {
				*day_s = temp;
				temp = 0;
			}
		}	
	}
}

static uint8_t RTC_GetLastSunday(uint16_t year, uint8_t month, uint8_t day)
{
	uint16_t tmp, tmp_2 = 0;
	const uint8_t months_val[] = {0, 0, 3, 3, 6, 1, 4, 6, 2,5, 0, 3, 5};
	uint8_t wod;
	tmp = day + months_val[month];
	tmp %= 7;
	tmp_2 = (year % 100) % 28;
	tmp_2 += (year % 100) / 4;

	if (RTC_YearIsLeap(year) && (month == 1 || month == 2))
		tmp_2--;

	wod = (tmp + tmp_2) % 7;

	if (wod)
		return 0;

	if (day + 7 > RTC_DaysInMonth(year, month)) {
		return day; 
	} else {
		while (day + 7 < RTC_DaysInMonth(year, month)) 
			day += 7;

		return day;
	}

	return 0;
}

static uint8_t RTC_CheckDST(uint32_t *temp, uint16_t year, uint8_t month, 
			    uint8_t day, uint8_t hour)
{
	uint8_t tmp;

	if (month > 10 || month < 3) {
		*temp += 60 * 60;
	} else if (month < 10 && month > 3) {
		*temp += 2 * 60 * 60;
	} else if (month == 3) {
		tmp = RTC_GetLastSunday(year, month, day);
		if (day < tmp) {
			*temp += 60 * 60;
		} else if (day > tmp) {
			*temp += 2 * 60 * 60;
		} else {
			if (hour < 1)
				*temp += 60 * 60;
			else
				*temp += 2 * 60 * 60;
		}
	} else {
		tmp = RTC_GetLastSunday(year, month, day);
		if (day < tmp) {
			*temp += 2 * 60 * 60;
		} else if (day > tmp) {
			*temp += 60 * 60;
		} else {
			if (hour < 1)
				*temp += 2 * 60 * 60;
			else
				*temp += 60 * 60;
		}
	}
	return 0;	
}

uint8_t RTC_GetDate(uint16_t *year, uint8_t *month, uint8_t *day, 
		    uint8_t *hour, uint8_t *min, uint8_t *sec) {
	uint32_t temp;
	uint8_t ret = 0;
	static uint8_t month_s, day_s, hour_s, min_s, sec_s;
	static uint16_t year_s;
	if (RTC_second_flag) {

		temp = (RTC->CNTH << 16) | RTC->CNTL;

		RTC_CalculateDate(temp, &month_s, &day_s, &hour_s, &min_s, &sec_s, &year_s);
		RTC_CheckDST(&temp, year_s, month_s, day_s, hour_s);
		RTC_CalculateDate(temp, &month_s, &day_s, &hour_s, &min_s, &sec_s, &year_s);		

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
