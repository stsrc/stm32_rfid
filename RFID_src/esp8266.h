#ifndef _ESP8266_H_
#define _ESP8266_H_

#include <stm32f1xx_hal_gpio.h>
#include <inttypes.h>
#include "UART.h"
#include "delay.h"
#include "secret_password.h"

void esp8266_InitPins(); 
void esp8266_HardReset();
int8_t esp8266_Init(); 
int8_t esp8266_Send(const char *command);
int8_t esp8266_SendGetReply(const char *command, char *output);
int8_t esp8266_GetReply(const char *command, char *output, unsigned int delay, uint8_t multiplier);
int8_t esp8266_WaitForOk(const char *command, unsigned int delay, uint8_t multiplier);
int8_t esp8266_GetTime(uint8_t *hour, uint8_t *minute, uint8_t *second);
#endif
