#include <stm32f10x_conf.h>
#include <spi_binds.h>
#include <rc522_binds.h>

/**
 * Used to bind current Architecture SPI Implementation to libs HAL.
 */
uint8_t spi_transmit(uint8_t byte, uint8_t skip_receive)
{
	// Clear flag if not empty.
	if (SPI_I2S_GetFlagStatus(SPIz, SPI_I2S_FLAG_RXNE) == SET) {
		SPI_I2S_ReceiveData(SPIz);
	}

	/* Wait for SPIy Tx buffer empty */
	while (SPI_I2S_GetFlagStatus(SPIz, SPI_I2S_FLAG_TXE) == RESET);

	/* Send SPIz data */
	SPI_I2S_SendData(SPIz, byte);

	while (SPI_I2S_GetFlagStatus(SPIz, SPI_I2S_FLAG_RXNE) == RESET);

	return SPI_I2S_ReceiveData(SPIz);
}

void rc522_select()
{
	__disable_irq();
	// Wait until SPI done the work.
	while (SPI_I2S_GetFlagStatus(SPIz, SPI_I2S_FLAG_BSY) == SET) {}
	GPIO_ResetBits(RC522_GPIO, RC522_CS_PIN);
}

void rc522_release()
{
	// Wait until SPI done the work.
	while (SPI_I2S_GetFlagStatus(SPIz, SPI_I2S_FLAG_BSY) == SET) {}
	GPIO_SetBits(RC522_GPIO, RC522_CS_PIN);
	__enable_irq();
}
