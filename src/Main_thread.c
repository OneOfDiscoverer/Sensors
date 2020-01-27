#include "Main_thread.h"
#include "stm32f0xx.h"
#include "usart.h"
#include "string.h"
#include "Flash_msp.h"
#include "MEM_map.h"

extern volatile uint8_t buf[BUF_LEN];
extern volatile uint16_t len;
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
			if(buf[0] == 0x55) 
			{
				FT_set_erase((uint16_t*)BOOT_LOADER_DATA_PAGE);
				FLASH->CR |= FLASH_CR_PG; 
				*(__IO uint16_t*)(BOOT_LOADER_DATA_PAGE) = KEY_UPDATE; 
				while(!(FLASH->SR & FLASH_SR_EOP));
				FLASH->SR |= FLASH_SR_EOP;
				FLASH->CR &= ~FLASH_CR_PG;
				NVIC_SystemReset();
			}
			else if(buf[0] == 0xDE) NVIC_SystemReset();
			else
			{
				sprintf(str,"Com err\n");
				USART_Puts(str);
			}
			buf_erase();
			MT_mode = MT_wait;
			break;
	}
}


//			case MT_update_f:
//				FT_set_erase((uint16_t*)BOOT_LOADER_DATA_PAGE);
//				MT_mode = MT_update_s;
//				break;
//			case MT_update_s:
//				FT_set_write(	(uint16_t*)BOOT_LOADER_DATA_PAGE+1, (uint16_t*)BOOT_LOADER_DATA_PAGE+7, (uint16_t*)&ext_start_addr_l);
//				ext_command_reg &= ~(_COM_UPDATE);
//				MT_mode = MT_wait;
//				break;
//			case MT_data_ready_f:
//				ext_end_addr_h = ext_start_addr_h;
//				ext_end_addr_l = ext_start_addr_l+_PAGE8_LENGHT;
//				if(crc_calc((uint32_t*)&ext_data_page, ((uint32_t*)&ext_data_page+_BUF32_LENGHT)))
//				{
//					ext_command_reg &= ~(_FLAG_CRC_MB_BUF);
//					FT_set_erase((uint16_t*)(ext_start_addr_h << 16 | ext_start_addr_l));
//					MT_mode = MT_data_ready_s;
//				}
//				else
//				{
//					ext_command_reg |= _FLAG_CRC_MB_BUF;
//					ext_command_reg &= ~(_COM_DATA_READY);
//					MT_mode = MT_wait;
//				}
//				break;
//			case MT_data_ready_s:
//				FT_set_write(	(uint16_t*)(ext_start_addr_h << 16 | ext_start_addr_l),
//											(uint16_t*)(ext_end_addr_h << 16 | ext_end_addr_l),
//											(uint16_t*)&ext_data_page);
//				MT_mode = MT_calc_all;
//				break;
//			case MT_calc_all:
//				if(crc_calc((uint32_t*)(ext_start_addr_h << 16 | ext_start_addr_l), 
//					(uint32_t*)(ext_end_addr_h << 16 | ext_end_addr_l)))
//					ext_command_reg &= ~(_FLAG_CRC_FLASH);
//				else 	
//					ext_command_reg |= _FLAG_CRC_FLASH;
//				ext_command_reg &= ~(_COM_DATA_READY | _COM_CALC_ALL);
//				MT_mode = MT_wait;
