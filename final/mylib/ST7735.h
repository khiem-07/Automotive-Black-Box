#ifndef __ST7735_H
#define __ST7735_H

#include "stm32f1xx_hal.h"

// Các l?nh di?u khi?n màn hình ST7735
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_MADCTL  0x36
#define ST7735_DISPON  0x29
#define ST7735_DISPOFF 0x28

// Các màu s?c
#define ST7735_BLACK   0x0000
#define ST7735_BLUE    0x001F
#define ST7735_RED     0xF800
#define ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF

// Khai báo các hàm
void ST7735_Init(SPI_HandleTypeDef *hspi);
void ST7735_WriteCommand(uint8_t cmd);
void ST7735_WriteData(uint8_t data);
void ST7735_SetCursor(uint8_t x, uint8_t y);
void ST7735_FillScreen(uint16_t color);
void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7735_WriteString(char* str, uint16_t color, uint16_t bgColor, uint8_t size);
void ST7735_SetTextColor(uint16_t color);
void ST7735_SetTextSize(uint8_t size);

#endif