#pragma once

#define BUF_LEN		0x100
#define MB_STATE_START_WAIT 0
#define MB_STATE_WAIT 1


void USART_Putchar(uint8_t d);
void USART_Config(uint32_t baudRate);
void USART_Puts(char *);
void USART_PutDat(uint32_t s);
void USART2_IRQHandler(void);
void do_modbus(void);
void buf_erase(void);

