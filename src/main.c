#include "stm32f0xx.h"
#include "stdio.h"
#include "setup.h"
#include "ad7091.h"
#include "usart.h"
#include "ad7091.h" 
volatile uint32_t tickCount, flags;
uint32_t adRegs[16];
char str[128];
volatile uint16_t regValues[24];
#define FL_SEC			(0x0001)

void SysTick_Handler(void) {
	if (++tickCount >= 1000) {
		tickCount = 0;
		flags |= FL_SEC;
	}
}

int main(void) {
	static volatile uint32_t AD_Status;
	static uint32_t regValues[24], cnt, max_cnt;
	SysClockConfig();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
	AD7091_Config();
	USART_Config(2000000);
	sprintf(str,"ChannelReg: %012X\n", (uint32_t)AD7091_ReadReg(REG_CHANNEL_ADDR));
	USART_Puts(str);
	for (uint8_t i = 0; i < 1; ++i)
	{
		AD7091_ConvstClr();
		AD7091_ConvstSet();
		AD7091_Select();
		SPI_SendData(0);
		SPI_SendData(0);
		AD7091_DeSelect();
	}
	
	while(1) {		
		static uint16_t tmp_0, tmp_1, shift;
		for (uint8_t i = 0; i < 6; ++i)
		{
			AD7091_ConvstClr();
			AD7091_ConvstSet();
			AD7091_Select();
			regValues[i + 6] 	= SPI_SendData(0) & 0x0fff;
			regValues[i] 			= SPI_SendData(0) & 0x0fff;
			AD7091_DeSelect();
		}
		for (int i = 0; i < 12; i++) 
		{
			sprintf(str, "%04d ", regValues[i]);
			USART_Puts(str);
		}
		sprintf(str, "cnt: %04d ", max_cnt);
		USART_Puts(str);
		USART_Putchar('\n');
		cnt++;
		if (flags & FL_SEC)
		{
			max_cnt = cnt;
			flags &= ~FL_SEC;
			cnt = 0;
		}
		

//			flags &= ~FL_SEC;			
//			for (int i = 0; i < 12; i++) 
//			{
//				uint16_t tmp = (regValues[i]/(cnt));
//				sprintf(str, "%04d ", regValues[i]);
//				regValues[i] = 0;
//				USART_Puts(str);
//				if(!((i+1)%6)) USART_Putchar('\n');
//			}
//			sprintf(str, "cnt: %04d ", cnt);
//			USART_Puts(str);
//			cnt = 0;
//			USART_Putchar('\n');
//		}
	}
	
}


void CRC_init(void)
{
	RCC->AHBENR |= RCC_AHBENR_CRCEN;
}

uint32_t crc_calc(uint32_t* frame)
{
	CRC->CR = CRC_CR_RESET | CRC_CR_REV_IN_0 | CRC_CR_REV_OUT; //CRC-32/JAMCRC, need xor for compabile with casual crc32
		while	(frame)
		{
			CRC->DR = *frame;
			frame++;
		}
	return CRC->DR;
}

