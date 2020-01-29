#include "Main_thread.h"
#include "stm32f0xx.h"
#include "usart.h"
#include "string.h"
#include "Flash_msp.h"
#include "MEM_map.h"
#include "setup.h"

extern volatile uint16_t buf[BUF_LEN];
extern volatile uint16_t len;
extern volatile uint32_t flags;
uint16_t num_page;
uint8_t MT_mode;
extern char str[128];
void up_mt(void)
{
	if(MT_mode == MT_wait) MT_mode = MT_update_f;
}
 
void Main_thread(void)
{
	switch(MT_mode)
	{
		case MT_wait:
			break;
		case MT_update_f:
			flags |= FL_NS;
			switch(buf[0])
			{
				case 0x55:
					FT_set_erase((uint16_t*)BOOT_LOADER_DATA_PAGE);
					FLASH->CR |= FLASH_CR_PG; 
					*(__IO uint16_t*)(BOOT_LOADER_DATA_PAGE) = KEY_UPDATE; 
					while(!(FLASH->SR & FLASH_SR_EOP));
					FLASH->SR |= FLASH_SR_EOP;
					FLASH->CR &= ~FLASH_CR_PG;
					NVIC_SystemReset();
					break;
				case 0xDE:
					NVIC_SystemReset();
					break;
				default:
					sprintf(str,"Com err\n");
					USART_Puts(str);
					break;
			}
			buf_erase();
			MT_mode = MT_wait;
			break;
	}
}

