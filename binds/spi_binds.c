#include <stdint.h>
#include <stm32f10x_conf.h>
#include <binds.h>

extern SPI_InitTypeDef SPI_InitStructure;
extern GPIO_InitTypeDef GPIO_InitStructure;

/**
 * Used to bind current Architecture SPI Implementation to libs HAL.
 */
uint8_t spi_transmit(uint8_t byte, uint8_t skip_receive, SPI_TypeDef * SPI_CH)
{
	// Clear flag if not empty.
	if (SPI_I2S_GetFlagStatus(SPI_CH, SPI_I2S_FLAG_RXNE) == SET) {
		SPI_I2S_ReceiveData(SPI_CH);
	}

	/* Wait for SPIy Tx buffer empty */
	while (SPI_I2S_GetFlagStatus(SPI_CH, SPI_I2S_FLAG_TXE) == RESET);

	/* Send SPIz data */
	SPI_I2S_SendData(SPI_CH, byte);

	while (SPI_I2S_GetFlagStatus(SPI_CH, SPI_I2S_FLAG_RXNE) == RESET);

	return SPI_I2S_ReceiveData(SPI_CH);
}

void set_spi_registers()
{
	#ifndef RCC_APB2Periph_SPIz_Enabled
		RCC_APB2PeriphClockCmd(SPIz_CLK, ENABLE);
	#endif

	uint16_t SPIz_Mode = SPI_Mode_Master;

	/* Configure SPIz pins: SCK, MISO and MOSI ---------------------------------*/
	GPIO_InitStructure.GPIO_Pin = SPIz_PIN_SCK | SPIz_PIN_MOSI;
	/* Configure SCK and MOSI pins as Alternate Function Push Pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPIz_GPIO, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = SPIz_PIN_MISO;

	/* Configure MISO pin as Input Floating  */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SPIz_GPIO, &GPIO_InitStructure);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPIz, &SPI_InitStructure);

	/* Enable SPIz */
	SPI_Cmd(SPIz, ENABLE);
}

void set_spi2_registers()
{
	#ifndef RCC_APB2Periph_SPIy_Enabled
		RCC_APB2PeriphClockCmd(SPIy_CLK, ENABLE);
	#endif

	uint16_t SPIy_Mode = SPI_Mode_Master;

	/* Configure SPIy pins: SCK, MISO and MOSI ---------------------------------*/
	GPIO_InitStructure.GPIO_Pin = SPIy_PIN_SCK | SPIy_PIN_MOSI;
	/* Configure SCK and MOSI pins as Alternate Function Push Pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPIy_GPIO, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = SPIy_PIN_MISO;

	/* Configure MISO pin as Input Floating  */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SPIy_GPIO, &GPIO_InitStructure);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPIy, &SPI_InitStructure);

	//GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);
	AFIO->MAPR |= AFIO_MAPR_SPI1_REMAP;
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

	/* Enable SPIy */
	SPI_Cmd(SPIy, ENABLE);
}
