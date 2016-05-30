TEMPLATEROOT = ..

# compilation flags for gdb

CFLAGS  = -Os -g3
ASFLAGS = -Os -g3

# object files

OBJS = main.o SPI.o tm_stm32f1_ili9341.o tm_stm32f1_fonts.o stm32f1xx_hal_gpio.o 
OBJS += stm32f1xx_hal_spi.o stm32f1xx_hal_dma.o stm32f1xx_hal_rcc.o stm32f1xx_hal_cortex.o 
OBJS += stm32f1xx_hal.o stm32f1xx_hal_uart.o xpt2046.o FatFs/diskio.o FatFs/ff.o FatFs/ff_gen_drv.o 
OBJS += FatFs/sd_diskio.o FatFs/stm3210c_eval_sd.o delay.o stm32f1xx_hal_tim.o 
OBJS += stm32f1xx_hal_tim_ex.o RTC.o   
OBJS += FatFs/option/unicode.o $(STARTUP) 
OBJS += UART.o RFID.o simple_buffer.o esp8266.o TIM2.o
# include common make file

include $(TEMPLATEROOT)/Makefile.common


