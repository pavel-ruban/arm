#include <stm32f10x_conf.h>
#include "enc28j60.h"
#include <binds.h>

GPIO_InitTypeDef GPIO_InitStructure;

void eth_select()
{
	// Wait until SPI done the work.
	while (SPI_I2S_GetFlagStatus(SPIy, SPI_I2S_FLAG_BSY) == SET);
	GPIO_ResetBits(ETH_GPIO, ETH_CS_PIN);
}

void eth_release()
{
	// Wait until SPI done the work.
	while (SPI_I2S_GetFlagStatus(SPIy, SPI_I2S_FLAG_BSY) == SET);
	GPIO_SetBits(ETH_GPIO, ETH_CS_PIN);
}

void eth_set_pins()
{
	#ifndef RCC_APB2Periph_ETH_Enabled
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	#endif

	// Configure RC522 SPI CS line 12 and reset 11 line.
	GPIO_InitStructure.GPIO_Pin = ETH_CS_PIN | ETH_RESET_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// Set reset line to start ETH chip to work and release it's SPI interface.
	GPIO_SetBits(ETH_GPIO, ETH_RESET_PIN | ETH_CS_PIN);

	// Configure ETH IRQ pin.
	GPIO_InitStructure.GPIO_Pin = ETH_IRQ_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(ETH_GPIO, &GPIO_InitStructure);

	GPIO_ResetBits(ETH_GPIO, ETH_IRQ_PIN);
}
