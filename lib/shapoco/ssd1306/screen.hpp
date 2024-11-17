#pragma once

#include <stdint.h>
#include <string.h>

#include "shapoco/common.hpp"
#include "shapoco/math_utils.hpp"
#include "shapoco/fixed12.hpp"
#include "shapoco/graphics/common.hpp"
#include "shapoco/graphics/tiny_font.hpp"
#include "shapoco/ssd1306/common.hpp"

namespace shapoco::ssd1306 {

using namespace shapoco::graphics;

class Screen {
public:
    const int width, height;
    const int numPages = (height + PAGE_HEIGHT - 1) / PAGE_HEIGHT;
    const int sizeBytes = width * numPages;

    uint8_t * const data;
    Rect clipRect;

    Screen(int w, int h) :
        width(w),
        height(h),
        numPages(SHPC_CEIL_DIV(h, PAGE_HEIGHT)),
        sizeBytes(w * numPages),
        data(new uint8_t[sizeBytes]),
        clipRect(0, 0, w, h) { }

    ~Screen() {
        delete[] data;
    }

    void clear(seg_t c = 0x00) {
        memset(data, c, sizeBytes * sizeof(seg_t));
    }

    int get_seg_index(int x, int y) const {
        return (y / PAGE_HEIGHT) * width + x;
    }

    seg_t *pagePtr(int page) const {
        return &data[page * width];
    }

    Rect getBounds() const { return Rect(0, 0, width, height); }

    void setClipRect(Rect bounds) {
        bounds.intersectSelf(getBounds());
        clipRect = bounds;
    }

    void setClipRect(int x, int y, int w, int h) {
        setClipRect(Rect(x, y, w, h));
    }

    void clearClipRect() {
        setClipRect(getBounds());
    }

    void set_pixel(int x, int y, pen_t c = pen_t::WHITE) {
        if (!clipRect.contains(x, y)) return;
        uint8_t mask = 1 << (y % PAGE_HEIGHT);
        int iseg = get_seg_index(x, y);
        switch (c) {
        case pen_t::BLACK: data[iseg] &= ~mask; break;
        case pen_t::WHITE: data[iseg] |= mask; break;
        }
    }

    void fillRect(int x, int y, int w, int h, pen_t pen = pen_t::WHITE) {
        fillRect(Rect(x, y, w, h), pen);
    }

    void fillRect(Rect rect, pen_t c = pen_t::WHITE) {
        rect.intersectSelf(clipRect);
        if (rect.w <= 0 || rect.h <= 0) return;
        int x = rect.x;
        int y = rect.y;
        int w = rect.w;
        //int h = rect.h;
        //int r = rect.r();
        int b = rect.b();

        int first_page = y / PAGE_HEIGHT;
        int final_page = (b - 1) / PAGE_HEIGHT;
        seg_t first_seg = ~(seg_t)((1 << (y % PAGE_HEIGHT)) - 1);
        seg_t final_seg = (1 << (((b + PAGE_HEIGHT - 1) % PAGE_HEIGHT) + 1)) - 1;
        
        if (first_page == final_page) {
            first_seg &= final_seg;
        }
        
        for (int p = first_page; p <= final_page; p++) {
            seg_t mask =
                (p == first_page) ? first_seg :
                (p == final_page) ? final_seg :
                ~(seg_t)0;
            seg_t *wr_ptr = &data[p * width + x];

            if (((seg_t)~mask) == 0) {
                memset(wr_ptr, c == pen_t::BLACK ? 0x00 : 0xff, w * sizeof(seg_t));
            }
            else if (c == pen_t::BLACK) {
                mask = ~mask;
                for (int i = 0; i < w; i++) {
                    *(wr_ptr++) &= mask;
                }
            }
            else {
                for (int i = 0; i < w; i++) {
                    *(wr_ptr++) |= mask;
                }
            }
        }
    }

    void drawRect(int x, int y, int w, int h, pen_t pen = pen_t::WHITE) {
        fillRect(x, y, w + 1, 1);
        fillRect(x, y + 1, 1, h - 1);
        fillRect(x + w, y + 1, 1, h - 1);
        fillRect(x, y + h, w + 1, 1);
    }

    void draw_line(int x0, int y0, int x1, int y1, pen_t pen = pen_t::WHITE) {
        x0 *= fxp12::ONE;
        y0 *= fxp12::ONE;
        x1 *= fxp12::ONE;
        y1 *= fxp12::ONE;
        drawLineF(x0, y0, x1, y1, pen);
    }

    void drawLineF(int32_t x0f, int32_t y0f, int32_t x1f, int32_t y1f, pen_t pen = pen_t::WHITE) {
        int32_t dxf = x1f - x0f;
        int32_t dyf = y1f - y0f;
        if (FXP_ABS(dxf) > FXP_ABS(dyf)) {
            int xi = fxp12::floorToInt(x0f);
            int n = FXP_ABS(fxp12::floorToInt(x1f) - xi);
            int xi_step = dxf >= 0 ? 1 : -1;
            for (int i = 0; i < n; i++) {
                int yi = fxp12::floorToInt(y0f + dyf * i / n);
                set_pixel(xi, yi, pen);
                xi += xi_step;
            }
        }
        else {
            int yi = fxp12::floorToInt(y0f);
            int n = FXP_ABS(fxp12::floorToInt(y1f) - yi);
            int yi_step = dyf >= 0 ? 1 : -1;
            for (int i = 0; i < n; i++) {
                int xi = fxp12::floorToInt(x0f + dxf * i / n);
                set_pixel(xi, yi, pen);
                yi += yi_step;
            }
        }
    }

    void fill_ellipse_f(int x, int y, int w, int h, pen_t pen = pen_t::WHITE) {
        fill_ellipse_f(Rect(x, y, w, h), pen);
    }

    void fill_ellipse_f(const Rect rectf, pen_t pen = pen_t::WHITE) {
        Rect dest_rect = rectf;
        const int r = (dest_rect.r() + fxp12::ONE - 1) / fxp12::ONE;
        const int b = (dest_rect.b() + fxp12::ONE - 1) / fxp12::ONE;
        dest_rect.x /= fxp12::ONE;
        dest_rect.y /= fxp12::ONE;
        dest_rect.w = r - dest_rect.x;
        dest_rect.h = b - dest_rect.y;
        dest_rect = clip_rect(dest_rect, width, height);
        if (dest_rect.w <= 0 || dest_rect.h <= 0) return;
        const int rxf = rectf.w / 2;
        const int ryf = rectf.h / 2;
        const int cxf = rectf.x + rxf;
        const int cyf = rectf.y + ryf;
        const int dx0 = dest_rect.x;
        const int dy0 = dest_rect.y;
        const int dx1 = dest_rect.r();
        const int dy1 = dest_rect.b();
        
        // 楕円内かどうかをピクセル中心で判定するため 0.5px オフセットする
        int yf = dy0 * fxp12::ONE + (fxp12::ONE / 2);
        for (int y = dy0; y < dy1; y++) {
            int rdy = (yf - cyf) * fxp12::ONE / ryf;
            int rdy2 = rdy * rdy;
            int xf = dx0 * fxp12::ONE + (fxp12::ONE / 2);
            for (int x = dx0; x < dx1; x++) {
                int rdx = (xf - cxf) * fxp12::ONE / rxf;
                int rdx2 = rdx * rdx;
                if (rdx2 + rdy2 < fxp12::ONE * fxp12::ONE) {
                    set_pixel(x, y, pen);
                }
                xf += fxp12::ONE;
            }
            yf += fxp12::ONE;
        }
    }

    void drawBitmap(int x0, int y0, const uint8_t *bitmap) {
        int w = ((int)bitmap[1] << 8) | ((int)bitmap[0]);
        int h = ((int)bitmap[3] << 8) | ((int)bitmap[2]);
        int stride = (w + 7) / 8;
        drawBitmap(x0, y0, bitmap + 4, 0, 0, w, h, stride);
    }

    int drawString(const TinyFont &font, int dx0, int dy0, const char* s) {
        int n = strlen(s);
        const char *c = s;
        for (int i = 0; i < n; i++) {
            dx0 += drawChar(font, dx0, dy0, *(c++)) + font.spacing;
        }
        return dx0;
    }

    int drawChar(const TinyFont &font, int dx0, int dy0, char c) {
        const TinyFontGlyph *glyph = font.getGlyph(c);
        if (!glyph) return 0;
        if (!glyph->isBlank()) {
            drawBitmap(dx0, dy0, font.bitmap + glyph->offset, 0, 0, glyph->width, font.height, (glyph->width + 7) / 8);
        }
        return glyph->width;
    }

    void drawBitmap(int dx0, int dy0, const uint8_t *src, int sx0, int sy0, int w, int h, int sstride) {
        const uint8_t *line_ptr = src + sy0 * sstride + sx0 / 8;
        for (int y = 0; y < h; y++) {
            int dy = dy0 + y;
            const uint8_t *rd_ptr = line_ptr;
            uint8_t sreg = 0;
            if (sx0 % 8 != 0) {
                sreg = *(rd_ptr++);
                sreg >>= (sx0 % 8);
            }
            for (int x = 0; x < w; x++) {
                int sx = sx0 + x;
                int dx = dx0 + x;
                if (sx % 8 == 0) {
                    sreg = *(rd_ptr++);
                }
                if (sreg & 1) {
                    set_pixel(dx, dy, pen_t::WHITE);
                }
                sreg >>= 1;
            }
            line_ptr += sstride;
        }
    }

};

}
