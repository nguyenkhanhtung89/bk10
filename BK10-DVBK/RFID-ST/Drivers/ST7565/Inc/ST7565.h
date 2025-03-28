#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include "stm32f30x.h"
#include "display.h"
#include <string.h>

#define swap(a, b) { uint8_t t = a; a = b; b = t; }

#define BLACK 1
#define WHITE 0

#define LCDWIDTH 128
#define LCDHEIGHT 64

#define CMD_DISPLAY_OFF   0xAE
#define CMD_DISPLAY_ON    0xAF

#define CMD_SET_DISP_START_LINE  0x40
#define CMD_SET_PAGE  0xB0

#define CMD_SET_COLUMN_UPPER  0x10
#define CMD_SET_COLUMN_LOWER  0x00

#define CMD_SET_ADC_NORMAL  0xA0
#define CMD_SET_ADC_REVERSE 0xA1

#define CMD_SET_DISP_NORMAL 0xA6
#define CMD_SET_DISP_REVERSE 0xA7

#define CMD_SET_ALLPTS_NORMAL 0xA4
#define CMD_SET_ALLPTS_ON  0xA5
#define CMD_SET_BIAS_9 0xA2 
#define CMD_SET_BIAS_7 0xA3

#define CMD_RMW  0xE0
#define CMD_RMW_CLEAR 0xEE
#define CMD_INTERNAL_RESET  0xE2
#define CMD_SET_COM_NORMAL  0xC0
#define CMD_SET_COM_REVERSE  0xC8
#define CMD_SET_POWER_CONTROL  0x28
#define CMD_SET_RESISTOR_RATIO  0x20
#define CMD_SET_VOLUME_FIRST  0x81
#define CMD_SET_VOLUME_SECOND  0
#define CMD_SET_STATIC_OFF  0xAC
#define CMD_SET_STATIC_ON  0xAD
#define CMD_SET_STATIC_REG  0x0
#define CMD_SET_BOOSTER_FIRST  0xF8
#define CMD_SET_BOOSTER_234  0
#define CMD_SET_BOOSTER_5  1
#define CMD_SET_BOOSTER_6  3
#define CMD_NOP  0xE3
#define CMD_TEST  0xF0

#define LCD_SPI_SCK_PIN         		GPIO_Pin_13
#define LCD_SPI_SCK_GPIO_PORT   		GPIOB
#define LCD_SPI_SCK_GPIO_CLK    		RCC_AHBPeriph_GPIOB

#define LCD_SPI_MOSI_PIN         		GPIO_Pin_15
#define LCD_SPI_MOSI_GPIO_PORT   		GPIOB
#define LCD_SPI_MOSI_GPIO_CLK    		RCC_AHBPeriph_GPIOB

#define LCD_SPI_SELECT_PIN         	GPIO_Pin_12
#define LCD_SPI_SELECT_GPIO_PORT   	GPIOB
#define LCD_SPI_SELECT_GPIO_CLK    	RCC_AHBPeriph_GPIOB

#define LCD_RESET_PIN         			GPIO_Pin_2
#define LCD_RESET_GPIO_PORT   			GPIOB
#define LCD_RESET_GPIO_CLK    			RCC_AHBPeriph_GPIOB

#define LCD_A0_PIN         					GPIO_Pin_14
#define LCD_A0_GPIO_PORT   					GPIOB
#define LCD_A0_GPIO_CLK    					RCC_AHBPeriph_GPIOB

#define LCD_LED_PIN									GPIO_Pin_1
#define LCD_LED_GPIO_PORT 		  		GPIOB
#define LCD_LED_GPIO_CLK						RCC_AHBPeriph_GPIOB

typedef enum{
	STR_ERROR = 0,
	STR_LV0 = 1,
	STR_LV1 = 2,
	STR_LV2 = 3,
	STR_LV3 = 4,
	STR_LV4 = 5
}enum_STR_LEVEL;

typedef enum{
	DISP_GPS = 0,
	DISP_GSM = 1,
	DISP_3G = 2,
	DISP_LTE = 3,
	DISP_NONE = 4
}enum_STR_DISP_type;

void ST7565_SPI_Init(void);
void ST7565_init(void);
void LCD_Init(uint8_t contrast);
void ST7565_command(uint8_t c);
void ST7565_data(uint8_t c);
void ST7565_set_brightness(uint8_t val);
void ST7565_clear_display(void);
void ST7565_clear(void);
void ST7565_display(void);
void ST7565_setpixel(uint8_t x, uint8_t y, uint8_t color);
uint8_t ST7565_getpixel(uint8_t x, uint8_t y);
void ST7565_fillcircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color);
void ST7565_drawcircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color);
void ST7565_drawrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void ST7565_fillrect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color);
void ST7565_drawline(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color);

void ST7565_drawbitmap(uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color);
void ST7565_my_setpixel(uint8_t x, uint8_t y, uint8_t color);
