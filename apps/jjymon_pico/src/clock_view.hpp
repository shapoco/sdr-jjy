#pragma once

#include <stdio.h>
#include <string.h>

#include "shapoco/jjy/jjy.hpp"
#include "shapoco/fixed12.hpp"
#include "shapoco/graphics/graphics.hpp"
#include "shapoco/ssd1306/pico/spi_lcd.hpp"

#include "jjymon.hpp"
#include "ui.hpp"
#include "images.hpp"
#include "fonts.hpp"
#include "rotary_counter.hpp"

namespace shapoco::jjymon {

using namespace shapoco::graphics;
using pen_t = shapoco::ssd1306::pen_t;

class ClockView {
public:
    const TinyFont &font;
    RotaryCounter year_h;
    RotaryCounter year_l;
    RotaryCounter month;
    RotaryCounter day;
    RotaryCounter hour;
    RotaryCounter minute;
    RotaryCounter second;

    ClockView() :
        font(fonts::font12),
        year_h(font, 0, 99),
        year_l(font, 0, 99),
        month(font, 1, 12),
        day(font, 1, 31),
        hour(font, 0, 23),
        minute(font, 0, 59),
        second(font, 0, 59)
    { 
        year_h.zeroPadding = true;
        year_l.zeroPadding = true;
        month.zeroPadding = true;
        day.zeroPadding = true;
        hour.zeroPadding = true;
        minute.zeroPadding = true;
        second.zeroPadding = true;

        year_h.forwardOnly = true;
        year_l.forwardOnly = true;
        month.forwardOnly = true;
        day.forwardOnly = true;
        hour.forwardOnly = true;
        minute.forwardOnly = true;
        second.forwardOnly = false;

        int32_t followRatio = fxp12::ONE / 8;
        year_h.followRatio = followRatio;
        year_l.followRatio = followRatio;
        month.followRatio = followRatio;
        day.followRatio = followRatio;
        hour.followRatio = followRatio;
        minute.followRatio = followRatio;
        second.followRatio = followRatio;

        int32_t followStepMax = fxp12::ONE / 8;
        year_h.followStepMax = followStepMax;
        year_l.followStepMax = followStepMax;
        month.followStepMax = followStepMax;
        day.followStepMax = followStepMax;
        hour.followStepMax = followStepMax;
        minute.followStepMax = followStepMax;
        second.followStepMax = followStepMax;
    }

    void setDateTime(uint64_t t_ms, jjy::JjyDateTime dt) {
        year_h.setNumber(dt.year / 100);
        year_l.setNumber(dt.year % 100);
        month.setNumber(dt.month);
        day.setNumber(dt.day);
        hour.setNumber(dt.hour);
        minute.setNumber(dt.minute);
        second.setNumber(dt.second);
    }

    void update(uint64_t t_ms) {
        year_h.update(t_ms);
        year_l.update(t_ms);
        month.update(t_ms);
        day.update(t_ms);
        hour.update(t_ms);
        minute.update(t_ms);
        second.update(t_ms);
    }

    void render(ssd1306::Screen &g, int x0, int y0) {
        int x = x0;
        x += renderCounter(g, year_h, x, y0);
        x += renderCounter(g, year_l, x, y0);
        x += g.drawChar(font, x, y0, '/') + font.spacing;
        x += renderCounter(g, month, x, y0);
        x += g.drawChar(font, x, y0, '/') + font.spacing;
        x += renderCounter(g, day, x, y0);
        x += g.drawChar(font, x, y0, ' ') + font.spacing;
        x += renderCounter(g, hour, x, y0);
        x += g.drawChar(font, x, y0, ':') + font.spacing;
        x += renderCounter(g, minute, x, y0);
        x += g.drawChar(font, x, y0, ':') + font.spacing;
        x += renderCounter(g, second, x, y0);
    }

    int renderCounter(ssd1306::Screen &g, RotaryCounter &cntr, int x0, int y0) {
        cntr.render(g, x0, y0);
        return cntr.width + fonts::font12.spacing;
    }
};

}
