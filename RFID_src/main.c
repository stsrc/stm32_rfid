#include <stdlib.h>
#include <string.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_spi.h>
#include "tm_stm32f1_ili9341.h"
#include "xpt2046.h"
#include "FatFs/ff.h"
#include "FatFs/sd_diskio.h"
#include "RFID.h"
#include "RTC.h"
#include "delay.h"
#include "esp8266.h"
#include "TIM2.h"
#include <string.h>

/**
 * @addtogroup RFID_System libraries
 * @{
 */

/**
 * @defgroup main
 * @brief main functions
 * @{
 */

/**
 * @brief function sets priorites for used interrupts handlers.
 */
void SetInterrupts()
{
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 12, 0);
	HAL_NVIC_SetPriority(EXTI3_IRQn, 15, 0);
	HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
	HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
	HAL_NVIC_SetPriority(RTC_IRQn, 14, 0);
	HAL_NVIC_SetPriority(TIM2_IRQn, 13, 0);
}

/**
 * @brief Date printing function.
 *
 * If 1 second has elapsed, new data is printed on to the screen. If not - function returns.
 */
void PrintDate() 
{
	uint8_t month, day, hour, min, sec;
	uint16_t year;
	char buf[25];
	memset(buf, 0, sizeof(buf));
	if(RTC_GetDate(&year, &month, &day, &hour, &min, &sec)) {
		sprintf(buf, "%02u.%02u.%04hu %02u:%02u:%02u", day, month, year, hour, min, sec);
		TM_ILI9341_Puts(10, 20, buf, &TM_Font_7x10, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
	}
}

inline static void LcdWrite(char *buf, size_t x, size_t y) {
	TM_ILI9341_DisplayOn();
	TIM2_TurnOffLCDAfterTimeInterval(10);
	TM_ILI9341_Puts(x, y, buf, &TM_Font_7x10, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
}

inline static void LcdClear() 
{
	TM_ILI9341_DrawFilledRectangle(0, 0, 319, 239, ILI9341_COLOR_BLACK);
}

/**
 * @brief Functions stops execution of program if parameter ret is not 0.
 * @param ret test parameter
 */
void CheckError(char * message, int ret)
{
	if (ret) {
		LcdWrite(message, 10, 10);
		while(1);
	}
}

/**
 * @brief Function downloads time and sets RTC.
 */
int8_t UpdateTime() 
{
	int ret;
	uint8_t hour, minute, second, day, month;
	uint16_t year;
	ret = esp8266_GetDate(&day, &month, &year, &hour, &minute, &second);
	if (ret)
		return ret;
	RTC_SetDate(year, month, day, hour + 2, minute, second);	
	return 0;
}

/**
 * @brief Function gets IP and prints it on a screen.
 */
void PrintIp()
{
	char buf[BUF_MEM_SIZE];
	int8_t ret;
	memset(buf, 0, BUF_MEM_SIZE);
	ret = esp8266_GetIp(buf);
	CheckError("GetIp Failed!\n", ret);
	LcdWrite(buf, 0, 40);	
}

/**
 * @brief FATFS library initalization function.
 *
 * @param SDFatFs pointer to FATFS structure
 * @param path root path of SD card.
 */
int8_t FATFS_Init(FATFS *SDFatFs, char *path)
{
	int8_t ret;
	ret = FATFS_LinkDriver((Diskio_drvTypeDef *)&SD_Driver, path);
	if (ret)
		return -1;
	ret = f_mount(SDFatFs, path, 1);
	if (ret)
		return -2;
	return ret;
}

/**
 * @brief Files format check function. 
 *
 * Action, which will server take (change RFID permission, add new card, etc.), is based on 
 * type and name of requested file. This function returns specific value, which fits to only
 * specific type of action.
 *
 * @param buf pointer to buffer which contains file name (with possible 
 * parameters if it is php call).
 * @retval int8_t 0 if html file is requested, 1 if there is a request to change settings, 
 * or 2 if new card will be added to system.
 */
int8_t CheckFormat(char *buf)
{
	char *ptr = strstr(buf, "z.html?");
	if (ptr) 
		return 1;
	
	ptr = strstr(buf, "w.html");

	if (ptr) 
		return 2;
		
	return 0;
}

/**
 * @brief Function to change RFID settings.
 *
 * Functions opens a text file with list of RFID, than changes permission of a card, and
 * saves result to SD card.
 *
 * @param buf buffer with ID and new permission.
 * @retval int8_t 0 on success, anything else on failure.
 */
int8_t ChangeRFIDSettings(char *buf)
{
	FIL file;
	int8_t ret = 0;
	char id[16];
	uint16_t cnt;
	char mod;
	char *temp;
	uint bytes_read;

	memset(id, 0, sizeof(id));
	temp = strstr(buf, "ID_List=");
	temp += strlen("ID_List=");
	temp = strtok(temp, "&");
	strcpy(id, temp);
	temp += strlen(id) + strlen("&MOD_List=");
	mod = *temp;
	
	ret = f_open(&file, "ID_list.txt", FA_OPEN_EXISTING | FA_READ | FA_WRITE);
	if (ret)
		return -1;

	memset(buf, 0, BUF_MEM_SIZE);
	ret = f_read(&file, buf, BUF_MEM_SIZE - 1, &bytes_read);
	if (ret) {
		f_close(&file);
		return -2;
	}
	
	sscanf(id, "%hu", &cnt);
	temp = buf;
	
	temp += 11 + 13 * cnt;
	*temp = mod;
	ret = f_lseek(&file, 0);
	if (ret) {
		f_close(&file);
		return -3;
	}

	ret = f_puts(buf, &file);
	if (ret < 0) {
		f_close(&file);
		return -4;
	}

	f_close(&file);	
	return 0;
}

/**
 * @brief Function transmits page to esp8266.
 *
 * It is the main point of sending answer to the client. It opens a file 
 * with a specific webpage, reads it, and sends to client.
 * @param buf buffer which will be used (with size which equals to BUF_MEM_SIZE)
 * @param id ID of channel to which page is transmitted.
 * @retval int8_t 0 on success, negative value on fail.
 */
int8_t SendPage(char *buf, const uint8_t id) 
{
	int8_t ret;
	uint8_t cnt = 0;
	FIL html_file;
	size_t bytes_read, to_read;
	volatile size_t file_size = 0;

	ret = f_open(&html_file, buf, FA_OPEN_EXISTING | FA_READ);
	if (ret) {
		ret = f_open(&html_file, "index.html", FA_OPEN_EXISTING | FA_READ);
		if (ret) {
			esp8266_WriteATCIPCLOSE(buf, id);
			return -1;
		}
	}

	file_size = f_size(&html_file);

	while(file_size) {
		to_read = (file_size > BUF_MEM_SIZE - 1) ? (BUF_MEM_SIZE - 1) : file_size;

		ret = f_read(&html_file, buf, to_read, 
			     (UINT *)&bytes_read);
		
		if (ret) {
			esp8266_WriteATCIPCLOSE(buf, id);
			f_close(&html_file);
			return -2;
		}
		
		file_size -= bytes_read;

		buf[bytes_read] = '\0';
		
		if (cnt) {
			ret = esp8266_WaitForAck(id, "SEND\0", 100, 10);
			if (ret) {
				f_close(&html_file);
				return -3;
			}
		} else {
			cnt++;
		}

		ret = esp8266_WriteATCIPSEND(buf, bytes_read, id);
		if (ret) {
			f_close(&html_file);
			return -4;
		}
	}
	
	ret = esp8266_WaitForAck(id, "SEND\0", 100, 10);
	if (ret) {
		f_close(&html_file);
		return -5;
	}

	ret = esp8266_WriteATCIPCLOSE(buf, id);

	ret = f_close(&html_file);
	if (ret)
		return -6;

	return 0;
}


/**
 * @brief Function adds new RFID card to system.
 *
 * Function used when new card is entered to the system. It opens list of cards in system,
 * checks if there already exists specific card. If not, it enters new card and sets permision
 * of card to unknown.
 * @param buf ID of new card.
 * @retval int8_t 0 on success, negative value on error.
 */
static int8_t AddNewRFIDCard(char *buf) 
{
	char temp[11];
	FIL file;
	int8_t ret; 
	uint bytes_cnt;
	char *ptr;
	
	CLEAR_BIT(UART_1_flag, ready_bit);

	while(!(READ_BIT(UART_1_flag, ready_bit))) {
		delay_ms(100);
	}

	CLEAR_BIT(UART_1_flag, ready_bit);
	
	RFID_CardNumber(temp); 

	ret = f_open(&file, "ID_list.txt", FA_OPEN_EXISTING | FA_READ | FA_WRITE);
	if (ret)
		return -1;

	
	memset(buf, 0, BUF_MEM_SIZE);
	ret = f_read(&file, buf, BUF_MEM_SIZE - 1, &bytes_cnt);
	if (ret) {
		f_close(&file);
		return -2;
	}
	
	ptr = strstr(buf, temp);
	if (ptr) {
		f_close(&file);
		return 0;
	}
	
	ret = f_lseek(&file, 0);
	
	strcat(buf, ";");
	strcat(buf, temp);
	strcat(buf, ";2");
	
	ret = f_write(&file, buf, strlen(buf), &bytes_cnt);
	f_close(&file);
	if (ret < 0) {
		return -1;	
	}
	
	TIM2_TurnOnRFIDAfterTimeInterval(1);
	return 0;
}

/**
 * @brief Server decision function.
 *
 *
 * It checks if there is pending HTTP request, if yes, function checks what 
 * kind of page is pending (is it GET, or changing permission of a RFID card).
 * Then it takes action specific to the type of request.
 * @param buf global buffer.
 * @retval int8_t 0 on success, negative value on error.
 */
int8_t PageRequest()
{
	uint8_t id;
	int8_t ret;
	char buf[BUF_MEM_SIZE];
	ret = esp8266_ScanForFile(buf, &id);
	if (ret)
		return 0;

	ret = CheckFormat(buf);
	if (ret) {
		if (ret == 1) {
			ChangeRFIDSettings(buf);
			strcpy(buf, "zarzadzaj.html");
		} else if (ret == 2) {
			AddNewRFIDCard(buf);
			strcpy(buf, "zarzadzaj.html");
		} else 
			strcpy(buf, "index.html");
	}
	return SendPage(buf, id);
}

static int8_t WiFi_Init()
{
	int ret = 0;
	const size_t max = 5;
	for (size_t i = 0; i < max; i++) {
		LcdWrite("esp8266 init...", 0, 0);
		ret = esp8266_Init();
		if (!ret) {
			break;
		} else if (i != max - 1) {
			LcdWrite("Problems with esp8266 init.\n"
				 "Retrying in 3 sec.", 0, 20);
			delay_ms(3000);
			LcdClear();
		}
	}
	LcdClear();
	CheckError("esp8266 init fail.", ret);
	return ret;
}

static void Server_Init() 
{
	int cnt = 0;
	int8_t ret;
	do {
		LcdWrite("Server initalization...", 0, 0);
		ret = esp8266_MakeAsServer();
		if (ret) {
			LcdWrite("Server init failed! Retrying in 3 sec.\0", 0, 20);
			delay_ms(3000);
			LcdClear();
		}
		cnt++;
	} while (ret && cnt < 3);
	LcdClear();
	CheckError("Server init failed!", ret);
}

/**
 * @brief Function checks if esp8266 has lost connection to Wi-Fi. 
 *
 * If esp8266 has lost connection, hardware and software reset is performed,
 * connection to WiFi is established again, and esp8266 again becomes server,
 * IP is printed and reset flag is cleared.
 */
void CheckWiFi() 
{
	if(esp8266_CheckResetFlag()) {
		TM_ILI9341_DrawFilledRectangle(0, 0, 239, 319, ILI9341_COLOR_BLACK);
		LcdWrite("esp8266 reset occured!", 0, 0);
		LcdWrite("esp8266 reinit in 3 sec!", 0, 10);
		delay_ms(3000);
		TM_ILI9341_DrawFilledRectangle(0, 0, 239, 319, ILI9341_COLOR_BLACK);
		
		WiFi_Init();	
		Server_Init();
		PrintIp();
		
		esp8266_ClearResetFlag();
		
		if (buffer_IsFull(&UART2_receive_buffer)) {
			buffer_Reset(&UART2_receive_buffer);
		}
	}
}

/**
 * @brief Function saves sensed ID to SD card. 
 *
 * Function saves ID and date, when it happend.
 *
 * @param buf global buffer.
 * @param temp buffer with ID written in ASCII.
 * @param temp_len size of temp buffer. 
 */
static int8_t SaveRFIDToHistory(char *buf, const char *temp, size_t temp_len)
{
	FIL file;
	int8_t ret = 0;
	uint8_t month, day, hour, min, sec;
	uint16_t year;
	unsigned int bytes_written;

	RTC_GetDate(&year, &month, &day, &hour, &min, &sec);
	sprintf(buf, "%02u.%02u.%04hu,%02u:%02u:%02u,%s;", day, month, year, hour, min, sec, temp);
	
	ret = f_open(&file, "history.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
	if (ret)
		return -1;

	ret = f_lseek(&file, f_size(&file));
	if (ret) {
		f_close(&file);
		return -2;
	}
	
	ret = f_write(&file, buf, strlen(buf), &bytes_written);
		
	f_close(&file);

	return 0;
}

/**
 * @brief Function presents rfid permission on a screen.
 *
 * Function opens file with permissions, and depending on a permission,
 * prints specific comment and puts color which is suitable for permission
 * (green - allowed, red - disallowed, red - unknown state).
 *
 * @param buf pointer to global buffer.
 * @param RFID_ID buffer with last sensed ID.
 * @retval int8_t 0 on success, negative value on error.
 */
static int8_t PresentRFIDPermission(char *buf, const char *RFID_ID)
{
	FIL file;
	int8_t ret; 
	uint bytes_read;
	char *temp; 

	ret = f_open(&file, "ID_list.txt", FA_OPEN_EXISTING | FA_READ);
	if (ret)
		return -1;

	
	memset(buf, 0, BUF_MEM_SIZE);
	ret = f_read(&file, buf, BUF_MEM_SIZE - 1, &bytes_read);
	if (ret) {
		f_close(&file);
		return -2;
	}

	temp = strstr(buf, RFID_ID);
	if (!temp) {
		TM_ILI9341_DrawFilledRectangle(0, 100, 239, 319, ILI9341_COLOR_RED);
		LcdWrite("Brak karty w systemie!", 0, 100);
	} else {
		temp += 11;
		if (*temp == '0') {
			TM_ILI9341_DrawFilledRectangle(0, 100, 239, 319, 
						      ILI9341_COLOR_RED);
			LcdWrite("Brak dostepu!", 0, 100); 
		} else if (*temp == '1') {
			TM_ILI9341_DrawFilledRectangle(0, 100, 239, 319, 
						      ILI9341_COLOR_GREEN);
			LcdWrite("Prosze wejsc.", 0, 100);
		}
	} 

	f_close(&file);
	return 0;
}

/**
 * @brief Function checks if there RFID card was sensed.
 *
 * Function checks global flag which contains such information. 
 * If it contains it, function checks CRC of sensed ID. If it is wrong,
 * it gives information on a screen. If it is good, function display
 * information about card (if it is allowed).
 *
 * @retval int8_t 0 on success, 1 in there was failure with CRC.
 */
static int8_t CheckNewRFID()
{
	int8_t ret = 0; 
	char temp[50];
	char buf[BUF_MEM_SIZE];

	if (READ_BIT(UART_1_flag, error_bit)) {
		CLEAR_BIT(UART_1_flag, error_bit);	
		TIM2_TurnOnRFIDAfterTimeInterval(1);
	} else if (READ_BIT(UART_1_flag, ready_bit)) {
		CLEAR_BIT(UART_1_flag, ready_bit);
		memset(buf, 0, BUF_MEM_SIZE);
		TM_ILI9341_DisplayOn();
		ret = RFID_CardNumber(temp);
		if (ret) {
			TM_ILI9341_DrawFilledRectangle(0, 100, 239, 319, ILI9341_COLOR_RED);
			LcdWrite("Nie mozna odczytac karty!", 0, 100);
		} else {
			SaveRFIDToHistory(buf, temp, sizeof(temp));	
			PresentRFIDPermission(buf, temp); 
		}
		TIM2_TurnOnRFIDAfterTimeInterval(4);
		TIM2_ClearLCDAfterTimeInterval(5);
		TIM2_TurnOffLCDAfterTimeInterval(10);
		TM_ILI9341_DisplayOn();
	}  
	return ret;	
}

/**
 * @brief Function initalizes SD card and FatFS. 
 *
 * If it fail, it hangs program.
 */
static void SD_Init() 
{
	FATFS SDFatFs;
	int cnt = 0;
	char org[5];
	int8_t ret;
	do {
		LcdWrite("FATFS init...\0", 0, 0); 
		ret = FATFS_Init(&SDFatFs, org);
		if(ret) {
			LcdWrite("FATFS initalize failed! Retrying in 3 sec.\0", 0, 20);
			delay_ms(3000);
			LcdClear();
		}
		cnt++;
	} while (ret && cnt < 3);
	LcdClear();	
	CheckError("FATFS initalization failed!", ret);
}

/**
 * @brief Function downloads actual time from the Internet. 
 *
 * If it fails, it hangs program.
 */
static void Clock_Init() 
{
	int cnt = 0;
	int8_t ret;	
	do {
		LcdWrite("Downloading actual time...", 0, 0);
		ret = UpdateTime();
		if (ret) {
			LcdWrite("Download failed! Retrying in 3 sec.\0", 0, 20);
			delay_ms(3000);
			LcdClear();
		}
		cnt++;
	} while (ret && cnt < 3);
	LcdClear();
	CheckError("Time download failed!", ret);
}

int main(void)
{

	SetInterrupts();
	TIM2_Init();
	TM_ILI9341_Init();
	xpt2046_Init();
	delay_init();
	RFID_Init();
	RTC_Init();

	SD_Init();
	WiFi_Init();
	Clock_Init();	

	Server_Init();
	PrintIp();

	while(1) {
		PageRequest();
		PrintDate();
		CheckWiFi();
		CheckNewRFID();	
	}
	return 0;
}
