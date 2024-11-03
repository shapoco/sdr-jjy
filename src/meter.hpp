#ifndef METER_HPP
#define METER_HPP

#include "jjymon.hpp"
#include "fixed12.hpp"
#include "ssd1309spi.hpp"
#include "images/images.hpp"

using pen_t = ssd1309spi::pen_t;

class Meter {
public:
    static constexpr int WIDTH = 32;
    static constexpr int HEIGHT = 12;

private:
    static constexpr int WAVEFORM_PERIOD = 1 << 7;

    int32_t waveform_max[WAVEFORM_PERIOD];
    int32_t waveform_min[WAVEFORM_PERIOD];

    int32_t disp_phase_offset = 0;
    int cursor_phase = 0;

public:
    void render(uint32_t t_now_ms, int x0, int y0, JjyLcd &lcd, int32_t val) {
        constexpr int32_t A_PERIOD = fxp12::ANGLE_PERIOD * 40 / 360;
        constexpr int RADIUS_MAX = 40;
        constexpr int RADIUS_MIN = RADIUS_MAX - 8;

        lcd.draw_bitmap(x0, y0, bmp_meter_frame);
        
        val = FXP_CLIP(0, fxp12::ONE, val);
        int32_t a = (fxp12::ANGLE_PERIOD * 3 / 4 - A_PERIOD / 2) + (A_PERIOD * val) / fxp12::ONE;
        int32_t sin = fxp12::sin(a);
        int32_t cos = fxp12::cos(a);
        int32_t cx = (x0 + 16) * fxp12::ONE;
        int32_t cy = (y0 + RADIUS_MAX) * fxp12::ONE + fxp12::ONE / 2;
        lcd.draw_line_f(cx + cos * (RADIUS_MAX - 1), cy + sin * (RADIUS_MAX - 1), cx + cos * RADIUS_MIN, cy + sin * RADIUS_MIN);
    }
};

#endif
