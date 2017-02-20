/*    
 *   
 * Copyright (c) 2008-2014 seeed technology inc.  
 * Author      : Ye Xiaobo(yexiaobo@seeedstudio.com)
 * Create Time: 2014/2/20
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "RFID.h"


static char data[RFID_DATA_SIZE];

void RFID_Init()
{
	UART_1_init();
	// init RFID data
	memset(data, 0, RFID_DATA_SIZE);
	RFID_Read();	
}

static uint8_t get_hex_from_ASCII(char input) {
	if (input == '0') return 0;
	else if (input == '1') return 1;
	else if (input == '2') return 2;
	else if (input == '3') return 3;
	else if (input == '4') return 4;
	else if (input == '5') return 5;
	else if (input == '6') return 6;
	else if (input == '7') return 7;
	else if (input == '8') return 8;
	else if (input == '9') return 9;
	else if (input == 'A') return 0x0A;
	else if (input == 'B') return 0x0B;
	else if (input == 'C') return 0x0C;
	else if (input == 'D') return 0x0D;
	else if (input == 'E') return 0x0E;
	else if (input == 'F') return 0x0F;
	else return 0;
}

static uint8_t make_byte_from_ASCII(char first, char second){
	return (get_hex_from_ASCII(first) << 4) | get_hex_from_ASCII(second);
}

static uint8_t RFID_check_CRC() {
	uint8_t CRC_from_data = 0, CRC_from_chunk;
	for (uint8_t i = 0; i < 5; i++)
		CRC_from_data ^= make_byte_from_ASCII(data[1 + 2*i], data[2*i + 1 + 1]); 
	CRC_from_chunk = make_byte_from_ASCII(data[11], data[12]);
	return CRC_from_data == CRC_from_chunk;
}

void RFID_Read()
{
	UART_1_read((uint8_t *)data, RFID_DATA_SIZE);
}

uint8_t RFID_CardNumber(char* const buf)
{	
	uint8_t ret;
	ret = RFID_check_CRC();

	if (ret) { 
		data[12] = '\0';
		strcpy(buf, &data[2]);
	}

	TM_ILI9341_DrawFilledRectangle(10, 80, 239, 120, ILI9341_COLOR_BLACK); 
	TM_ILI9341_Puts(10, 80, data, &TM_Font_7x10, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

	return !ret;
}

