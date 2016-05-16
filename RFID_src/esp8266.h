#ifndef _ESP8266_H_
#define _ESP8266_H_

#include <inttypes.h>
#include <stm32f1xx_hal_gpio.h>
#include "UART.h"
#include "delay.h"
#include "secret_password.h"

void esp8266_InitPins(); 
void esp8266_HardReset();
int8_t esp8266_Init(); 
int8_t esp8266_Send(const char *command, size_t data_size);
int8_t esp8266_SendGetReply(const char *command, const char *delimiter, 
			    char *output, unsigned int delay,
			    uint8_t multiplier);
int8_t esp8266_GetReply(const char *command, const char *delimiter, 
			char *output, unsigned int delay, uint8_t multiplier);
int8_t esp8266_WaitForOk(const char *command, unsigned int delay, 
			 uint8_t multiplier);
int8_t esp8266_GetTime(uint8_t *hour, uint8_t *minute, uint8_t *second);
int8_t esp8266_GetIp(char *buf);

int8_t esp8266_MakeAsServer();
int8_t esp8266_WritePage(char *buf, size_t data_size, uint8_t id, uint8_t close);
int8_t esp8266_ScanForFile(char *file, uint8_t *id);
void esp8266_CheckInput(uint8_t data);
int8_t esp8266_CheckResetFlag();
void esp8266_CloseAllChannels();

int8_t esp8266_WriteATCIPSEND(char *data, size_t data_size, uint8_t id);
int8_t esp8266_WriteATCIPCLOSE(char *buf, uint8_t id);

int8_t esp8266_WaitForAck(const char *command, unsigned int delay, uint8_t multiplier); 
void esp8266_ClearResetFlag();
#endif
