#include <stm32f10x_conf.h>
#include <spi_binds.h>

/**
 * Used to bind current Architecture SPI Implementation to libs HAL.
 */
uint8_t spi_transmit(uint8_t byte)
{
	/* Wait for SPIy Tx buffer empty */
	while (SPI_I2S_GetFlagStatus(SPIz, SPI_I2S_FLAG_TXE) == RESET);

	/* Send SPIz data */
	SPI_I2S_SendData(SPIz, byte);

	//return SPI_I2S_SendData
}

void rc522_select()
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);
}

void rc522_release()
{
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
}
