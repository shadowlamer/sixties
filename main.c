#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>

#include "inc/generator.h"
#include "inc/delay.h"
#include "inc/lcd.h"
#include "inc/CP866.h"
#include "inc/pattern.h"

#include "inc/message.h"

void putchar(uint8_t x, uint8_t y, uint8_t c) {
	uint8_t i;
	uint16_t scr_offset = x + y*64;
	uint16_t chr_offset = c*8;
	for (i=0;i<8;i++)
		lcd_buf[i*8+scr_offset]=font[i+chr_offset];

}

void putchar_big(uint8_t x, uint8_t y, uint8_t c) {
	uint8_t i;
	uint8_t j;
	uint16_t scr_offset = x + y*64;
	uint16_t chr_offset = c*8;
	uint16_t tmpbuf;
	uint8_t cline;
	for (i=0;i<8;i++) {
		cline = font[i+chr_offset];
		for (j=0;j<8;j++) {
			tmpbuf |= ((cline&0x80)>>7);
			tmpbuf<<=1;
			tmpbuf |= ((cline&0x80)>>7);
			tmpbuf<<=1;
			cline<<=1;
		}
		tmpbuf>>=1;
		lcd_buf[i*16+scr_offset]=((uint8_t*)&tmpbuf)[1];
		lcd_buf[i*16+1+scr_offset]=((uint8_t*)&tmpbuf)[0];
		lcd_buf[i*16+8+scr_offset]=((uint8_t*)&tmpbuf)[1];
		lcd_buf[i*16+8+1+scr_offset]=((uint8_t*)&tmpbuf)[0];
	}

}

void print(uint8_t x, uint8_t y, char* s, uint8_t len) {
	uint8_t i;
	for(i=0;i<len;i++)
		putchar(x++,y,s[i]);
}

void scroll_text(char* s){
	uint16_t i;
	for (i=0;s[i]!=0;i++) {
		scroll_screen(s[i]);
	}
}

void scroll_screen(char c){
	uint8_t offset;
	for (offset=0;offset<8;offset++) {
		scroll_line(c,offset);
		if (!(offset%2)) {
			while (lcd_is_busy());
			lcd_dma_start();
		}
//		delay_ms(150);
	}
}

void scroll_line(uint8_t c, uint8_t n){
	uint16_t x;
	uint16_t y;
	uint16_t offset;
	uint16_t tmpbuf;
	uint8_t cline;

	for (y=0;y<16;y++) {
		for(x=0;x<7;x++) {
			offset = (y+24)*8+x;
			lcd_buf[offset]<<=1;
			lcd_buf[offset]|=((lcd_buf[offset+1]&0x80)>>7);
		}
		cline = font[c*8+y/2];
		cline<<=n;
		offset = (y+24)*8+x;
		lcd_buf[offset]<<=1;
		lcd_buf[offset]|=((cline&0x80)>>7);
	}
}

int main(void)
{

	uint32_t x = 0;
	uint32_t y = 0;

	RCC_DeInit();
	RCC_HSEConfig(RCC_HSE_ON);
	if(RCC_WaitForHSEStartUp() == SUCCESS)
	{
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		RCC_PCLKConfig(RCC_HCLK_Div1);
		RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_8);
		RCC_PLLCmd(ENABLE);
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while(RCC_GetSYSCLKSource() != 0x08);
	}
	generator_init();
	generator_start();
	delay_init();
	lcd_init();

	lcd_clear();
	lcd_dma_start();


	for (x=0;x<50;x++) {
		while (lcd_is_busy());
		lcd_noise();
		lcd_dma_start();
	}


	while (lcd_is_busy());

	for (x=0;x<patternWidthPages*patternHeightPixels;x++)
		lcd_buf[x] = patternBitmaps[x];

	print(1,0,"PLEASE",6);
	print(0,7,"STAND_BY",8);

	while (lcd_is_busy());
	lcd_dma_start();

	delay_ms(20000);


	for (x=0;x<patternWidthPages*patternHeightPixels;x++)
		lcd_buf[x] = patternBitmaps[x];

	for (x=9;x>0;x--) {
		putchar_big(3,3,'0'+x);
		while (lcd_is_busy());
		lcd_dma_start();
		delay_ms(3000);
	}

	lcd_clear();
	print(1,1,"URGENT",6);
	print(0,6,"MESSAGE!",8);
	while (lcd_is_busy());
	lcd_dma_start();
	delay_ms(3000);

	scroll_text(MESSAGE);

	delay_ms(3000);
	lcd_clear();
	print(1,1,"THAT'S",6);
	print(3,3,"ALL",3);
	print(1,5,"FOLKS!",6);
	while (lcd_is_busy());
	lcd_dma_start();

	delay_ms(100000);

	for (x=0;x<10000;x++) {
		while (lcd_is_busy());
		lcd_noise();
		lcd_dma_start();
	}

	lcd_clear();
	while (lcd_is_busy());
	lcd_dma_start();

	scroll_text(SECRET_MESSAGE);

	while(1) //Infinite loop!
	{
		while (lcd_is_busy());
		lcd_noise();
		lcd_dma_start();
	}
}
