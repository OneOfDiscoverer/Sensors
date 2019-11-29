#pragma once

void AD7091_Config(void);
uint64_t AD7091_ReadReg(uint8_t regAddr);
void AD7091_WriteReg(uint8_t regAddr, uint16_t value);
void AD7091_ReadValues(void);
void AD7091_ReadConv(void);
uint16_t GetValue(uint8_t n);
uint32_t AD7091_Test(void);
uint16_t SPI_SendData(uint16_t data);
void SpiDelay(uint32_t n);

#define REG_RESULT_ADDR			(0x00)
#define REG_CHANNEL_ADDR		(0x01)
#define REG_CONFIG_ADDR			(0x02)
#define REG_ALERT_ADDR			(0x03)
#define REG_CH0LOW_ADDR			(0x04)
#define REG_CH0HIGH_ADDR		(0x05)
#define REG_CH0HYST_ADDR		(0x06)

#define AD7091_Select()			(GPIOA->BRR = (1 << 4))
#define AD7091_DeSelect()		(GPIOA->BSRR = (1 << 4))
#define AD7091_ConvstSet()		(GPIOA->BSRR = (1 << 1))
#define AD7091_ConvstClr()		(GPIOA->BRR = (1 << 1))
