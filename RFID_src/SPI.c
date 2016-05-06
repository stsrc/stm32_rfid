#include "SPI.h"

static SPI_HandleTypeDef spi_1_handler, spi_2_handler;
static DMA_HandleTypeDef dma_handler;

void SPI_show_error(HAL_StatusTypeDef rt)
{
	if (rt) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);
		while(1);
	}
}

void DMA1_Channel3_IRQHandler(void) 
{
	HAL_DMA_IRQHandler(&dma_handler);
}

static HAL_StatusTypeDef SPI_1_DMA_init(void)
{
	HAL_StatusTypeDef rt;
	DMA_InitTypeDef dma_init;
	memset(&dma_init, 0, sizeof(DMA_InitTypeDef));
	memset(&dma_handler, 0, sizeof(DMA_HandleTypeDef));

	dma_init.Direction = DMA_MEMORY_TO_PERIPH;
	dma_init.PeriphInc = DMA_PINC_DISABLE; 
	dma_init.MemInc = DMA_MINC_DISABLE;
	dma_init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; 
	dma_init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	dma_init.Mode = DMA_NORMAL;
	dma_init.Priority = DMA_PRIORITY_VERY_HIGH;
	
	dma_handler.Instance = DMA1_Channel3;
	dma_handler.Init = dma_init;
	dma_handler.Parent = &spi_1_handler;
	
	__HAL_RCC_DMA1_CLK_ENABLE();

	rt = HAL_DMA_Init(&dma_handler);
	if (rt) {
		__HAL_RCC_DMA1_CLK_DISABLE();
		return rt;
	}
	
	spi_1_handler.hdmatx = &dma_handler;
	
	HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
	
	return HAL_OK;
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
	/* Enabling SP1*/
	GPIO_InitTypeDef gpio_sck_mosi =
	{
		GPIO_PIN_5 | GPIO_PIN_7,
		GPIO_MODE_AF_PP,
		GPIO_NOPULL,
		GPIO_SPEED_FREQ_HIGH
	};

	GPIO_InitTypeDef gpio_miso = 
	{
		GPIO_PIN_6,
		GPIO_MODE_AF_INPUT,
		GPIO_PULLDOWN,
		GPIO_SPEED_FREQ_HIGH
	};

	__HAL_RCC_SPI1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(GPIOA, &gpio_sck_mosi);
	HAL_GPIO_Init(GPIOA, &gpio_miso);
	SPI_1_DMA_init();

	gpio_sck_mosi.Pin = GPIO_PIN_13 | GPIO_PIN_15;
	gpio_sck_mosi.Mode = GPIO_MODE_AF_PP;
	gpio_sck_mosi.Pull = GPIO_PULLUP;
	gpio_sck_mosi.Speed = GPIO_SPEED_FREQ_HIGH;

 	gpio_miso.Pin = GPIO_PIN_14;
	gpio_miso.Mode = GPIO_MODE_AF_INPUT;
	gpio_miso.Pull = GPIO_PULLUP;
	gpio_miso.Speed = GPIO_SPEED_FREQ_HIGH;

	__HAL_RCC_SPI2_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	HAL_GPIO_Init(GPIOB, &gpio_sck_mosi);
	HAL_GPIO_Init(GPIOB, &gpio_miso);
}

static void SPI_handler_basic_init(SPI_HandleTypeDef *spi_handler, 
				   SPI_TypeDef *inst) 
{
	spi_handler->Instance = inst;
	spi_handler->Init.Mode = SPI_MODE_MASTER;
	spi_handler->Init.Direction = SPI_DIRECTION_2LINES;
	spi_handler->Init.DataSize = SPI_DATASIZE_8BIT;
	spi_handler->Init.CLKPolarity = SPI_POLARITY_LOW;
	spi_handler->Init.CLKPhase = SPI_PHASE_1EDGE;
	spi_handler->Init.NSS = SPI_NSS_SOFT;
	if (inst == SPI1)
		spi_handler->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	else
		spi_handler->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	spi_handler->Init.FirstBit = SPI_FIRSTBIT_MSB;
	spi_handler->Init.TIMode = SPI_TIMODE_DISABLE;
	spi_handler->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
}
static uint16_t SPI_wait_for_EOT(SPI_HandleTypeDef *handler)
{
	while(handler->Instance->CR2 & SPI_CR2_TXDMAEN);
	while(handler->Instance->CR2 & SPI_CR2_RXDMAEN);
	while(!(handler->Instance->SR & SPI_SR_TXE));
	while(handler->Instance->SR & SPI_SR_BSY);
	return 0;
}

HAL_StatusTypeDef SPI_1_DMA_send(uint8_t *data, uint16_t bytes)
{
	HAL_StatusTypeDef rt;

	/* Waiting to disable SPI */
	SPI_wait_for_EOT(&spi_1_handler);

	/* Disabling SPI */
	spi_1_handler.Instance->CR1 &= ~SPI_CR1_SPE;
	
	/* Changing data frame format to 16 bits */
	spi_1_handler.Instance->CR1 |= SPI_CR1_DFF; 
	
	/* Enabling SPI */
	spi_1_handler.Instance->CR1 |= SPI_CR1_SPE;
	
	rt = HAL_SPI_Transmit_DMA(&spi_1_handler, data, bytes);	
	if (rt)
		SPI_show_error(rt);
	
	/* Waiting to disable SPI */
	SPI_wait_for_EOT(&spi_1_handler);
	
	/* Disabling SPI */
	spi_1_handler.Instance->CR1 &= ~SPI_CR1_SPE;
	
	/* Changing data frame format to 8 bits */
	spi_1_handler.Instance->CR1 &= ~SPI_CR1_DFF; 
	
	/* Enabling SPI */
	spi_1_handler.Instance->CR1 |= SPI_CR1_SPE;

	
	return rt;
}

HAL_StatusTypeDef SPI_1_init(void)
{
	HAL_StatusTypeDef rt;
	memset(&spi_1_handler, 0, sizeof(SPI_HandleTypeDef));
	SPI_handler_basic_init(&spi_1_handler, SPI1);		
	rt = HAL_SPI_Init(&spi_1_handler);
	if (rt)
		return rt;
	return HAL_OK;
}

HAL_StatusTypeDef SPI_1_send(uint8_t *data)
{
	HAL_StatusTypeDef rt;
	SPI_wait_for_EOT(&spi_1_handler);
	rt = HAL_SPI_Transmit(&spi_1_handler, data, 1, 0xFFFF); 
	SPI_wait_for_EOT(&spi_1_handler);
	return rt;
}

HAL_StatusTypeDef SPI_1_change_speed(uint32_t *old, uint32_t new)
{
	if ((new > SPI_CR1_BR) || ((new < SPI_CR1_BR_0) & (new != 0)))
		return HAL_ERROR;
	if (old != NULL)
		*old = SPI1->CR1 & SPI_CR1_BR;
	SPI_wait_for_EOT(&spi_1_handler);
	SPI1->CR1 &= ~SPI_CR1_BR;
	SPI1->CR1 |= new;
	return HAL_OK;
}

HAL_StatusTypeDef SPI_1_read(uint8_t *data, uint16_t bytes)
{
	HAL_StatusTypeDef rt;
	SPI_wait_for_EOT(&spi_1_handler);
	rt = HAL_SPI_Receive(&spi_1_handler, data, bytes, 0xFFFF);
	SPI_wait_for_EOT(&spi_1_handler);
	return rt;
}

/* TODO: Unification */
HAL_StatusTypeDef SPI_2_init(void)
{
	HAL_StatusTypeDef rt;
	SPI_handler_basic_init(&spi_2_handler, SPI2);		
	rt = HAL_SPI_Init(&spi_2_handler);
	if (rt)
		return rt;
	return HAL_OK;
}

HAL_StatusTypeDef SPI_2_write(uint8_t *data)
{
	HAL_StatusTypeDef rt = 0;
	SPI_wait_for_EOT(&spi_2_handler);
	do
		rt = HAL_SPI_Transmit(&spi_2_handler, data, 1, 0xFFFF); 
	while ((rt == HAL_TIMEOUT) || (rt == HAL_BUSY));
	return rt;
}

HAL_StatusTypeDef SPI_2_read(uint8_t *data, uint16_t bytes)
{
	HAL_StatusTypeDef rt;
	SPI_wait_for_EOT(&spi_2_handler);
	//rt = HAL_SPI_Receive(&spi_2_handler, data, bytes, 0xFFFF);
	SPI2->DR = 0xff;
	SPI_wait_for_EOT(&spi_2_handler);
	while(!(SPI2->SR & SPI_SR_RXNE));
	*data = SPI2->DR;	
	rt = HAL_OK;
	return rt;
}

