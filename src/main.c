#include "stm32f0xx.h"
#include "stdio.h"
#include "setup.h"
#include "ad7091.h"
#include "usart.h"
#include "ad7091.h" 
#include "gpio_init.h"
#include "MEM_map.h"
#include "Main_thread.h"
#include "Flash_msp.h"

volatile uint32_t tickCount, flags;
uint16_t encode;
uint32_t adRegs[16];
char str[128];
volatile uint16_t regValues[24];


void Encoder_config(void);
void Start_VTinit(void);
void Watch_dog_init(void);
void Watch_dog_reload(void);

void Watch_dog_init(void)
{
	IWDG -> KR = 0x0000CCCC;
	IWDG -> KR = 0x00005555;
	IWDG -> PR = 7;
	IWDG -> RLR = 0xFFFFFFFF;
	while(!(IWDG->SR == 0x00000000));
}

void Watch_dog_reload(void)
{
	IWDG -> KR = 0x0000AAAA;	
}
void SysTick_Handler(void) {
	if (++tickCount >= 1000) {
		tickCount = 0;
		flags |= FL_SEC;
		flags &= ~FL_NS;
	} 
}

void Start_VTinit(void)
{
	__disable_irq();
  
  volatile uint32_t *RAMVectorTable = (volatile uint32_t *)RAM_START_ADDRESS;
  for(uint32_t iVector = 0; iVector < 48; iVector++) 
      RAMVectorTable[iVector] = *(__IO uint32_t *)((uint32_t)MAIN_PROGRAM_START_ADDRESS + (iVector << 2));
	
  SYSCFG->CFGR1 |= SYSCFG_CFGR1_MEM_MODE;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
	
	__enable_irq();
}

int main(void) {
	static volatile uint32_t AD_Status;
	static uint32_t regValues[24], cnt, max_cnt;
	Start_VTinit();
	SysClockConfig();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
	USART_Config(2000000);
	sprintf(str,"Main app\n");
	USART_Puts(str);
	
	Encoder_config();
	while(1) 
	{		
		
		Watch_dog_reload();
		do_modbus();
		Main_thread();
		Flash_thread();
		
		AD7091_Config();
		uint64_t tmp_req = AD7091_ReadReg(REG_CHANNEL_ADDR);
		if(tmp_req == (uint64_t)0x003f003f003f003f) flags |= FL_ADC_OK;
		else flags &= ~FL_ADC_OK;
		if(flags & FL_ADC_OK)
		{
			for (uint8_t i = 0; i < 1; ++i)
			{
				AD7091_ConvstClr();
				AD7091_ConvstSet();
				AD7091_Select();
				SPI_SendData(0);
				SPI_SendData(0);
				SPI_SendData(0);
				SPI_SendData(0);
				AD7091_DeSelect();
			}
			for (uint8_t i = 0; i < 6; ++i)
			{
				//encode &= 0xFFFF0000;
				encode = TIM1->CNT;
				AD7091_ConvstClr();
				AD7091_ConvstSet();
				AD7091_Select();
				regValues[i + 18] = SPI_SendData(0);
				regValues[i + 12] = SPI_SendData(0);
				regValues[i + 6] 	= SPI_SendData(0);
				regValues[i] 			= SPI_SendData(0);
				AD7091_DeSelect();
			}
			for (int i = 0; i < 24; i++) 
			{
//				if((regValues[i] == 0x0000) || (regValues[i] == 0xFFFF)) sprintf(str, "[%02d nop]", i);
//				else 
				sprintf(str, "%04d;", regValues[i] & 0x0FFF);
				USART_Puts(str);
			}
			sprintf(str, "%u", encode);
			USART_Puts(str);
			USART_Putchar('\n');
		//		cnt++;
		}
		if (flags & FL_SEC)
		{
			if(!(flags & FL_ADC_OK))
			{
				sprintf(str,"ADC not found\n");
				USART_Puts(str);
			}
			flags &= ~FL_SEC;
			cnt = 0;
		}
	}	
}

//void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
//{
//	TIM1->SR &= ~TIM_SR_UIF;
//	if(!TIM1->CNT) encode += 0x10000;
//	else encode -= 0x10000;
//}

void Encoder_config(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	gpio_init(GPIOA, 8, MODE_af, OTYPE_pp, OSPEED_vfast, PUPD_no, AF_2);
	gpio_init(GPIOA, 9, MODE_af, OTYPE_pp, OSPEED_vfast, PUPD_no, AF_2);
	
	TIM1->CR1 = 0;
	TIM1->CR2 = 0;
	//TIM1->DIER |= TIM_DIER_UIE;
	TIM1->CCMR1 = TIM_CCMR1_CC1S_0|TIM_CCMR1_CC2S_0;						// CC1 input, IC1 mapped on TI1; CC2 input, IC2 mapped on TI2
	TIM1->SMCR = TIM_SMCR_SMS_0|TIM_SMCR_SMS_1;								// External trigger filter with N = 4, Encoder mode 3
//	TIM3->SMCR |= TIM_SMCR_ETF_1;
	TIM1->CCER = TIM_CCER_CC1E|TIM_CCER_CC2E;								// Capture CC1, CC2 enable
	TIM1->ARR = 0xffff;
	TIM1->CR1 = TIM_CR1_CEN;
	//NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
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

