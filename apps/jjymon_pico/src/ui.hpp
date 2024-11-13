#pragma once

#include "shapoco/ssd1306/pico/spi_lcd.hpp"

namespace shapoco::jjymon {

static constexpr int LCD_W = 128;
static constexpr int LCD_H = 64;

using JjySpiLcd = shapoco::ssd1306::pico::SpiLcd<LCD_W, LCD_H, LCD_SPI_INDEX, LCD_SPI_FREQ, PIN_LCD_RES_N, PIN_LCD_CS_N, PIN_LCD_DC, PIN_LCD_SCLK, PIN_LCD_MOSI>;

void ui_init(void);
void ui_loop(void);

}
