#include "SPI.h"

static SPI_HandleTypeDef spi_1_handler;
static DMA_InitTypeDef dma_init;
static DMA_HandleTypeDef dma_handler;


void SPI_check_return(HAL_StatusTypeDef rt)
{
	if (rt) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);
		while(1);
	}
}

void DMA1_Channel3_IRQHandler() 
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);
	for(int i = 0; i < 250000; i++);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);
	DMA1->IFCR |= DMA_IFCR_CGIF3 | DMA_IFCR_CTEIF3 | DMA_IFCR_CHTIF3 |
		      DMA_IFCR_CTCIF3;
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

HAL_StatusTypeDef SPI_1_DMA_send(uint8_t *data, uint16_t bytes)
{
	HAL_StatusTypeDef rt;
	while(SPI1->SR & SPI_SR_BSY);
	rt = HAL_SPI_Transmit_DMA(&spi_1_handler, data, bytes);	
	if (rt)
		SPI_check_return(rt);
	while(SPI1->SR & SPI_SR_BSY);
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
	while(SPI1->SR & SPI_SR_BSY);
	return HAL_SPI_Transmit(&spi_1_handler, data, 1, 0xFFFF); 
}
