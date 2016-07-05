#pragma once

#define ETH_GPIO		GPIOA

#define ETH_SPI_CH		SPIy

#define ETH_CS_PIN		GPIO_Pin_4
#define ETH_RESET_PIN		GPIO_Pin_3
#define ETH_IRQ_PIN		GPIO_Pin_2

void eth_select();
void eth_release();
void eth_set_pins();
