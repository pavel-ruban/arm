#pragma once

#include <enc28j60.h>
#include <ethernet.h>

#define ETH_GPIO		GPIOA
#define ETH_GPIO_CLK		RCC_APB2Periph_GPIOA

#define ETH_SPI_CH		SPIy

#define ETH_CS_PIN		GPIO_Pin_3
#define ETH_RESET_PIN		GPIO_Pin_4
#define ETH_IRQ_PIN		GPIO_Pin_2

#define ETHERNET_MAXFRAME	ENC28J60_MAXFRAME

void enc28j60_select();
void enc28j60_release();
void enc28j60_set_pins();

extern uint8_t mac_addr[6];
extern uint32_t ip_addr;
extern uint32_t ip_mask;
extern uint32_t ip_gateway;
extern uint8_t net_buf[ETHERNET_MAXFRAME];

