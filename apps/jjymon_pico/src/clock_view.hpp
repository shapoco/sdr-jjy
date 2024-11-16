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
    static constexpr int HEIGHT = 18;
    static constexpr char* DAY_OF_WEEK_STR[] = {
        "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
    };

    bool decStsToggle = false;
    bool clockInitialized = false;
    jjy::JjyDateTime dateTime;
    jjy::JjyClock clock;

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

        int32_t followRatio = fxp12::ONE / 4;
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
    }

    void update(uint64_t nowMs, const jjymon::receiver_status_t &sts) {
        year_h.update(nowMs);
        year_l.update(nowMs);
        month.update(nowMs);
        day.update(nowMs);
        hour.update(nowMs);
        minute.update(nowMs);
        second.update(nowMs);

        if (sts.dec.toggle != decStsToggle) {
            decStsToggle = sts.dec.toggle;
            jjy::ParseResut parseResult = sts.dec.last_parse_result;
            if (parseResult.success()) {
                clock.set(sts.dec.lastRecvTimestampMs, sts.dec.lastRecvDateTimeEffective);
                clockInitialized = true;
            }
        }

        if (clockInitialized) {
            dateTime = clock.get(nowMs);
            year_h.setNumber(dateTime.year / 100);
            year_l.setNumber(dateTime.year % 100);
            month.setNumber(dateTime.month);
            day.setNumber(dateTime.day);
            hour.setNumber(dateTime.hour);
            minute.setNumber(dateTime.minute);
            second.setNumber(dateTime.second);
        }
    }

    void render(ssd1306::Screen &g, int x0, int y0) {
        {
            int x = x0, y = y0;
            if (clockInitialized) {
                g.drawString(fonts::font5, x, y, DAY_OF_WEEK_STR[(int)dateTime.day_of_week]);
            }
            else {
                g.drawString(fonts::font5, x, y, "---");
            }
        }

        {
            int x = x0, y = y0 + HEIGHT - fonts::font12.height;
            if (clockInitialized) {
                x += renderCounter(g, year_h, x, y);
                x += renderCounter(g, year_l, x, y);
                x += g.drawChar(font, x, y, '/') + font.spacing;
                x += renderCounter(g, month, x, y);
                x += g.drawChar(font, x, y, '/') + font.spacing;
                x += renderCounter(g, day, x, y);
                x += g.drawChar(font, x, y, ' ') + font.spacing;
                x += renderCounter(g, hour, x, y);
                x += g.drawChar(font, x, y, ':') + font.spacing;
                x += renderCounter(g, minute, x, y);
                x += g.drawChar(font, x, y, ':') + font.spacing;
                x += renderCounter(g, second, x, y);
            }
            else {
                g.drawString(fonts::font12, x, y, "----/--/-- --:--:--");
            }
        }
    }

    int renderCounter(ssd1306::Screen &g, RotaryCounter &cntr, int x0, int y0) {
        cntr.render(g, x0, y0);
        return cntr.width + fonts::font12.spacing;
    }
};

}
