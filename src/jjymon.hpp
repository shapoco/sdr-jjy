#ifndef JJYMON_HPP
#define JJYMON_HPP

#include "jjy/jjy.hpp"
#include "ssd1309spi.hpp"

static constexpr int LCD_W = 128;
static constexpr int LCD_H = 64;

static constexpr int LCD_SPI_INDEX = 0;
static constexpr uint32_t LCD_SPI_FREQ = 2e6;
static constexpr int PIN_LCD_CS_N = 17;
static constexpr int PIN_LCD_SCLK = 18;
static constexpr int PIN_LCD_MOSI = 19;
static constexpr int PIN_LCD_RES_N = 20;
static constexpr int PIN_LCD_DC = 21;

typedef struct {
    jjy::rx::rf_status_t rf;
    jjy::rx::sync_status_t sync;
} receiver_status_t;

using JjyLcd = ssd1309spi::Lcd<LCD_W, LCD_H, LCD_SPI_INDEX, LCD_SPI_FREQ, PIN_LCD_RES_N, PIN_LCD_CS_N, PIN_LCD_DC, PIN_LCD_SCLK, PIN_LCD_MOSI>;

#endif
