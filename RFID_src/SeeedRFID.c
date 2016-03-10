/*    
 * SeeedRFID.cpp
 * A library for RFID moudle.
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

#include "SeeedRFID.h"


enum RFIDType
{
	RFID_UART,
	RFID_WIEGAND
};

struct SeeedRFID
{
	struct RFIDdata _data;
	bool _isAvailable;
	enum RFIDType _type;
};

static struct SeeedRFID rfid;

void SeeedRFID_init()
{
	USART_1_init();
	// init RFID data
	rfid._data.dataLen = 0;
	rfid._data.chk = 0;
	rfid._data.valid = false;

	rfid._isAvailable = false;
	rfid._type = RFID_UART;
}

bool SeeedRFID_read()
{
	rfid._isAvailable = false;

	if (rfid._data.dataLen != 0)
	{
		rfid._data.dataLen = 0;
	}	
	memset(rfid._data.raw, 0, RFID_DATA_LEN);
	while (rfid._data.dataLen < RFID_DATA_LEN - 1)
	{
		while(!USART_1_available());
		rfid._data.raw[rfid._data.dataLen++] = USART_1_read();
	}

	return false;
}

bool SeeedRFID_WaitAndGetData()
{
	switch(rfid._type){
		case RFID_UART:
			return SeeedRFID_read();
			break;
		case RFID_WIEGAND:
			return false;
			break;
		default:
			return false;
			break;
	}
}

char* SeeedRFID_CardNumber()
{
	memset(rfid._data.card_number, 0, 11);	
	strncpy(rfid._data.card_number, &(rfid._data.raw[1]), 10);	
	return rfid._data.raw;
}

