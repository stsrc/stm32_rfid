#ifndef _ESP8266_H_
#define _ESP8266_H_

#include <stm32f1xx_hal_gpio.h>
#include "UART.h"
#include "delay.h"
#include "secret_password.h"

void esp8266_InitPins(); 
void esp8266_HardReset();
void esp8266_Init(); 
int8_t esp8266_Send(const char *command);
int8_t esp8266_GetReply(const char *command, char *output);
#endif
