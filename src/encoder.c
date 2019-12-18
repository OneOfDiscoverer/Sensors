#include "stm32f10x.h"
#include "encoder.h"
#include "gpio_lib.h"

#define PI	(314L)

static int32_t cur_speed, cur_rps, cur_rpm, old_speed = 0, cur_accel;

//		Configure PC6, PC7 as encoder input
//
void Encoder_config(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	
	GPIO_PinConfig(GPIOC, 6, GPIO_MODE_IF);
	GPIO_PinConfig(GPIOC, 7, GPIO_MODE_IF);
	
	AFIO->MAPR &= ~AFIO_MAPR_TIM3_REMAP;
	AFIO->MAPR |= AFIO_MAPR_TIM3_REMAP_FULLREMAP;				// TIM3 Full remap: CH1-PC6, CH2-PC7
	
	TIM3->CR1 = 0;
	TIM3->CR2 = 0;
	TIM3->CCMR1 = TIM_CCMR1_CC1S_0|TIM_CCMR1_CC2S_0;						// CC1 input, IC1 mapped on TI1; CC2 input, IC2 mapped on TI2
	TIM3->SMCR = TIM_SMCR_SMS_0|TIM_SMCR_SMS_1;								// External trigger filter with N = 4, Encoder mode 3
//	TIM3->SMCR |= TIM_SMCR_ETF_1;
	TIM3->CCER = TIM_CCER_CC1E|TIM_CCER_CC2E;								// Capture CC1, CC2 enable
	TIM3->ARR = 0xffff;
	TIM3->CR1 = TIM_CR1_CEN;
}
//		Расчет скорости и ускорения
//		enc_value - скорость в единицах энкодера (положение)
//		ppr_value - кол-во импульсов энкодера на оборот
//		freq - частота измерений, Гц
//
void Encoder_check(int32_t enc_value, uint16_t ppr_value, uint16_t freq)
{
	cur_speed = (int64_t)(enc_value * 2 * PI * freq) / ppr_value;
	cur_rps = (int32_t)(enc_value * freq ) / ppr_value;			// скорость в RPS
	cur_rpm = cur_rps * 60;

	cur_accel = (cur_speed - old_speed);
	old_speed = cur_speed;
	
}
int16_t GetCurrentRPM(void)
{
	return cur_rpm;
}
int16_t GetCurrentRPS(void)
{
	return cur_rps;
}
int16_t GetCurrentSpeed(void)
{
	return cur_speed;
}
int16_t GetCurrentAccel(void)
{
	return cur_accel;
}
void SetEncPPR(uint32_t ppr)
{
}
