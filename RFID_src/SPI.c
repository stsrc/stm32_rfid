#include "SPI.h"

static SPI_HandleTypeDef spi_1_handler;
static SPI_HandleTypeDef spi_2_handler;

void SPI_check_return(HAL_StatusTypeDef rt)
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

HAL_StatusTypeDef SPI_1_init(void)
{
	memset(&spi_1_handler, 0, sizeof(SPI_HandleTypeDef));
	
	spi_1_handler.Instance = SPI1;
	spi_1_handler.Init.Mode = SPI_MODE_MASTER;
	spi_1_handler.Init.Direction = SPI_DIRECTION_2LINES;
	spi_1_handler.Init.DataSize = SPI_DATASIZE_8BIT;
	spi_1_handler.Init.CLKPolarity = SPI_POLARITY_LOW;
	spi_1_handler.Init.CLKPhase = SPI_PHASE_1EDGE;
	spi_1_handler.Init.NSS = SPI_NSS_SOFT;
	spi_1_handler.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	spi_1_handler.Init.FirstBit = SPI_FIRSTBIT_MSB;
	spi_1_handler.Init.TIMode = SPI_TIMODE_DISABLE;
	spi_1_handler.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	
	return HAL_SPI_Init(&spi_1_handler);
}

HAL_StatusTypeDef SPI_1_send(uint8_t *data)
{
	return HAL_SPI_Transmit(&spi_1_handler, data, 1, 0xFFFF); 
}
