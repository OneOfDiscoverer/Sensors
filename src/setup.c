#include "stm32f0xx.h"
#include "setup.h"

//		Конфигурирование SysClock
//		старт HSE, настройка PLL
//
void SysClockConfig(void)
{
	uint32_t HSEStartupTimeout = 1000000L;
	
	RCC->CR |= RCC_CR_HSEON;
	while(HSEStartupTimeout-- && !(RCC->CR & RCC_CR_HSERDY));
	
	if (RCC->CR & RCC_CR_HSERDY)
	{
		RCC->CFGR &= ~RCC_CFGR_HPRE;										// AHB not prescaled
		RCC->CFGR &= ~RCC_CFGR_PPRE;										// PCLK = APB
		RCC->CFGR &= ~(RCC_CFGR_PLLMUL | RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE);
		RCC->CFGR |= RCC_CFGR_PLLMUL6 | RCC_CFGR_PLLSRC_HSE_PREDIV;			// PLL mul = 6 (8x6 = 48MHz)
		RCC->CR |= RCC_CR_PLLON;
		
		while(!(RCC->CR & RCC_CR_PLLRDY));
		
		RCC->CFGR &= ~(RCC_CFGR_SW);
		RCC->CFGR |= RCC_CFGR_SW_PLL;
		
		while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
	}
}
