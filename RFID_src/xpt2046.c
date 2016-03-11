#include "xpt2046.h"

#ifndef XPT2046_CS_PIN
#define XPT2046_CS_PORT GPIOA
#define XPT2046_CS_PIN	GPIO_PIN_1
#endif /* XPT2046_CS_PIN */

#ifndef XPT2046_EXTI_PIN
#define XPT2046_EXTI_PORT	GPIOC
#define XPT2046_EXTI_PIN	GPIO_PIN_3
#endif /* XPT2046_EXTI_PIN */

#ifndef XPT2046_CS_SET
#define XPT2046_CS_SET		HAL_GPIO_WritePin(XPT2046_CS_PORT, XPT2046_CS_PIN, GPIO_PIN_SET)
#define XPT2046_CS_RESET	HAL_GPIO_WritePin(XPT2046_CS_PORT, XPT2046_CS_PIN, GPIO_PIN_RESET)
#endif /* PT2046_CS_SET */ 

struct cmd_input {
	uint8_t START;
	uint8_t A2;
	uint8_t A1;
	uint8_t A0; 
	uint8_t MODE; 
	uint8_t SER_DFR;
	uint8_t PID1;
	uint8_t PID0;
};

void EXTI3_IRQHandler(){
	TIM2_cnt = 0;
	TM_ILI9341_DisplayOn();
	HAL_NVIC_DisableIRQ(EXTI3_IRQn);
	EXTI->PR |= EXTI_PR_PR3;
}

static uint8_t touch_generate_command(struct cmd_input *in)
{
	uint8_t ret = 0;
	if (in->START)
		ret = 1 << 7;
	if (in->A2)
		ret |= 1 << 6;
	if (in->A1)
		ret |= 1 << 5;
	if (in->A0)
		ret |= 1 << 4;
	if (in->MODE)
		ret |= 1 << 3;
	if (in->SER_DFR)
		ret |= 1 << 2;
	if (in->PID1)
		ret |= 1 << 1;
	if (in->PID0)
		ret |= 1;
	return ret;
}

static uint16_t touch_generate_short(uint8_t byte_1, uint16_t byte_2)
{
	uint16_t ret = byte_1 << 4;
	ret |= byte_2 >> 4;
	return ret;
}

static void touch_calculate_pos(uint16_t input, uint16_t *val, uint16_t max)
{
	float temp = ((float)input)/2000.0f;
	temp = ((float)max)*temp;
	*val = (uint16_t)temp; 
}

static inline void xpt2046_transmit(uint8_t *cmd, uint8_t *rx, uint16_t bytes)
{
	uint32_t old_speed, new_speed = SPI_BAUDRATEPRESCALER_64;
	SPI_1_change_speed(&old_speed, new_speed);
	HAL_NVIC_DisableIRQ(EXTI3_IRQn);
	XPT2046_CS_RESET;	
	SPI_1_send(cmd);
	SPI_1_read(rx, bytes);
	XPT2046_CS_SET;
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	SPI_1_change_speed(NULL, old_speed);	
}

void xpt2046_init(void)
{
	struct cmd_input inp = {
		.START = 1,
		.A2 = 1,
		.A1 = 0,
		.A0 = 1,
		.MODE = 0,
		.SER_DFR = 0,
		.PID1 = 0,
		.PID0 = 0
	};
	GPIO_InitTypeDef gpio_cs = 
	{
		GPIO_PIN_1,
		GPIO_MODE_OUTPUT_PP,
		GPIO_NOPULL,
		GPIO_SPEED_FREQ_MEDIUM
	};
	GPIO_InitTypeDef gpio_irq = 
	{
		GPIO_PIN_3,
		GPIO_MODE_IT_FALLING,
		GPIO_NOPULL,
		GPIO_SPEED_FREQ_HIGH
	};	
	uint8_t rx[3];
	uint8_t cmd;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	HAL_GPIO_Init(GPIOA, &gpio_cs);
	HAL_GPIO_Init(GPIOC, &gpio_irq);
	HAL_NVIC_SetPriority(EXTI3_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	cmd = touch_generate_command(&inp);
	xpt2046_transmit(&cmd, rx, 3);
}

int xpt2046_read(uint16_t *x, uint16_t *y, uint16_t *z) 
{
	struct cmd_input inp = {
		.START = 1,
		.A2 = 1,
		.A1 = 0,
		.A0 = 1,
		.MODE = 0,
		.SER_DFR = 0,
		.PID1 = 0,
		.PID0 = 0
	};
	uint16_t value;
	uint8_t rx[3];
	uint8_t cmd;
	cmd = touch_generate_command(&inp);
	xpt2046_transmit(&cmd, rx, 2);
	value = touch_generate_short(rx[0], rx[1]);
	touch_calculate_pos(value, x, 200);
	inp.A2 = 0;
	cmd = touch_generate_command(&inp);
	xpt2046_transmit(&cmd, rx, 2);
	value = touch_generate_short(rx[0], rx[1]);
	touch_calculate_pos(value, y, 200);
	inp.A1 = 1;
	cmd = touch_generate_command(&inp);
	xpt2046_transmit(&cmd, rx, 2);
	value = touch_generate_short(rx[0], rx[1]);
	*z = value;
	inp.A2 = 1;
	inp.A1 = 0;
	inp.A0 = 0;
	cmd = touch_generate_command(&inp);
	xpt2046_transmit(&cmd, rx, 2);
	value = touch_generate_short(rx[0], rx[1]);
	value = 2050 - value;
	*z = (*z + value)/2;
	touch_calculate_pos(*z, z, 200);
	return 0;
}
