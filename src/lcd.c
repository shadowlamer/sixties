#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_spi.h"
#include "delay.h"
//#include "stm32f0xx_dma.h"
#include "stm32f0xx_rcc.h"
#include "lcd.h"

#define GPIO_Pin_LCDReset GPIO_Pin_0
#define GPIO_Pin_LCDSelect GPIO_Pin_1

#ifdef LCD_EPSON
#define EP_DISON       0xAF
#define EP_DISOFF      0xAE
#define EP_DISNOR      0xA6
#define EP_DISINV      0xA7
#define EP_SLPIN       0x95
#define EP_SLPOUT      0x94
#define EP_COMSCN      0xBB
#define EP_DISCTL      0xCA
#define EP_PASET       0x75
#define EP_CASET       0x15
#define EP_DATCTL      0xBC
#define EP_RGBSET8     0xCE
#define EP_RAMWR       0x5C
#define EP_RAMRD       0x5D
#define EP_PTLIN       0xA8
#define EP_PTLOUT      0xA9
#define EP_RMWIN       0xE0
#define EP_RMWOUT      0xEE
#define EP_ASCSET      0xAA
#define EP_SCSTART     0xAB
#define EP_OSCON       0xD1
#define EP_OSCOFF      0xD2
#define EP_PWRCTR      0x20
#define EP_VOLCTR      0x81
#define EP_VOLUP       0xD6
#define EP_VOLDOWN     0xD7
#define EP_TMPGRD      0x82
#define EP_EPCTIN      0xCD
#define EP_EPCOUT      0xCC
#define EP_EPMWR       0xFC
#define EP_EPMRD       0xFD
#define EP_EPSRRD1     0x7C
#define EP_EPSRRD2     0x7D
#define EP_NOP         0x25
#endif

#ifdef LCD_PHILIPS
#define LCD_CMD_SLEEPOUT 0x11
#define LCD_CMD_BSTRON   0x03
#define LCD_CMD_SLEEPOUT 0x11
#define LCD_CMD_COLMOD   0x3A
#define LCD_CMD_SETCON   0x25
#define LCD_CMD_DISPON   0x29
#define LCD_CMD_PASET    0x2B
#define LCD_CMD_CASET    0x2A
#define LCD_CMD_RAMWR    0x2C
#define LCD_CMD_MADCTL   0x36
#endif


#define LCD_PIXEL_FORMAT_8BIT 0x02
#define LCD_PIXEL_FORMAT_16BIT 0x05

uint8_t lcd_buf[LCD_BUF_LENGTH];

uint16_t lcd_xpointer = 0;
uint16_t lcd_ypointer = 0;
uint32_t next = 1;

static volatile uint8_t busy;

/* K&R rand function : return pseudo-random integer on 0..32767 */
int rand(void)
{
next = next * 1103515245 + 12345;
return (unsigned int)(next/65536);
}

/* srand: set seed for rand() */
void srand(unsigned int seed)
{
next = seed;
}

void lcd_process_memory()
{
	uint16_t buf_pointer;
	uint8_t shift;
	static uint8_t buf;

	if (!(lcd_ypointer%2)) //interlace lines
		buf=0x00;
	else {
		if (!(lcd_xpointer%4)) {
			buf_pointer = ((lcd_ypointer/2) * LCD_BUF_ROW_SIZE) + (lcd_xpointer/32);
			shift = (lcd_xpointer/4)%8;
			if ((lcd_buf[buf_pointer]<<shift)&0x80)
				buf=0xff;
			else {
				buf=0x00;
			}
		}
	}
	lcd_xpointer++;
	if (lcd_xpointer==LCD_WIDTH*2) { //line feed
		lcd_xpointer=0;
		lcd_ypointer++;
	}
	if (lcd_ypointer<LCD_HEIGHT)
		lcd_data(buf);
	else {
		lcd_data(buf);
		NVIC_DisableIRQ(SPI1_IRQn);
		lcd_xpointer = 0;
		lcd_ypointer = 0;
		busy = 0;
	}

}

uint8_t lcd_is_busy()
{
	return busy;
}

void lcd_reset(FunctionalState cmd)
{
	switch (cmd)
	{
#ifdef LCD_EPSON
	case ENABLE:
		GPIO_ResetBits(GPIOA, GPIO_Pin_LCDReset);
		break;
	case DISABLE:
		GPIO_SetBits(GPIOA, GPIO_Pin_LCDReset);
		break;
	}
#endif
#ifdef LCD_PHILIPS
	case ENABLE:
		GPIO_ResetBits(GPIOA, GPIO_Pin_LCDReset);
		break;
	case DISABLE:
		GPIO_SetBits(GPIOA, GPIO_Pin_LCDReset);
		break;
	}
#endif
}

void lcd_select(FunctionalState cmd)
{
	switch (cmd)
	{
	case ENABLE:
		GPIO_ResetBits(GPIOA, GPIO_Pin_LCDSelect);
		break;
	case DISABLE:
		GPIO_SetBits(GPIOA, GPIO_Pin_LCDSelect);
		break;
	}
}

void lcd_command(uint8_t byte)
{
	SPI_I2S_SendData16(SPI1, byte & 0x0ff);
}

void lcd_data(uint8_t byte)
{
	SPI_I2S_SendData16(SPI1, byte | 0x100);
}

void lcd_dma_start()
{
#ifdef LCD_EPSON
	lcd_command(EP_RAMWR);
#endif
#ifdef LCD_PHILIPS
	lcd_command(LCD_CMD_RAMWR);
#endif
	NVIC_EnableIRQ(SPI1_IRQn);
	busy=0;
	lcd_process_memory();
}

void lcd_on()
{
	uint16_t i;
	lcd_reset(ENABLE);
	delay_ms(100);
	lcd_reset(DISABLE);
	delay_ms(1);
#ifdef LCD_EPSON
    lcd_command(EP_DISCTL);  	// display control(EPSON)
    lcd_data(0x0C);   		// 12 = 1100 - CL dividing ratio [don't divide] switching period 8H (default)
	lcd_data(0x20);			// 130 line display
	lcd_data(0x00);			// no inversely highlighted lines
	lcd_data(0x01);			// dispersion

    lcd_command(EP_COMSCN);  	// common scanning direction(EPSON)
    lcd_data(0x01);

    lcd_command(EP_OSCON);  	// internal oscillator ON(EPSON)

    lcd_command(EP_SLPOUT);  	// sleep out(EPSON)

    lcd_command(EP_PWRCTR); 	// power ctrl(EPSON)
    lcd_data(0x0F);    		//everything on, no external reference resistors

	lcd_command(EP_DISINV);  	// invert display mode(EPSON)

    lcd_command(EP_DATCTL);  	// data control(EPSON)
    lcd_data((LCD_ROTATE << 2)|(LCD_INVERTY << 1)|(LCD_INVERTX << 0));	// normal column addressing
	lcd_data(0x00);   		// normal RGB arrangement
	lcd_data(0x04);			// 16-bit Grayscale Type A

    lcd_command(EP_VOLCTR);  	// electronic volume, this is the contrast/brightness(EPSON)
	lcd_data(0x25);   		// volume (contrast) setting - fine tuning, original
    lcd_data(0x03);   		// internal resistor ratio - coarse adjustment

    lcd_command(EP_NOP);  	// nop(EPSON)

	delay_ms(1);

    lcd_command(EP_DISON);   	// display on(EPSON)
	delay_ms(1);

    lcd_command(EP_PASET);
	lcd_data(1);
	lcd_data(132);

	lcd_command(EP_CASET);
	lcd_data(1);
	lcd_data(132);

	lcd_command(EP_RAMWR);
	for (i=0;i<130*130*2;i++)
		lcd_data(0x00);

    lcd_command(EP_PASET);
	lcd_data(2);
	lcd_data(129);

	lcd_command(EP_CASET);
	lcd_data(2);
	lcd_data(129);

#endif
#ifdef LCD_PHILIPS
	lcd_command(LCD_CMD_SLEEPOUT);
	lcd_command(LCD_CMD_BSTRON);
	delay_ms(1);
	lcd_command(LCD_CMD_COLMOD);
	lcd_data(LCD_PIXEL_FORMAT_16BIT);
	delay_ms(1);
	lcd_command(LCD_CMD_SETCON);
	lcd_data(LCD_CONTRAST);
	delay_ms(1);
	lcd_command(LCD_CMD_MADCTL);
	lcd_data((LCD_ROTATE << 5)|(LCD_INVERTY << 7)|(LCD_INVERTX << 6));
	delay_ms(1);
	lcd_command(LCD_CMD_DISPON);
	delay_ms(1);
	lcd_command(LCD_CMD_PASET);
	lcd_data(1);
	lcd_data(130);
	delay_ms(1);
	lcd_command(LCD_CMD_CASET);
	lcd_data(1);
	lcd_data(130);
	delay_ms(1);
	lcd_command(LCD_CMD_RAMWR);
	for (i=0;i<130*130*2;i++)
		lcd_data(0x00);
	delay_ms(1);
	lcd_command(LCD_CMD_PASET);
	lcd_data(2);
	lcd_data(129);
	delay_ms(1);
	lcd_command(LCD_CMD_CASET);
	lcd_data(2);
	lcd_data(129);
	delay_ms(1);
#endif

}

void lcd_prepare()
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); //for delay timer
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_0);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_0);



	SPI_InitTypeDef SPI_InitStructure;
	SPI_StructInit(&SPI_InitStructure);
	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_9b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);
	SPI_I2S_ITConfig(SPI1,SPI_I2S_IT_TXE,ENABLE);
	SPI_Cmd(SPI1, ENABLE);

	lcd_select(ENABLE);
}

void lcd_clear() {
	uint16_t i;
	for (i=0;i<LCD_BUF_LENGTH;i++) //clear video bufer
		lcd_buf[i]=0x00;
}

void lcd_noise() {
	uint16_t i;
	uint16_t* buf = (uint32_t *)lcd_buf;
	for (i=0;i<LCD_BUF_LENGTH/2;i++) //clear video bufer
		buf[i]=rand();
}

void set_pixel(uint8_t x, uint8_t y) {
	uint16_t buf_pointer = (y*8)+x/8;
	uint8_t shift = x%8;
	lcd_buf[buf_pointer]|=0x80>>shift;
}

void lcd_init() {
	lcd_clear();
	lcd_prepare();
	lcd_on();
//	lcd_dma_start();
}



void SPI1_IRQHandler (void) {
	static volatile uint8_t i = 0;

	if (SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==SET) {
		lcd_process_memory();
	}
}


