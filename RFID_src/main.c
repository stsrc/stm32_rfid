#include <stdlib.h>
#include <string.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_spi.h>

static inline void test_return(HAL_StatusTypeDef rt)
{
	if (rt) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);
		while(1);
	}
}


void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
	GPIO_InitTypeDef gpio_sck_mosi =
	{
		GPIO_PIN_5 | GPIO_PIN_7,
		GPIO_MODE_AF_PP,
		GPIO_NOPULL,
		GPIO_SPEED_FREQ_MEDIUM
	};
	GPIO_InitTypeDef gpio_miso = 
	{
		GPIO_PIN_6,
		GPIO_MODE_INPUT,
		GPIO_PULLDOWN,
		GPIO_SPEED_FREQ_MEDIUM
	};
	__HAL_RCC_SPI1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(GPIOA, &gpio_sck_mosi);
	HAL_GPIO_Init(GPIOA, &gpio_miso);
}

HAL_StatusTypeDef SPI1_init(SPI_HandleTypeDef *spi_handler)
{
	memset(spi_handler, 0, sizeof(SPI_HandleTypeDef));
	
	spi_handler->Instance = SPI1;
	spi_handler->Init.Mode = SPI_MODE_MASTER;
	spi_handler->Init.Direction = SPI_DIRECTION_2LINES;
	spi_handler->Init.DataSize = SPI_DATASIZE_8BIT;
	spi_handler->Init.CLKPolarity = SPI_POLARITY_LOW;
	spi_handler->Init.CLKPhase = SPI_PHASE_1EDGE;
	spi_handler->Init.NSS = SPI_NSS_SOFT;
	spi_handler->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
	spi_handler->Init.FirstBit = SPI_FIRSTBIT_MSB;
	spi_handler->Init.TIMode = SPI_TIMODE_DISABLE;
	spi_handler->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	
	return HAL_SPI_Init(spi_handler);
}

int main(void)
{
	uint8_t tx = 127, rx;
	HAL_StatusTypeDef rt;
	GPIO_InitTypeDef gpio_str = 
	{
		GPIO_PIN_8 | GPIO_PIN_9,
		GPIO_MODE_OUTPUT_PP,
		GPIO_NOPULL,
		GPIO_SPEED_FREQ_MEDIUM		
	};
	
	SPI_HandleTypeDef spi_handler;

	__HAL_RCC_GPIOC_CLK_ENABLE();

	HAL_GPIO_Init(GPIOC, &gpio_str);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	while(1) {
		for(unsigned int it = 0; it < 500000; it++);
		rt = HAL_SPI_Transmit(&spi_handler, &tx, 1, 0xFF);
		test_return(rt);
		rt = HAL_SPI_Receive(&spi_handler, &rx, 1, 0xFF);
		test_return(rt);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);
		for(unsigned int it = 0; it < 500000; it++);
		if (rx != tx)
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
	}
	return 0;
}
