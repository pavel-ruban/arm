#pragma once

#include <mfrc522.h>

#define RC522_GPIO		GPIOB
#define	RC522_GPIO_CLK		RCC_APB2Periph_GPIOB

#define RC522_SPI_CH		SPIz

#define RC522_CS_PIN		GPIO_Pin_12
#define RC522_RESET_PIN		GPIO_Pin_11
#define RC522_IRQ_PIN		GPIO_Pin_10

#define RC522_2_GPIO		GPIOA
#define	RC522_2_GPIO_CLK	RCC_APB2Periph_GPIOA
#define RC522_2_CS_PIN		GPIO_Pin_8
#define RC522_2_RESET_PIN	GPIO_Pin_9
#define RC522_2_IRQ_PIN		GPIO_Pin_11

#define RC522_PCD_1		1
#define RC522_PCD_2		2

extern void (*rc522_select)();
extern void (*rc522_release)();

void rc522_1_select();
void rc522_1_release();
void rc522_set_pins();

void rc522_2_select();
void rc522_2_release();
void rc522_2_set_pins();

void rc522_pcd_select(uint8_t pcd);
