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
    static constexpr int WIDTH = 128;
    static constexpr int HEIGHT = 18;

    bool decStsToggle = false;
    bool clockInitialized = false;
    jjy::JjyDateTime dateTime;
    jjy::JjyClock clock;

    const TinyFont &font;
    RotaryCounter yearH;
    RotaryCounter yearL;
    RotaryCounter month;
    RotaryCounter day;
    RotaryCounter hour;
    RotaryCounter minute;
    RotaryCounter second;
    RotaryCounter dayOfWeek;

    ProgressBar progBar;

    ClockView() :
        font(fonts::font12),
        yearH(font, 0, 99),
        yearL(font, 0, 99),
        month(font, 1, 12),
        day(font, 1, 31),
        hour(font, 0, 23),
        minute(font, 0, 59),
        second(font, 0, 59),
        dayOfWeek(fonts::font5, 0, 6),
        progBar(fonts::font5, 48)
    { 
        dayOfWeek.setAliases(DAY_OF_WEEK_STRING);

        yearH.zeroPadding = true;
        yearL.zeroPadding = true;
        month.zeroPadding = true;
        day.zeroPadding = true;
        hour.zeroPadding = true;
        minute.zeroPadding = true;
        second.zeroPadding = true;

        yearH.forwardOnly = true;
        yearL.forwardOnly = true;
        month.forwardOnly = true;
        day.forwardOnly = true;
        hour.forwardOnly = true;
        minute.forwardOnly = true;
        second.forwardOnly = false;
        dayOfWeek.forwardOnly = true;

        int32_t followRatio = fxp12::ONE / 4;
        yearH.followRatio = followRatio;
        yearL.followRatio = followRatio;
        month.followRatio = followRatio;
        day.followRatio = followRatio;
        hour.followRatio = followRatio;
        minute.followRatio = followRatio;
        second.followRatio = followRatio;
        dayOfWeek.followRatio = followRatio;

        int32_t followStepMax = fxp12::ONE / 8;
        yearH.followStepMax = followStepMax;
        yearL.followStepMax = followStepMax;
        month.followStepMax = followStepMax;
        day.followStepMax = followStepMax;
        hour.followStepMax = followStepMax;
        minute.followStepMax = followStepMax;
        second.followStepMax = followStepMax;
        dayOfWeek.followStepMax = followStepMax;

        yearH.setBlank();
        yearL.setBlank();
        month.setBlank();
        day.setBlank();
        hour.setBlank();
        minute.setBlank();
        second.setBlank();
        dayOfWeek.setBlank();
    }

    void update(uint64_t nowMs, const jjymon::receiver_status_t &sts) {
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
            yearH.setNumber(nowMs, dateTime.year / 100);
            yearL.setNumber(nowMs, dateTime.year % 100);
            month.setNumber(nowMs, dateTime.month);
            day.setNumber(nowMs, dateTime.day);
            hour.setNumber(nowMs, dateTime.hour);
            minute.setNumber(nowMs, dateTime.minute);
            second.setNumber(nowMs, dateTime.second);
            dayOfWeek.setNumber(nowMs, (int)dateTime.dayOfWeek);
            progBar.setVisible(nowMs, false);
        }
        else if (!sts.sync.phaseLocked || !sts.dec.synced) {
            progBar.setMessage("STBY...");
            progBar.setValue(nowMs, 0);
            progBar.setVisible(nowMs, true);
        }
        else {
            progBar.setMessage("RECV...");
            progBar.setValue(nowMs, sts.dec.lastBitIndex * fxp12::ONE / 60);
            progBar.setVisible(nowMs, true);
        }
        
        yearH.update(nowMs);
        yearL.update(nowMs);
        month.update(nowMs);
        day.update(nowMs);
        hour.update(nowMs);
        minute.update(nowMs);
        second.update(nowMs);
        dayOfWeek.update(nowMs);
        progBar.update(nowMs);
    }

    void render(uint64_t nowMs, ssd1306::Screen &g, int x0, int y0) {
        {
            int x = x0, y = y0;
            if (clockInitialized) {
                dayOfWeek.render(g, x, y);
            }
            else {
                g.drawString(fonts::font5, x, y, "---");
            }
        }

        {
            int x = x0, y = y0 + HEIGHT - fonts::font12.height;
            x += renderCounter(g, yearH, x, y);
            x += renderCounter(g, yearL, x, y);
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

        progBar.render(nowMs, g, x0 + (WIDTH - progBar.width) / 2, y0 + (HEIGHT - progBar.height) / 2);
    }

    int renderCounter(ssd1306::Screen &g, RotaryCounter &cntr, int x0, int y0) {
        cntr.render(g, x0, y0);
        return cntr.width + fonts::font12.spacing;
    }
};

}
