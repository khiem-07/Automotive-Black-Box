#include "main.h"

#include <string.h>

#include "fonts.h"

extern SPI_HandleTypeDef hspi1;



#define LCD_CMD_DELAY_MS 0xFF
#define LCD_CMD_EOF 0xFF
void ST7735_WriteCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);  // CS LOW
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);  // DC LOW
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 1000);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);  // CS HIGH
}

void ST7735_WriteData(uint8_t data)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);  // CS LOW
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);  // DC HIGH
    HAL_SPI_Transmit(&hspi1, &data, 1, 1000);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);  // CS HIGH
}

void ST7735_SetWindow(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    ST7735_WriteCommand(0x2A); 
    ST7735_WriteData(0x00);
    ST7735_WriteData(x1);
    ST7735_WriteData(0x00);
    ST7735_WriteData(x2);
    
    ST7735_WriteCommand(0x2B);   
    ST7735_WriteData(0x00);
    ST7735_WriteData(y1);
    ST7735_WriteData(0x00);
    ST7735_WriteData(y2);
}




static const uint8_t u8InitCmdList[] = {
//  Command     Length      Data
    0xB1,       0x03,       0x01, 0x2C, 0x2D,                       // Frame Rate Control (In normal mode/ Full colors)
    0xB2,       0x03,       0x01, 0x2C, 0x2D,                       // Frame Rate Control (In Idle mode/ 8-colors)
    0xB3,       0x06,       0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,     // Frame Rate Control (In Partial mode/ full colors)
    0xB4,       0x01,       0x07,                                   // Display Inversion Control
    0xC0,       0x03,       0xA2, 0x02, 0x84,                       // Power Control 1
    0xC1,       0x01,       0xC5,                                   // Power Control 2
    0xC2,       0x02,       0x0A, 0x00,                             // Power Control 3 (in Normal mode/ Full colors)
    0xC3,       0x02,       0x8A, 0x2A,                             // Power Control 4 (in Idle mode/ 8-colors)
    0xC4,       0x02,       0x8A, 0xEE,                             // Power Control 5 (in Partial mode/ full-colors)
    0xC5,       0x01,       0x0E,                                   // VCOM Control 1
    0xE0,       0x10,       0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10,     //Gamma adjustment(+ polarity)
    0xE1,       0x10,       0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10,     //Gamma adjustment(- polarity)
    LCD_CMD_EOF, LCD_CMD_EOF
};

static void ST7735_SendCommandList(const uint8_t* cmdList)
{
    uint8_t dat = 0;
    uint8_t cmd = 0;
    uint8_t num = 0;

    while (1)
    {
        cmd = *cmdList++;
        num = *cmdList++;

        if (cmd == LCD_CMD_EOF)  {
            break;
        }
        else {
            ST7735_WriteCommand(cmd);
            for (dat = 0; dat < num; ++dat)
                ST7735_WriteData(*cmdList++);
        }
    }
}

void ST7735_FillScreen(uint16_t color)
{
    ST7735_WriteCommand(0x2C);
    for (int i = 0; i < 128 * 160; i++) {
        ST7735_WriteData(color >> 8);
        ST7735_WriteData(color & 0xFF);
    }
}

void ST7735_DrawPixel(uint8_t x, uint8_t y, uint16_t color) {
    if (x >= 128 || y >= 160) return; 
    ST7735_SetWindow(x, y, x + 1, y + 1); 
    ST7735_WriteCommand(0x2C); 
    ST7735_WriteData(color >> 8);
    ST7735_WriteData(color & 0xFF);
}

void ST7735_DrawChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bg) {
    uint16_t i, j;
    uint16_t pixelData;

    for (i = 0; i < font.height; i++) {
        pixelData = font.data[(ch - 32) * font.height + i]; 

        for (j = 0; j < font.width; j++) {
            if ((pixelData << j) & 0x8000) 
                ST7735_DrawPixel(x + j, y + i, color);
            else
                ST7735_DrawPixel(x + j, y + i, bg);
        }
    }
}

void ST7735_DrawString(uint16_t x, uint16_t y, char* str, FontDef font, uint16_t color, uint16_t bg) {
    while (*str) {
        ST7735_DrawChar(x, y, *str, font, color, bg);
        x += font.width;
        str++;
    }
}


void ST7735_Init()
{
	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET); // RESET = 0
    HAL_Delay(20);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);   // RESET = 1
    HAL_Delay(150);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);  // CS = 0
  
    ST7735_WriteCommand(0x01); // Software Reset
    HAL_Delay(150);
    ST7735_WriteCommand(0x11); // Sleep Out
    HAL_Delay(255);

    ST7735_WriteCommand(0x36); // Memory Access Control
    ST7735_WriteData(0x08);  // Row/Column exchange, RGB color filter

    ST7735_WriteCommand(0x3A); //Interface Pixel Format
    ST7735_WriteData(0x05);


    ST7735_SetWindow(0, 0, 128, 160);

    ST7735_SendCommandList(u8InitCmdList);

    ST7735_WriteCommand(0x29);
    HAL_Delay(100);
}