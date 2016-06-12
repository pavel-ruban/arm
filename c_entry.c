/******************** (C) COPYRIGHT 2016 Pavel Ruban ********************
 * File Name          : c_entry.c
 * Author             : Pavel Ruban
 * Version            : V1.0
 * Date               : 12/06/2016
 * Description        : Main program body
 ********************************************************************************

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stm32f10x_conf.h>
#include <spi_binds.h>
#include <rc522_binds.h>
#include <mfrc522.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
SPI_InitTypeDef SPI_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;
ErrorStatus HSEStartUpStatus;

/* Private function prototypes -----------------------------------------------*/
void RCC_Configuration(void);
void NVIC_Configuration(void);
void Delay(vu32 nCount);
void custom_asm();
void rc522_irq_prepare();
void reset_asm();

void __initialize_hardware_early()
{
	/* Configure the system clocks */
	RCC_Configuration();

	/* NVIC Configuration */
	NVIC_Configuration();
}

void WRONG_IRQ_EXCEPTION()
{
	while (1) {}
}

void EXTI4_IRQHandler()
{
	while (1) {}
}

void rfid_irq_tag_handler()
{
	__disable_irq();
	uint8_t tag_id[4];
	uint8_t status = mfrc522_get_card_serial(tag_id);
	__enable_irq();

	if (status == CARD_FOUND) {
		static uint8_t led_state = 0;
		led_state ? GPIO_SetBits(GPIOA, GPIO_Pin_1) : GPIO_ResetBits(GPIOA, GPIO_Pin_1);
		led_state ^= 1;
	}

	// Activate timer.
	rc522_irq_prepare();
}

void EXTI15_10_IRQHandler()
{
	// Get active interrupts from RC522.
	uint8_t mfrc522_com_irq_reg = mfrc522_read(ComIrqReg);

	// If some PICC answered handle it to retrieve additional data from it.
	if (mfrc522_com_irq_reg & (1 << RxIEn)) {
		//static uint8_t led_state = 0;

		// Acknowledge receive irq.
		mfrc522_write(ComIrqReg, 1 << RxIEn);

		// Attempt to retrieve tag ID and in case of success check node access.
		rfid_irq_tag_handler();

		rc522_irq_prepare();
	}
	// If it's timer IRQ then request RC522 to start looking for CARD again
	// and back control to the main thread.
	else if (mfrc522_com_irq_reg & TimerIEn) {
		// Down timer irq.
		mfrc522_write(ComIrqReg, TimerIEn);

		// As was checked, after Transceive_CMD the FIFO is emptied, so we need put PICC_REQALL
		// here again, otherwise PICC won't be able response to RC522.
		mfrc522_write(FIFOLevelReg, mfrc522_read(FIFOLevelReg) | 0x80); // flush FIFO data
		mfrc522_write(FIFODataReg, PICC_REQALL);

		// Unfortunately Transceive seems not terminates receive stage automatically in PICC doesn't respond.
		// so we again need activate command to enter transmitter state to pass PICC_REQALL cmd to PICC
		// otherwise it won't response due to the ISO 14443 standard.
		mfrc522_write(CommandReg, Transceive_CMD);

		// When data and command are correct issue the transmit operation.
		mfrc522_write(BitFramingReg, mfrc522_read(BitFramingReg) | 0x80);

		// Start timer. When it will end it again cause this IRQ handler to search the PICC.

		// Clear STM32 irq bit.
		EXTI_ClearITPendingBit(EXTI_Line10);

		mfrc522_write(ControlReg, 1 << TStartNow);
		return;
	}

	EXTI_ClearITPendingBit(EXTI_Line10);
}

void interrupt_initialize();

void __initialize_hardware()
{
	/* Enable GPIOC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* Configure PC.4 as Output push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Configure RC522 SPI CS line 12 and reset 11 line.
	GPIO_InitStructure.GPIO_Pin = RC522_CS_PIN | RC522_RESET_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// Set reset line to start MFRC55 chip to work and release it's SPI interface.
	GPIO_SetBits(RC522_GPIO, RC522_RESET_PIN | RC522_CS_PIN);

	// Configure RC522 IRQ pin.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(RC522_GPIO, RC522_IRQ_PIN);

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

	interrupt_initialize();
}

void __reset_hardware()
{
	reset_asm();
}

void rc522_irq_prepare()
{
	mfrc522_write(BitFramingReg, 0x07); // TxLastBists = BitFramingReg[2..0]	???

	// Clear all interrupts flags.
	mfrc522_write(ComIrqReg, (uint8_t) ~0x80);

	// Start timer.
	mfrc522_write(ControlReg, 1 << TStartNow);
}

int main(void)
{
	mfrc522_init();

	// Check if timer started.
	uint8_t status = mfrc522_read(Status1Reg);

	__enable_irq();

	rc522_irq_prepare();

	while (1)
	{
	}
}

void interrupt_initialize()
{
	__disable_irq();

	EXTI_InitTypeDef EXTI_InitStructure;
	// NVIC structure to set up NVIC controller
	NVIC_InitTypeDef NVIC_InitStructure;
	// GPIO structure used to initialize Button pins
	// Connect EXTI Lines to Button Pins
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);

	// Select EXTI line0
	EXTI_InitStructure.EXTI_Line = EXTI_Line10;
	//select interrupt mode
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	//generate interrupt on rising edge
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	//send values to registers
	EXTI_Init(&EXTI_InitStructure);

	// Clear STM32 irq bit.
	EXTI_ClearITPendingBit(EXTI_Line10);

	// Configure NVIC
	// Select NVIC channel to configure
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	// Set priority to lowest
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x08;
	// Set subpriority to lowest
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x08;
	// Enable IRQ channel
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	// Update NVIC registers
	NVIC_Init(&NVIC_InitStructure);

	NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
}

/*******************************************************************************
 * Function Name  : RCC_Configuration
 * Description    : Configures the different system clocks.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void RCC_Configuration(void)
{
	/* RCC system reset(for debug purpose) */
	RCC_DeInit();

	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);

	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if(HSEStartUpStatus == SUCCESS)
	{
		/* Enable Prefetch Buffer */
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

		/* Flash 2 wait state */
		FLASH_SetLatency(FLASH_Latency_2);

		/* HCLK = SYSCLK */
		RCC_HCLKConfig(RCC_SYSCLK_Div1);

		/* PCLK2 = HCLK */
		RCC_PCLK2Config(RCC_HCLK_Div1);

		/* PCLK1 = HCLK/2 */
		RCC_PCLK1Config(RCC_HCLK_Div2);

		/* PLLCLK = 8MHz * 9 = 72 MHz */
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

		/* Enable PLL */
		RCC_PLLCmd(ENABLE);

		/* Wait till PLL is ready */
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}

		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		/* Wait till PLL is used as system clock source */
		while(RCC_GetSYSCLKSource() != 0x08) {}
	}

	/* PCLK2 = HCLK/2 */
	RCC_PCLK2Config(RCC_HCLK_Div2);

	//enable AFIO clock
    	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/* Enable GPIO clock for SPIz */
	RCC_APB2PeriphClockCmd(SPIz_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);

	/* Enable SPIz Periph clock */
	RCC_APB1PeriphClockCmd(SPIz_CLK, ENABLE);
}

/*******************************************************************************
 * Function Name  : NVIC_Configuration
 * Description    : Configures Vector Table base location.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void NVIC_Configuration(void)
{
#ifdef  VECT_TAB_RAM
	/* Set the Vector Table base location at 0x20000000 */
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
	/* Set the Vector Table base location at 0x08000000 */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
}

/*******************************************************************************
 * Function Name  : Delay
 * Description    : Inserts a delay time.
 * Input          : nCount: specifies the delay time length.
 * Output         : None
 * Return         : None
 *******************************************************************************/
void Delay(vu32 nCount)
{
	for(; nCount != 0; nCount--);
}

#ifdef  DEBUG
/*******************************************************************************
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 *******************************************************************************/
void assert_failed(u8* file, u32 line)
{
	/* User can add his own implementation to report the file name and line number,
ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/

