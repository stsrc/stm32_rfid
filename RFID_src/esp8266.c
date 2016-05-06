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
#define AT_CIPSTO		"AT+CIPSTO=7000\r\n\0"
#define AT_CIFSR		"AT+CIFSR\r\n\0"
#define AT_CIPSEND		"AT+CIPSEND=\0"
#define AT_CLOSE_SOCKET		"AT+CIPCLOSE=\0"


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
	delay_ms(500);
	HAL_GPIO_WritePin(ESP8266_RST_PORT, ESP8266_RST_PIN, GPIO_PIN_SET);
	delay_ms(500);	
}

int8_t esp8266_WaitForOk(const char *command, unsigned int delay, uint8_t multiplier) 
{
	int8_t ret;
	char buf[BUF_MEM_SIZE];
	ret = esp8266_GetReply(command, "OK\0", buf, delay, multiplier);
	return ret;	
}

static int8_t esp8266_ConnectToWiFi()
{
	int ret;
	esp8266_Send(AT_CWMODE_1, strlen(AT_CWMODE_1));	
	ret = esp8266_WaitForOk(AT_CWMODE_1, 100, 100);
	if (ret)
		return -1;
	esp8266_Send(WIFI_PASSWD_SECRET, strlen(WIFI_PASSWD_SECRET));
	ret = esp8266_WaitForOk(WIFI_PASSWD_SECRET, 100, 100);
	if (ret)
		return -2;
	esp8266_Send(AT_PING_GOOGLE, strlen(AT_PING_GOOGLE));
	ret = esp8266_WaitForOk(AT_PING_GOOGLE, 100, 100);
	if (ret)
		return -3;
	return 0;
}

int8_t esp8266_MakeAsServer()
{
	int8_t ret;
	ret = esp8266_Send(AT_CIPMODE_0, strlen(AT_CIPMODE_0));
	if (ret)
		return -1;
	ret = esp8266_WaitForOk(AT_CIPMODE_0, 100, 100);
	if (ret)
		return -2;
	ret = esp8266_Send(AT_CIPMUX_1, strlen(AT_CIPMUX_1));
	if (ret)
		return -3;
	ret = esp8266_WaitForOk(AT_CIPMUX_1, 100, 100);
	if (ret)
		return -4;
	ret = esp8266_Send(AT_CIPSERVER, strlen(AT_CIPSERVER));
	if (ret)
		return -5;
	ret = esp8266_WaitForOk(AT_CIPSERVER, 100, 100);
	if (ret)
		return -6;
	ret = esp8266_Send(AT_CIPSTO, strlen(AT_CIPSTO));
	if (ret)
		return -7;
	ret = esp8266_WaitForOk(AT_CIPSTO, 100, 100);
	if (ret)
		return -8;

	return 0;
}

int8_t esp8266_Init(char *global_buf) 
{
	int8_t ret;
	const char * const RST_CMD = AT_RESET_CMD;
	esp8266_InitPins();
	esp8266_HardReset();
	UART_2_init();
	buffer_Reset(&UART2_transmit_buffer);
	delay_ms(5000);
	esp8266_Send(RST_CMD, strlen(RST_CMD));
	delay_ms(5000);
	buffer_Reset(&UART2_receive_buffer);
	ret = esp8266_ConnectToWiFi();
	if (ret)
		return ret;
	return 0;
}

int8_t esp8266_Send(const char *data, size_t data_size) 
{
	int8_t ret;
	ret = buffer_set_text(&UART2_transmit_buffer, data, data_size);
	if (!ret)
		UART_2_transmit();
	return ret;
}

int8_t esp8266_SendGetReply(const char *command, const char *delimiter, 
			    char *output, unsigned int delay,
			    uint8_t multiplier)
{
	int8_t ret;
	if (buffer_IsFull(&UART2_transmit_buffer))
		return -ENOMEM;
	buffer_Reset(&UART2_receive_buffer);		
	esp8266_Send(command, strlen(command));
	ret = esp8266_GetReply(command, delimiter, output, delay, multiplier);	
	return ret;
}

int8_t esp8266_GetReply(const char *command, const char *delimiter, 
			char *output, unsigned int delay, uint8_t multiplier)
{
	int8_t ret, cnt = 0;
	do {
		ret = buffer_SearchGetLabel(&UART2_receive_buffer, command, 
					    delimiter, output);
		if (ret)
			delay_ms(delay);
	} while (((ret == -EBUSY) || (ret == -EINVAL)) && (++cnt < multiplier));
	return ret;
}

static inline void ParseTime(uint8_t *hour, uint8_t *minute, uint8_t *second, 
			     char *buf)
{
	size_t len = strlen(buf);
	unsigned short temp_h, temp_min, temp_sec;
	buf = &buf[len - 1 - 7];
	sscanf(buf, " %hu:%hu:%hu", &temp_h, &temp_min, &temp_sec);
	*hour = (uint8_t)temp_h;
	*minute = (uint8_t)temp_min;
	*second = (uint8_t)temp_sec;	
}	

int8_t esp8266_GetTime(uint8_t *hour, uint8_t *minute, uint8_t *second)
{
	char *http_connect = "AT+CIPSTART=\"TCP\",\"www.google.com\",80\r\n\0";
	char *http_disconnect = "AT+CIPCLOSE\r\n\0";
	char *http_command = "AT+CIPSEND=19\r\n\0";
	char *http_data = "HEAD / HTTP/1.1\r\n\r\n\0";
	int8_t ret = 0;
	char buf[BUF_MEM_SIZE];
	memset(buf, 0, BUF_MEM_SIZE);
	esp8266_Send(http_connect, strlen(http_connect));
	ret = esp8266_WaitForOk(http_connect, 100, 100);
	if (ret)
		return -1;
	esp8266_Send(http_command, strlen(http_command));
	ret = esp8266_WaitForOk(http_command, 100, 100);
	if (ret) 
		return -2;
	esp8266_Send(http_data, strlen(http_data));
	delay_ms(2000);
	ret = buffer_SearchGetLabel(&UART2_receive_buffer, "Date: \0", " GMT\0", buf);
	if (ret)
		return -3;
	ParseTime(hour, minute, second, buf);
	buffer_Reset(&UART2_receive_buffer);
	esp8266_Send(http_disconnect, strlen(http_disconnect));
	ret = esp8266_WaitForOk(http_disconnect, 100, 100);
	if (ret)
		return -4;
	return 0;
}

static inline int8_t esp8266_WriteATCIPSEND(char *data, size_t data_size, uint8_t id) 
{
	char temp[32];
	char temp_2[16];
	int ret;
	memset(temp, 0, 32);
	memset(temp_2, 0, 16);
	sprintf(temp_2, "%u,%u\r\n", (unsigned int)id, (unsigned int)data_size); //TODO BUF OVERFLOW
	strcpy(temp, AT_CIPSEND);
	strcat(temp, temp_2);
	ret = esp8266_Send(temp, strlen(temp));
	if (ret)
		return -1;
	ret = esp8266_WaitForOk(temp, 100, 100);
	if (ret)
		return -2;
	ret = esp8266_Send(data, data_size);
	if (ret)
		return -3;
	ret = esp8266_WaitForOk("SEND\0", 100, 100);
	if (ret)
		return -4;
	return 0;
}

static inline int8_t esp8266_WriteATCIPCLOSE(uint8_t id) 
{
	char temp[4];
	char buf[20];
	int ret;
	memset(temp, 0, sizeof(temp));
	memset(buf, 0, sizeof(buf));
	sprintf(temp, "%u\r\n", id);
	strcpy(buf, AT_CLOSE_SOCKET);
	strcat(buf, temp);
	ret = esp8266_Send(buf, strlen(buf));
	if (ret)
		return -1;
	ret = esp8266_WaitForOk(buf, 100, 100); //TODO BUF OBERFLOW
	if (ret)
		return -2;
	return ret;
}

int8_t esp8266_WritePage(char *buf, size_t data_size, uint8_t id, uint8_t close)
{
	int8_t ret;
	ret = esp8266_WriteATCIPSEND(buf, data_size, id);
	if (ret)
		return -1;
	if (close) {
		ret = esp8266_WriteATCIPCLOSE(id);
		if (ret)
			return -2;
	}
	return 0;	
}

inline int8_t esp8266_GetIp(char *buf)
{
	memset(buf, 0, BUF_MEM_SIZE);
	return esp8266_SendGetReply(AT_CIFSR, "OK\0", buf, 100, 10);
}

int8_t esp8266_ScanForGET(char *file, uint8_t *id)
{
	int8_t ret;
	uint16_t temp_id, temp_len;
	char *temp_buf;
	memset(file, 0, BUF_MEM_SIZE);
	ret = buffer_SearchGetLabel(&UART2_receive_buffer, "+IPD,\0", 
				    "HTTP/\0", file);
	if (ret) 
		return -1;
	temp_buf = malloc(strlen(file) + 1);
	if (!temp_buf)
		return -2;
	memset(temp_buf, 0, strlen(file) + 1);
	sscanf(file, "%hu,%hu:GET /%s ", &temp_id, &temp_len, temp_buf);
	buffer_SetIgnore(&UART2_receive_buffer, temp_len - (strlen("GET /") 
			 + strlen(temp_buf) + strlen(" HTTP/")));
	strcpy(file, temp_buf);
	free(temp_buf);
	*id = temp_id;
	return 0;
}

struct channel_data {
	char buf[5][32];
	char ready[5];
};



static struct channel_data chn_data;
static uint8_t do_it = 0;
int8_t esp8266_ScanForFile(char *file, uint8_t *id)
{
	do_it = 1;
	for (size_t i = 0; i < 5; i++) {
		if (chn_data.ready[i]) {
			strncpy(file, chn_data.buf[i], 32);
			memset(chn_data.buf[i], 0, 32);
			chn_data.ready[i] = 0;
			*id = i;
			return 0;
		}
	}
	return -EINVAL;	
}

int8_t CheckInput(uint8_t data)
{
	static char buffer[20];
	for(size_t i = 0; i < 18; i++)
		buffer[i] = buffer[i+1];
	if(!strncmp(buffer, "reset", 5))
		return 1;
	else if(!strncmp(buffer, "+IPD", 4))
		return 2;
	return 0;
}

void esp8266_CheckInput(uint8_t data)
{
	static uint8_t state = 0;
	static uint16_t id = 0;
	static uint16_t len = 0;
	static char buf[50];
	static uint8_t cnt = 0;
	if (!do_it)
		return;
	switch(state){
	case 0:
		len = 0;
		id = 0;
		cnt = 0;
		memset(buf, 0, 50);
		if (data == '+')
			state = 1;
		break;
	case 1:
		if (data == 'I')
			state = 2;
		else
			state = 0;
		break;
	case 2:
		if (data == 'P')
			state = 3;
		else 
			state = 0;
		break;
	case 3:
		if (data == 'D')
			state = 4;
		else 
			state = 0;
		break;
	case 4:
		if (data == ',')
			state = 5;
		else
			state = 0;
		break;
	case 5:
		id = data - 48;
		state = 6;
		break;
	case 6:
		if (data == ',')
			state = 7;
		else
			state = 0;
		break;
	case 7:	
		if (data != ':') {
			buf[cnt] = data;
			cnt++;
		} else {
			sscanf(buf, "%hu", &len);
			state = 8;
			buffer_SetIgnore(&UART2_receive_buffer, len);
			cnt = 0;
			memset(buf, 0, 50);
		}
		break;
	case 8:
		len--;
		if (data == ' ')
			state = 9;
		break;
	case 9:
		len--;
		if(data == ' ') {
			state = 10;
		} else {
			buf[cnt] = data;
			cnt++;
		}
		break;
	case 10:
		len--;
		strncpy(chn_data.buf[id], buf, 31);
		state = 11;
		break;
	case 11:
		len--;
		if (len == 0) {
			state = 0;
			chn_data.ready[id] = 1;
		}
	       break;	
	default:
		break;
	}	
}
