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

/**************************************************************************
* Pins
* ====
*
* 1. VCC support 3.3 ~ 5V
* 2. TX, RX connect to Arduino or Seeeduino
* 3. T1, T2 is the Signal port for RFID antenna
* 4. W0, W1 is for wiegand protocol, but this library not support yet.
* 
* ```
* 		+-----------+
* 	----|VCC	  T1|----
* 	----|GND	  T2|----
* 	----|TX		 SER|----
* 	----|RX		 LED|----	
* 	----|W0		BEEP|----
* 	----|W1		 GND|----
* 		+-----------+
* ```
***************************************************************************/

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

bool SeeedRFID_checkBitValidationUART()
{
	if( 5 == rfid._data.dataLen && (rfid._data.raw[4] == (rfid._data.raw[0]^rfid._data.raw[1]^rfid._data.raw[2]^rfid._data.raw[3])))
	{
		rfid._data.valid = rfid._isAvailable = true;
		return true;
	} else
	{
		rfid._data.valid = rfid._isAvailable = false;
		return false;
	}
}

void SeeedRFID_listen()
{
	USART_1_listen();
}

bool SeeedRFID_read()
{

	rfid._isAvailable = false;

	if (rfid._data.dataLen != 0)
	{
		rfid._data.dataLen = 0;
	}
	SeeedRFID_listen();	
	while (USART_1_available())
	{
		rfid._data.raw[rfid._data.dataLen++] = USART_1_read();

		delay_ms(10);
	}

	return SeeedRFID_checkBitValidationUART();
}

bool SeeedRFID_isAvailable()
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

struct RFIDdata SeeedRFID_data()
{
	if (rfid._data.valid)
	{
		return rfid._data;
	}else{
		// empty data
		struct RFIDdata _tag;
		return _tag;
	}
}

unsigned long SeeedRFID_cardNumber()
{
	// unsigned long myZero = 255;

	unsigned long sum = 0;
	if(0 != rfid._data.raw[0]){
		// _data.raw[0] = 	_data.raw[0] & myZero;
		sum = sum + rfid._data.raw[0];
		sum = sum<<24;
	}
	// _data.raw[1] = 	_data.raw[1] & myZero;
	sum = sum + rfid._data.raw[1];
	sum = sum<<16;

	unsigned long sum2 = 0;
	// _data.raw[2] = 	_data.raw[2] & myZero;
	sum2 = sum2  + rfid._data.raw[2];
	sum2 = sum2 <<8;
	// _data.raw[3] = 	_data.raw[3] & myZero;
	sum2 = sum2  + rfid._data.raw[3];

	sum = sum + sum2;
  return sum;
}
