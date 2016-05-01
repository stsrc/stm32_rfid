#include "esp8266.h"

#define ESP8266_RST_PORT	GPIOA
#define ESP8266_RST_PIN		GPIO_PIN_4

#define ESP8266_FW_PORT		GPIOA
#define ESP8266_FW_PIN		GPIO_PIN_8

#define AT_RESET_CMD		"AT+RST\r\n\0"
#define AT_CWMODE_1		"AT+CWMODE=1\r\n\0"
#define AT_PING_GOOGLE		"AT+PING=\"www.google.com\"\r\n\0"
#define AT_CIPMODE_0		"AT+CIPMODE=0\r\n\0"
#define AT_CIPMUX_1		"AT+CIPMUX=1\r\n\0"
#define AT_CIPSERVER		"AT+CIPSERVER=1,80\r\n\0"
#define AT_CIPSTO		"AT+CIPSTO=30\r\n\0"

void esp8266_InitPins() 
{
	GPIO_InitTypeDef init;
	init.Mode = GPIO_MODE_OUTPUT_PP;
	init.Pull = GPIO_NOPULL;
	init.Speed = GPIO_SPEED_FREQ_HIGH;
	init.Pin = ESP8266_RST_PIN | ESP8266_FW_PIN;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(ESP8266_RST_PORT, &init);
	HAL_GPIO_WritePin(ESP8266_FW_PORT, ESP8266_FW_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ESP8266_RST_PORT, ESP8266_RST_PIN, GPIO_PIN_SET);
}

void esp8266_HardReset() 
{
	HAL_GPIO_WritePin(ESP8266_RST_PORT, ESP8266_RST_PIN, GPIO_PIN_RESET);
	delay_ms(50);
	HAL_GPIO_WritePin(ESP8266_RST_PORT, ESP8266_RST_PIN, GPIO_PIN_SET);
	delay_ms(500);	
}

int8_t esp8266_WaitForOk(const char *command, unsigned int delay, uint8_t multiplier) 
{
	int8_t ret;
	char temp[BUF_MEM_SIZE];
	ret = esp8266_GetReply(command, temp, delay, multiplier);
	return ret;	
}

static int8_t esp8266_ConnectToWiFi()
{
	int ret;
	esp8266_Send(AT_CWMODE_1);	
	ret = esp8266_WaitForOk(AT_CWMODE_1, 100, 100);
	if (ret)
		return -1;
	esp8266_Send(WIFI_PASSWD_SECRET);
	ret = esp8266_WaitForOk(WIFI_PASSWD_SECRET, 100, 100);
	if (ret)
		return -2;
	esp8266_Send(AT_PING_GOOGLE);
	ret = esp8266_WaitForOk(AT_PING_GOOGLE, 100, 100);
	if (ret)
		return -3;
	return 0;
}

int8_t esp8266_MakeAsServer()
{
	int8_t ret;
	ret = esp8266_Send(AT_CIPMODE_0);
	if (ret)
		return -1;
	ret = esp8266_WaitForOk(AT_CIPMODE_0, 100, 100);
	if (ret)
		return -2;
	ret = esp8266_Send(AT_CIPMUX_1);
	if (ret)
		return -3;
	ret = esp8266_WaitForOk(AT_CIPMUX_1, 100, 100);
	if (ret)
		return -4;
	ret = esp8266_Send(AT_CIPSERVER);
	if (ret)
		return -5;
	ret = esp8266_WaitForOk(AT_CIPSERVER, 100, 100);
	if (ret)
		return -6;
	ret = esp8266_Send(AT_CIPSTO);
	if (ret)
		return -7;
	ret = esp8266_WaitForOk(AT_CIPSTO, 100, 100);
	if (ret)
		return -8;
	return 0;
}

int8_t esp8266_Init() 
{
	int ret;
	const char * const RST_CMD = AT_RESET_CMD;
	esp8266_InitPins();
	esp8266_HardReset();
	UART_2_init();
	buffer_Reset(&UART2_transmit_buffer);
	delay_ms(5000);
	esp8266_Send(RST_CMD);
	delay_ms(5000);
	buffer_Reset(&UART2_receive_buffer);
	ret = esp8266_ConnectToWiFi();
	if (ret)
		return ret;
	return ret;
}

int8_t esp8266_Send(const char *data) 
{
	int8_t ret;
	ret = buffer_set_text(&UART2_transmit_buffer, data);
	if (!ret)
		UART_2_transmit();
	return ret;
}

int8_t esp8266_SendGetReply(const char *command, char *output, 
			    unsigned int delay, uint8_t multiplier)
{
	int8_t ret;
	if (buffer_IsFull(&UART2_transmit_buffer))
		return -ENOMEM;
	buffer_Reset(&UART2_receive_buffer);		
	esp8266_Send(command);
	ret = esp8266_GetReply(command, output, delay, multiplier);	
	return ret;
}

int8_t esp8266_GetReply(const char *command, char *output, unsigned int delay,
			uint8_t multiplier)
{
	int8_t ret, cnt = 0;
	do {
		delay_ms(delay);
		ret = buffer_SearchGetLabel(&UART2_receive_buffer, command, output);
	} while (((ret == -EBUSY) || (ret == -EINVAL)) && (++cnt < multiplier));
	return ret;
}

static inline void parseTime(uint8_t *hour, uint8_t *minute, uint8_t *second, 
			     char *buf)
{
	size_t len = strlen(buf);
	unsigned short temp_h, temp_min, temp_sec;
	char hour_c[3] = {'\0', '\0', '\0'};
	char min_c[3] = {'\0', '\0', '\0'};
	char sec_c[3] = {'\0', '\0', '\0'}; 
	char *temp;
	temp = &buf[len - 2]; //wtf
	sec_c[1] = *temp--;
	sec_c[0] = *temp;
	temp = temp - 2;
	min_c[1] = *temp--;
	min_c[0] = *temp;
	temp = temp - 2;
	hour_c[1] = *temp--;
	hour_c[0] = *temp;
	sscanf(hour_c, "%hu", &temp_h);
	sscanf(min_c, "%hu", &temp_min);
	sscanf(sec_c, "%hu", &temp_sec);
	*hour = (uint8_t)temp_h;
	*minute = (uint8_t)temp_min;
	*second = (uint8_t)temp_sec;	
}	

int8_t esp8266_GetTime(uint8_t *hour, uint8_t *minute, uint8_t *second)
{
	char buf[BUF_MEM_SIZE];
	char *http_connect = "AT+CIPSTART=\"TCP\",\"www.google.com\",80\r\n\0";
	char *http_disconnect = "AT+CIPCLOSE\r\n\0";
	char *http_command = "AT+CIPSEND=19\r\n\0";
	char *http_data = "HEAD / HTTP/1.1\r\n\r\n\0";
	int8_t ret = 0;
	esp8266_Send(http_connect);
	ret = esp8266_WaitForOk(http_connect, 100, 100);
	if (ret)
		return -1;
	esp8266_Send(http_command);
	ret = esp8266_WaitForOk(http_command, 100, 100);
	if (ret) 
		return -2;
	esp8266_Send(http_data);
	delay_ms(2000);
	ret = buffer_MoveTailToLabel(&UART2_receive_buffer, "Date:\r\n\0");
	if (ret)
		return -3;
	ret = buffer_CopyToNearestWord(&UART2_receive_buffer, buf, "GMT\0");
	if (ret)
		return -4;
	parseTime(hour, minute, second, buf);
	buffer_Reset(&UART2_receive_buffer);
	esp8266_Send(http_disconnect);
	ret = esp8266_WaitForOk(http_disconnect, 100, 100);
	if (ret)
		return -5;
	return 0;
}

inline int8_t esp8266_GetIp(char *buf)
{
	return esp8266_SendGetReply("AT+CIFSR\r\n\0", buf, 100, 10);
}

int8_t esp8266_ScanForData(char *buf)
{
	int ret;
	ret = buffer_MoveTailToLabel(&UART2_receive_buffer, "+IPD,\r\n\0"); 
	if (ret) 
		return -1;
	buffer_CopyTillHead(&UART2_receive_buffer, buf);	
	return 0;
}
