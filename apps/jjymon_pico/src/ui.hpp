#pragma once

#include "shapoco/ssd1306/pico/spi_lcd.hpp"
#include "shapoco/ssd1306/pico/i2c_lcd.hpp"

namespace shapoco::jjymon {

static constexpr int LCD_W = 128;
static constexpr int LCD_H = 64;

using JjySpiLcd = shapoco::ssd1306::pico::SpiLcd<
    LCD_W, LCD_H, 
    LCD_SPI_INDEX, 
    PIN_SPI_LCD_RES_N, 
    PIN_SPI_LCD_CS_N, 
    PIN_SPI_LCD_DC, 
    PIN_SPI_LCD_SCLK, 
    PIN_SPI_LCD_MOSI, 
    LCD_SPI_FREQ
>;

using JjyI2cLcd = shapoco::ssd1306::pico::I2cLcd<
    LCD_W, LCD_H, 
    LCD_I2C_INDEX, 
    PIN_I2C_LCD_SDA, 
    PIN_I2C_LCD_SCL, 
    LCD_I2C_ADDR,
    LCD_I2C_FREQ
>;

void ui_init(void);
void ui_loop(void);

}
