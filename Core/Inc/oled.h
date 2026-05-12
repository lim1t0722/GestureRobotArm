/**
  * @file    oled.h
  * @brief   SSD1306 0.96" OLED driver (128x64, I2C via software bit-bang on PB10/PB11)
  */
#ifndef __OLED_H
#define __OLED_H

#include "main.h"

#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define OLED_PAGES      8
#define OLED_I2C_ADDR   0x78

#define OLED_SCL_PIN    GPIO_PIN_10
#define OLED_SDA_PIN    GPIO_PIN_11
#define OLED_GPIO_PORT  GPIOB

#define OLED_FONT_W_6x8     6
#define OLED_FONT_W_8x16    8

typedef enum {
    OLED_FONT_6x8 = 0,
    OLED_FONT_8x16 = 1
} OLED_FontSize_t;

uint8_t OLED_Init(void);
void OLED_DeInit(void);
void OLED_Clear(void);
void OLED_Refresh(void);
void OLED_RefreshArea(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void OLED_SetPixel(uint8_t x, uint8_t y, uint8_t val);
void OLED_DrawChar(uint8_t x, uint8_t y, char ch, OLED_FontSize_t size);
void OLED_DrawString(uint8_t x, uint8_t y, const char *str, OLED_FontSize_t size);
void OLED_DrawInt(uint8_t x, uint8_t y, int32_t val, OLED_FontSize_t size);
void OLED_DrawHex(uint8_t x, uint8_t y, uint16_t val, OLED_FontSize_t size);
void OLED_DrawFloat(uint8_t x, uint8_t y, float val, uint8_t decimals, OLED_FontSize_t size);
void OLED_FillArea(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t val);
void OLED_DrawHLine(uint8_t x, uint8_t y, uint8_t len);
void OLED_DrawVLine(uint8_t x, uint8_t y, uint8_t len);
void OLED_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void OLED_SetContrast(uint8_t contrast);
void OLED_SetDisplayOn(uint8_t on);
void OLED_InvertDisplay(uint8_t invert);
uint8_t OLED_IsReady(void);
extern uint8_t oled_fb[];
void OLED_WriteCmd(uint8_t cmd);
void OLED_WriteDataBurst(const uint8_t *data, uint16_t len);

#endif /* __OLED_H */
