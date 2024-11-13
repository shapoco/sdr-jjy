#pragma once

#include "shapoco/pico/ssd1309spi.hpp"

namespace shapoco::jjymon {

static constexpr int LCD_W = 128;
static constexpr int LCD_H = 64;

using JjyLcd = shapoco::pico::Ssd1309Spi<LCD_W, LCD_H, LCD_SPI_INDEX, LCD_SPI_FREQ, PIN_LCD_RES_N, PIN_LCD_CS_N, PIN_LCD_DC, PIN_LCD_SCLK, PIN_LCD_MOSI>;

void ui_init(void);
void ui_loop(void);

}
