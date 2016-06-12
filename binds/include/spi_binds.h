#ifndef SPI_CONF

#define SPI_CONF

#include <stdint.h>

// Bind SPI to GPIO pins
#define	SPI_I2S_FLAG_TXE	((uint16_t)0x0002)
#define	SPIy			SPI1
#define	SPIy_CLK		RCC_APB2Periph_SPI1
#define	SPIy_GPIO		GPIOA
#define	SPIy_GPIO_CLK		RCC_APB2Periph_GPIOA
#define	SPIy_PIN_SCK		GPIO_Pin_5
#define	SPIy_PIN_MISO		GPIO_Pin_6
#define	SPIy_PIN_MOSI		GPIO_Pin_7
#define	SPIz			SPI2
#define	SPIz_CLK		RCC_APB1Periph_SPI2
#define	SPIz_GPIO		GPIOB
#define	SPIz_GPIO_CLK		RCC_APB2Periph_GPIOB
#define	SPIz_PIN_SCK		GPIO_Pin_13
#define	SPIz_PIN_MISO		GPIO_Pin_14
#define	SPIz_PIN_MOSI		GPIO_Pin_15

#define SKIP_RECEIVE		1
#define RECEIVE_BYTE		0

uint8_t spi_transmit(uint8_t byte, uint8_t);
void rc522_select();
void rc522_release();

#endif
