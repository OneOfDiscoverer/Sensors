#include "stm32f0xx.h"
#include "ad7091.h"



extern uint16_t regValues[24];

void SpiDelay(uint32_t n) {
	volatile uint32_t t = n;
	while(t--);
}

/*
	Configure SPI for AD7091 and init AD7091
	PA0 - HS_ALERT		IF
	PA1 - HS_CONVST		GP
	PA4 - AD_CS			GP
	PA5 - SCK			AF0
	PA6 - MISO			AF0
	PA7 - MOSI			AF0
*/
void AD7091_Config(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	
	GPIOA->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
	GPIOA->MODER |= GPIO_MODER_MODER1_0 | GPIO_MODER_MODER4_0;										// PA1, PA4 - GP
	GPIOA->MODER |= GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;				// PA5..PA7 - AF
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFRL5_Msk | GPIO_AFRL_AFRL6_Msk | GPIO_AFRL_AFRL7_Msk);			// PA5..7 - AF0
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5 | GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7;		// PA5..7 - Max speed
	
	SPI1->CR1 = 0;
	SPI1->CR2 = SPI_CR2_DS; 																		// 16-bit
	SPI1->CR1 = SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_MSTR | SPI_CR1_SPE;				// 1.5MHz, Master
	
	AD7091_DeSelect();
	AD7091_ConvstSet();
	SpiDelay(10000l);
	
	//	Power-on initialization
	for (int i=0; i < 66; i++) {
		AD7091_ConvstClr();
		SpiDelay(15);
		AD7091_ConvstSet();
		SpiDelay(15);
	}
	SpiDelay(1000);
	AD7091_Select();
	AD7091_WriteReg(REG_CHANNEL_ADDR, 0x3f);		// Enable only CH0..CH5
	AD7091_DeSelect();
	SpiDelay(1000);
	AD7091_Select();
	AD7091_WriteReg(REG_CONFIG_ADDR, (1UL << 9) );
	SpiDelay(1000);
	AD7091_DeSelect();
}
/*
	Send data (16bit) to SPI1
	return: received data
*/
uint16_t SPI_SendData(uint16_t data) {
	while(SPI1->SR & SPI_SR_BSY);
	SPI1->DR = data;
	while(!(SPI1->SR & SPI_SR_RXNE));
	return SPI1->DR;
}
/*
	Generate CONVST pulse for AD7091
*/
void AD7091_Convst() {
	AD7091_ConvstClr();
	AD7091_ConvstSet();
}
/*
	Reading registers from both AD7091 (daisy chain connect)
	regAddr - register address
	return: values of registers (bit 16..31 - #1 ADC, bit 0..15 - #2 ADC)
*/
uint64_t AD7091_ReadReg(uint8_t regAddr) {
	
	uint16_t cmd = (uint16_t)regAddr << 11;
	AD7091_Select();
	SPI_SendData(cmd);
	SPI_SendData(cmd);
	AD7091_DeSelect();
	AD7091_Select();
//	uint64_t resp = (uint64_t)SPI_SendData(0) << 32;
	uint64_t resp = SPI_SendData(0) << 16;
	resp |= SPI_SendData(0);
	AD7091_DeSelect();
	
	return resp;
}


/*
	Write to both AD7091 registers  (daisy chain connect) 0000.1100.0011.1111
*/
void AD7091_WriteReg(uint8_t regAddr, uint16_t value) {
	uint16_t cmd = ((uint16_t)regAddr << 11) | (1L << 10) | (value & 0x3f);
	AD7091_Select();
	SPI_SendData(cmd);
	SPI_SendData(cmd);
//	SPI_SendData(cmd);
	AD7091_DeSelect();
}
/*
	Start conversion and read result from both AD7091 (daisy chain connect)
	return: result
*/
void AD7091_ReadConv(void)
{

	
}
void AD7091_ReadValues() {
//	uint64_t res;
//	 {
//		res = AD7091_ReadConv();
//		regValues[i] = (res & 0x0fff);
//		regValues[i + 6] = ((res >> 16) & 0x0fff);
////		regValues[i + 12] = ((res >> 32) & 0x0fff);
////		regValues[i + 18] = ((res >> 48) & 0x0fff);
//	}
}
uint16_t GetValue(uint8_t n) {
	return regValues[n];
}
/*
	Test for AD7091
	read 1000 times of Configuration register (#2), 
	which should contain a value = 0x00C0
	return: count of errors
*/
uint32_t AD7091_Test(void) {
	uint32_t errCount = 0;
	for (int i=0; i < 1000; i++) {
		if (AD7091_ReadReg(REG_CONFIG_ADDR) != 0x00c000c0) errCount++;
	}
	return errCount;
}
