#include <stm32f10x_conf.h>
#include <binds.h>

extern GPIO_InitTypeDef GPIO_InitStructure;

void (*rc522_select)();
void (*rc522_release)();

void rc522_1_select()
{
	__disable_irq();
	// Wait until SPI done the work.
	while (SPI_I2S_GetFlagStatus(SPIz, SPI_I2S_FLAG_BSY) == SET) {}
	GPIO_SetBits(RC522_2_GPIO, RC522_2_CS_PIN);
	GPIO_ResetBits(RC522_GPIO, RC522_CS_PIN);
}

void rc522_1_release()
{
	// Wait until SPI done the work.
	while (SPI_I2S_GetFlagStatus(SPIz, SPI_I2S_FLAG_BSY) == SET) {}
	GPIO_SetBits(RC522_GPIO, RC522_CS_PIN);
	__enable_irq();
}

void rc522_set_pins()
{
	#ifndef RCC_APB2Periph_RC522_Enabled
		RCC_APB2PeriphClockCmd(RC522_GPIO_CLK, ENABLE);
	#endif

	// Configure RC522 SPI CS line 12 and reset 11 line.
	GPIO_InitStructure.GPIO_Pin = RC522_CS_PIN | RC522_RESET_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(RC522_GPIO, &GPIO_InitStructure);

	// Set reset line to start RC522 chip to work and release it's SPI interface.
	GPIO_SetBits(RC522_GPIO, RC522_RESET_PIN | RC522_CS_PIN);

	// Configure RC522 IRQ pin.
	GPIO_InitStructure.GPIO_Pin = RC522_IRQ_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(RC522_GPIO, &GPIO_InitStructure);

	GPIO_ResetBits(RC522_GPIO, RC522_IRQ_PIN);
}

void rc522_2_select()
{
	__disable_irq();
	// Wait until SPI done the work.
	while (SPI_I2S_GetFlagStatus(SPIz, SPI_I2S_FLAG_BSY) == SET) {}
	GPIO_SetBits(RC522_GPIO, RC522_CS_PIN);
	GPIO_ResetBits(RC522_2_GPIO, RC522_2_CS_PIN);
}

void rc522_2_release()
{
	// Wait until SPI done the work.
	while (SPI_I2S_GetFlagStatus(SPIz, SPI_I2S_FLAG_BSY) == SET) {}
	GPIO_SetBits(RC522_2_GPIO, RC522_2_CS_PIN);
	__enable_irq();
}

void rc522_2_set_pins()
{
	#ifndef RCC_APB2Periph_RC522_2_Enabled
		RCC_APB2PeriphClockCmd(RC522_2_GPIO_CLK, ENABLE);
	#endif

	// Configure RC522 SPI CS line 12 and reset 11 line.
	GPIO_InitStructure.GPIO_Pin = RC522_2_CS_PIN | RC522_2_RESET_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(RC522_2_GPIO, &GPIO_InitStructure);

	// Set reset line to start RC522 chip to work and release it's SPI interface.
	GPIO_SetBits(RC522_2_GPIO, RC522_2_RESET_PIN | RC522_2_CS_PIN);

	// Configure RC522 IRQ pin.
	GPIO_InitStructure.GPIO_Pin = RC522_2_IRQ_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(RC522_2_GPIO, &GPIO_InitStructure);

	GPIO_ResetBits(RC522_2_GPIO, RC522_2_IRQ_PIN);
}

void rc522_pcd_select(uint8_t n)
{
	switch (n)
	{
		case RC522_PCD_1:
			rc522_select = rc522_1_select;
			rc522_release = rc522_1_release;
			break;

		case RC522_PCD_2:
			rc522_select = rc522_2_select;
			rc522_release = rc522_2_release;
			break;
	}
}

