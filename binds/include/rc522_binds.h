#pragma once

#include <mfrc522.h>

#define RC522_GPIO		GPIOB
#define	RC522_GPIO_CLK		RCC_APB2Periph_GPIOB

#define RC522_SPI_CH		SPIz

#define RC522_CS_PIN		GPIO_Pin_12
#define RC522_RESET_PIN		GPIO_Pin_11
#define RC522_IRQ_PIN		GPIO_Pin_10

void rc522_select();
void rc522_release();
void rc522_set_pins();
