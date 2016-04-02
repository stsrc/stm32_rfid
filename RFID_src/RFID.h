/*
 * SeeedRFID.h
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

/*
 * RFID data format:
 *
 * 1st and 2nd byte - control bytes.
 * 
 * from 3rd to 13th byte - card ID number. Each byte contains ASCII sign, which 
 * may be a single digit or letter from A to F (each sign belongs to hexadecimal
 * system). Therefore those 10 bytes makes value written in hex.
 *
 * last 2 bytes - CRC bytes.
 *
 */

#ifndef SeeedRFID_H
#define SeeedRFID_H

#define RFID_DATA_SIZE 14

#include <string.h>
#include <stdint.h>
#include "UART.h"
#include "delay.h"

void RFID_Init();
uint8_t RFID_CardNumber(char* const buf);
uint8_t RFID_Read();

#endif //__SeeedRFID_H__
