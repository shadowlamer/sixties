 #ifndef __LCD_H
#define __LCD_H

//#define LCD_EPSON
#define LCD_PHILIPS
#define LCD_CONTRAST 0x40
#define LCD_WIDTH  128
#define LCD_HEIGHT 128
#define LCD_INVERTX 1
#define LCD_INVERTY 0
#define LCD_ROTATE 1
#define LCD_INIT_COLOR 0x01

#include "stm32f0xx.h"

#define LCD_BUF_ROW_SIZE (LCD_WIDTH/8/2)
#define LCD_BUF_LENGTH LCD_BUF_ROW_SIZE * LCD_HEIGHT/2
extern uint8_t lcd_buf[LCD_BUF_LENGTH];

void lcd_init();
void lcd_clear();
void lcd_noise();

void set_pixel(uint8_t x, uint8_t y);
void lcd_dma_start();
uint8_t lcd_is_busy();


#endif
