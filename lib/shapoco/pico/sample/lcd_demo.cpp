#if 0

#include <stdint.h>

#include "shapoco/fixed12.hpp"
#include "shapoco/graphics/graphics.hpp"

#include "jjymon.hpp"
#include "ui/ui.hpp"
#include "fonts.hpp"

#include "lcd_demo.hpp"

using namespace shapoco::graphics;

void lcd_demo_render(JjySpiLcd &lcd, uint32_t t_now_ms) {
    const int32_t xline = 43;
    const int32_t xelip = 80;

    const int32_t yfxp = 8;
    const int32_t yint = 36;

    lcd.draw_string(bmpfont::font5, xline + 4, 0, "LINE");
    lcd.draw_string(bmpfont::font5, xelip, 0, "ELLIPSE");

    lcd.draw_string(bmpfont::font5, 10, yfxp + 8, "FIXED");
    lcd.draw_string(bmpfont::font5, 10, yfxp + 14, "POINT");
    lcd.draw_string(bmpfont::font5, 15, yint + 11, "INT");

    {
        uint32_t t = t_now_ms * 4;
        int32_t a0 = (t * 11 / 100) % fxp12::ANGLE_PERIOD;
        int32_t a1 = (t * 12 / 100) % fxp12::ANGLE_PERIOD;
        int32_t a2 = (t * 13 / 100) % fxp12::ANGLE_PERIOD;
        int32_t a3 = (t * 9 / 100) % fxp12::ANGLE_PERIOD;
        int32_t a4 = (t * 15 / 100) % fxp12::ANGLE_PERIOD;
        int32_t scale = fxp12::ONE + fxp12::sin(a2) / 4;
        int32_t lr = 10 * scale;
        int32_t dx = fxp12::cos(a0) * lr / fxp12::ONE;
        int32_t dy = fxp12::sin(a0) * lr / fxp12::ONE;
        int32_t cx = (xline + 12) * fxp12::ONE + fxp12::cos(a1) * 2;
        int32_t cy = (yfxp + 14) * fxp12::ONE + fxp12::sin(a1) * 2;
        int32_t ew = 15 * (fxp12::ONE + fxp12::sin(a3) / 4) * scale / fxp12::ONE;
        int32_t eh = 15 * (fxp12::ONE + fxp12::sin(a4) / 4) * scale / fxp12::ONE;
        int32_t ecx = cx + (xelip - xline) * fxp12::ONE;
        int32_t ecy = cy;
        lcd.draw_line_f(cx + dx, cy + dy, cx + dy, cy - dx);
        lcd.draw_line_f(cx + dy, cy - dx, cx - dx, cy - dy);
        lcd.draw_line_f(cx - dx, cy - dy, cx - dy, cy + dx);
        lcd.draw_line_f(cx - dy, cy + dx, cx + dx, cy + dy);
        lcd.fill_ellipse_f(ecx - ew / 2, ecy - eh / 2, ew, eh);
        cy += (yint - yfxp) * fxp12::ONE;
        ecy += (yint - yfxp) * fxp12::ONE;
        lcd.draw_line(fxp12::to_int(cx + dx), fxp12::to_int(cy + dy), fxp12::to_int(cx + dy), fxp12::to_int(cy - dx));
        lcd.draw_line(fxp12::to_int(cx + dy), fxp12::to_int(cy - dx), fxp12::to_int(cx - dx), fxp12::to_int(cy - dy));
        lcd.draw_line(fxp12::to_int(cx - dx), fxp12::to_int(cy - dy), fxp12::to_int(cx - dy), fxp12::to_int(cy + dx));
        lcd.draw_line(fxp12::to_int(cx - dy), fxp12::to_int(cy + dx), fxp12::to_int(cx + dx), fxp12::to_int(cy + dy));
        int32_t el = fxp12::to_int(ecx - ew / 2) * fxp12::ONE;
        int32_t et = fxp12::to_int(ecy - eh / 2) * fxp12::ONE;
        int32_t er = fxp12::to_int(ecx + ew / 2) * fxp12::ONE;
        int32_t eb = fxp12::to_int(ecy + eh / 2) * fxp12::ONE;
        lcd.fill_ellipse_f(el, et, er - el, eb - et);
    }
}

#endif