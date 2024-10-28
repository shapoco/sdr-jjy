#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "core0.hpp"
#include "core1.hpp"

#include "ssd1309spi.hpp"
using pen_t = ssd1309spi::pen_t;

extern volatile uint8_t glb_jjy_curr_signal;
extern volatile uint8_t glb_jjy_qty_percent;

static constexpr int LCD_W = 128;
static constexpr int LCD_H = 64;

static constexpr int LCD_SPI_INDEX = 0;
static constexpr uint32_t LCD_SPI_FREQ = 1e6;
static constexpr int PIN_LCD_CS_N = 17;
static constexpr int PIN_LCD_SCLK = 18;
static constexpr int PIN_LCD_MOSI = 19;
static constexpr int PIN_LCD_RES_N = 20;
static constexpr int PIN_LCD_DC = 21;

ssd1309spi::Lcd<LCD_W, LCD_H, LCD_SPI_INDEX, LCD_SPI_FREQ, PIN_LCD_RES_N, PIN_LCD_CS_N, PIN_LCD_DC, PIN_LCD_SCLK, PIN_LCD_MOSI> lcd;

void core1_init() {
    lcd.init();
}

void core1_main() {
    uint32_t t_next_flip_ms = to_ms_since_boot(get_absolute_time());
    float x = LCD_W / 2;
    float y = LCD_H / 2;
    float dx = 1.0 / 10;
    float dy = 0.99999 / 10;
    while (true) {
        uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());
        if (t_now_ms >= t_next_flip_ms) {
            t_next_flip_ms = t_now_ms + 10;
            //lcd.set_pixel(x, y);
            //lcd.clear();
            lcd.fill_rect(x - 15, y - 15, 30, 30, (t_now_ms >> 9) & 1 ? pen_t::WHITE : pen_t::BLACK);
            x += dx;
            y += dy;
            if ((x < 0 && dx < 0) || (x >= LCD_W && dx > 0)) dx = -dx;
            if ((y < 0 && dy < 0) || (y >= LCD_H && dy > 0)) dy = -dy;
            lcd.commit();
        }
        lcd.service();
    }
}
