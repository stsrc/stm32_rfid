#include "SPI.h"

static SPI_HandleTypeDef spi_1_handler;
static DMA_InitTypeDef dma_init;
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
	
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
	
	return HAL_OK;
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
	SPI_1_DMA_init();
}

static void SPI_handler_basic_init(SPI_HandleTypeDef *spi_handler) 
{
	spi_handler->Instance = SPI1;
	spi_handler->Init.Mode = SPI_MODE_MASTER;
	spi_handler->Init.Direction = SPI_DIRECTION_2LINES;
	spi_handler->Init.DataSize = SPI_DATASIZE_8BIT;
	spi_handler->Init.CLKPolarity = SPI_POLARITY_LOW;
	spi_handler->Init.CLKPhase = SPI_PHASE_1EDGE;
	spi_handler->Init.NSS = SPI_NSS_SOFT;
	spi_handler->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	spi_handler->Init.FirstBit = SPI_FIRSTBIT_MSB;
	spi_handler->Init.TIMode = SPI_TIMODE_DISABLE;
	spi_handler->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
}
static uint16_t SPI_wait_for_EOT(SPI_HandleTypeDef *handler)
{
	uint16_t temp;
	while(handler->Instance->CR2 & SPI_CR2_TXDMAEN);
	while(handler->Instance->CR2 & SPI_CR2_RXDMAEN);
	while(handler->Instance->SR & SPI_SR_RXNE) 
		temp = handler->Instance->DR;
	while(!(handler->Instance->SR & SPI_SR_TXE));
	while(handler->Instance->SR & SPI_SR_BSY);
	return temp;
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
	SPI_handler_basic_init(&spi_1_handler);		
	rt = HAL_SPI_Init(&spi_1_handler);
	if (rt)
		return rt;
	return HAL_OK;
}

HAL_StatusTypeDef SPI_1_send(uint8_t *data)
{
	SPI_wait_for_EOT(&spi_1_handler);
	return HAL_SPI_Transmit(&spi_1_handler, data, 1, 0xFFFF); 
}
