#include <stm32f10x_conf.h>
#include <enc28j60.h>
#include <binds.h>

extern GPIO_InitTypeDef GPIO_InitStructure;

void enc28j60_select()
{
	__disable_irq();
	// Wait until SPI done the work.
	while (SPI_I2S_GetFlagStatus(SPIy, SPI_I2S_FLAG_BSY) == SET);
	GPIO_ResetBits(ETH_GPIO, ETH_CS_PIN);
}

void enc28j60_release()
{
	// Wait until SPI done the work.
	while (SPI_I2S_GetFlagStatus(SPIy, SPI_I2S_FLAG_BSY) == SET);
	GPIO_SetBits(ETH_GPIO, ETH_CS_PIN);
	__enable_irq();
}

void enc28j60_set_pins()
{
	#ifndef RCC_APB2Periph_ETH_Enabled
		RCC_APB2PeriphClockCmd(ETH_GPIO_CLK, ENABLE);
	#endif

	// Configure RC522 SPI CS line 12 and reset 11 line.
	GPIO_InitStructure.GPIO_Pin = ETH_CS_PIN | ETH_RESET_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(ETH_GPIO, &GPIO_InitStructure);

	// Set reset line to start ETH chip to work and release it's SPI interface.
	GPIO_SetBits(ETH_GPIO, ETH_RESET_PIN | ETH_CS_PIN);

	// Configure ETH IRQ pin.
	GPIO_InitStructure.GPIO_Pin = ETH_IRQ_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(ETH_GPIO, &GPIO_InitStructure);

	GPIO_ResetBits(ETH_GPIO, ETH_IRQ_PIN);
}

void eth_send_packet(uint8_t *frame, uint16_t len)
{
	enc28j60_send_packet(frame, len);
}

uint16_t eth_recv_packet(uint8_t *buf, uint16_t buflen)
{
	return enc28j60_recv_packet(buf, buflen);
}

void __enable_enc28j60_irq()
{
	EXTI->IMR |= EXTI_IMR_MR2;
}

void __disable_enc28j60_irq()
{
	EXTI->IMR &= ~EXTI_IMR_MR2;
}
