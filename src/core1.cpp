#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "jjymon.hpp"

#include "core0.hpp"
#include "core1.hpp"

#include "atomic.hpp"
#include "jjy/jjy.hpp"
#include "fixed12.hpp"

#include "ssd1309spi.hpp"
#include "images/images.hpp"
#include "bmpfont/bmpfont.hpp"

#include "rader.hpp"

extern atomic<receiver_status_t> glb_receiver_status;
static receiver_status_t sts;

static JjyLcd lcd;
using pen_t = ssd1309spi::pen_t;

static Rader rader;

static void paint_sync_status(uint32_t t_now_ms);

void core1_init() {
    lcd.init();
}

void core1_main() {
    uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());
    uint32_t t_next_paint_ms = t_now_ms;
    uint32_t t_next_sync_ms = t_now_ms;

    sts = glb_receiver_status.load();

    while (true) {
        uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());

        if (t_now_ms >= t_next_sync_ms) {
            t_next_sync_ms = t_now_ms + 5;
            sts = glb_receiver_status.load();
            rader.update(t_now_ms, sts);
        }

        if (t_now_ms >= t_next_paint_ms) {
            t_next_paint_ms = t_now_ms + 20;

            lcd.clear();
            rader.render(t_now_ms, lcd, sts);
            paint_sync_status(t_now_ms);
            lcd.commit();
        }

        lcd.service();
    }
}

static void paint_sync_status(uint32_t t_now_ms) {
    int x = 0;
    int y = 32;
    const bmpfont::Font &font = bmpfont::font16;
    if (!sts.sync.phase_locked) {
        lcd.draw_string(font, x, y, "Phase Sync...");
        y += font.height - 2;
        lcd.fill_rect(x, y, (LCD_W / 2) * sts.sync.phase_lock_progress / jjy::ONE, 1);
    }
    else if (!sts.sync.bit_det_ok) {
        lcd.draw_string(font, x, y, "Pulse Check...");
        y += font.height - 2;
        lcd.fill_rect(x, y, (LCD_W / 2) + (LCD_W / 2) * sts.sync.bit_det_progress / jjy::ONE, 1);
    }
}