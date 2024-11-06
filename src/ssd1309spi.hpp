#ifndef SSD1309_SPI_HPP
#define SSD1309_SPI_HPP

#include <stdint.h>
#include <string.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "fixed12.hpp"
#include "bmpfont/bmpfont.hpp"

namespace ssd1309spi {

template<typename T>
static inline T min(T a, T b) { return a < b ? a : b; }

template<typename T>
static inline T max(T a, T b) { return a > b ? a : b; }

template<typename T>
static inline T clip(T min, T max, T value) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

class Rect {
public:
    int x, y, w, h;
    Rect() : x(0), y(0), w(0), h(0) {}
    Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) { }
    int r() const { return x + w; }
    int b() const { return y + h; }
    int cx() const { return (x + w / 2); }
    int cy() const { return (y + h / 2); }
};

Rect clip_rect(const Rect rect, int w, int h);

using seg_t = uint8_t;

typedef enum : bool {
    CMD = false,
    DATA = true,
} dc_t;

typedef enum {
    SET_MEM_MODE        = 0x20,
    SET_COL_ADDR        = 0x21,
    SET_PAGE_ADDR       = 0x22,
    SET_HORIZ_SCROLL    = 0x26,
    SET_SCROLL          = 0x2E,
    SET_DISP_START_LINE = 0x40,
    SET_CONTRAST        = 0x81,
    SET_CHARGE_PUMP     = 0x8D,
    SET_SEG_REMAP       = 0xA0,
    SET_ENTIRE_ON       = 0xA4,
    SET_ALL_ON          = 0xA5,
    SET_NORM_DISP       = 0xA6,
    SET_INV_DISP        = 0xA7,
    SET_MUX_RATIO       = 0xA8,
    SET_DISP            = 0xAE,
    SET_COM_OUT_DIR     = 0xC0,
    SET_COM_OUT_DIR_FLIP= 0xC0,
    SET_DISP_OFFSET     = 0xD3,
    SET_DISP_CLK_DIV    = 0xD5,
    SET_PRECHARGE       = 0xD9,
    SET_COM_PIN_CFG     = 0xDA,
    SET_VCOM_DESEL      = 0xDB,
} cmd_t;

typedef enum : uint8_t {
    BLACK = 0,
    WHITE = 1,
} pen_t;

template<int W, int H, int SPI_INDEX, uint32_t SPI_FREQ, int PIN_RES_N, int PIN_CS_N, int PIN_DC, int PIN_SCLK, int PIN_MOSI>
class Lcd {
public:

    static constexpr int PAGE_H = 8;
    static constexpr int NUM_PAGES = (H + PAGE_H - 1) / PAGE_H;
    static constexpr int NUM_PLANES = 2;
    static constexpr int NUM_SEGS = W * NUM_PAGES;

    static_assert(
        (W == 128 && H == 64) ||
        (W == 128 && H == 32));

private:
    spi_inst_t* spi = NULL;
    int dma_ch = 0;
    dma_channel_config dma_cfg;
    seg_t front_buff[NUM_SEGS];
    seg_t back_buff[NUM_SEGS];

    int curr_page = 0;
    int num_sent_pages = 0;

public:
    Lcd() : spi(SPI_INDEX == 0 ? spi0 : spi1) { }

    void init() {
        curr_page = 0;
        num_sent_pages = NUM_PAGES;
        memset(back_buff, 0x00, NUM_SEGS * sizeof(seg_t));
        memset(front_buff, 0x00, NUM_SEGS * sizeof(seg_t));

        gpio_init(PIN_RES_N);
        gpio_init(PIN_CS_N);
        gpio_init(PIN_DC);

        gpio_put(PIN_RES_N, false);
        gpio_put(PIN_CS_N, true);
        gpio_put(PIN_DC, dc_t::CMD);

        gpio_set_dir(PIN_RES_N, GPIO_OUT);
        gpio_set_dir(PIN_CS_N, GPIO_OUT);
        gpio_set_dir(PIN_DC, GPIO_OUT);

        spi_init(spi, SPI_FREQ);
        gpio_set_function(PIN_SCLK, GPIO_FUNC_SPI);
        gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

        //bi_decl(bi_2pins_with_func(PIN_MISO, PIN_MOSI, PIN_SCLK, GPIO_FUNC_SPI));
        //bi_decl(bi_1pin_with_name(PIN_CS_N, "SPI CS"));

        sleep_ms(100); // check
        gpio_put(PIN_RES_N, true);
        sleep_ms(100); // check

        dma_ch = dma_claim_unused_channel(true);
        dma_cfg = dma_channel_get_default_config(dma_ch);
        channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_8);
        channel_config_set_dreq(&dma_cfg, spi_get_dreq(spi, true));
        
        set_disp(false);
        write_cmd(cmd_t::SET_MEM_MODE, 0);
        write_cmd(cmd_t::SET_DISP_START_LINE);
        write_cmd(cmd_t::SET_SEG_REMAP | 0x01);
        write_cmd(cmd_t::SET_MUX_RATIO, H - 1);
        write_cmd(cmd_t::SET_COM_OUT_DIR | 0x08);
        write_cmd(cmd_t::SET_DISP_OFFSET, 0);
        write_cmd(cmd_t::SET_COM_PIN_CFG, 
            (W == 128 && H == 32) ? 0x02 :
            (W == 128 && H == 64) ? 0x12 : 0x02);
        write_cmd(cmd_t::SET_DISP_CLK_DIV, 0x80);
        write_cmd(cmd_t::SET_PRECHARGE, 0xF1);
        write_cmd(cmd_t::SET_VCOM_DESEL, 0x30);
        write_cmd(cmd_t::SET_CONTRAST, 0xFF);
        write_cmd(cmd_t::SET_ENTIRE_ON);
        write_cmd(cmd_t::SET_NORM_DISP);
        write_cmd(cmd_t::SET_CHARGE_PUMP, 0x14);
        write_cmd(cmd_t::SET_SCROLL);

        write_cmd(cmd_t::SET_COL_ADDR, 0, W - 1);
        write_cmd(cmd_t::SET_PAGE_ADDR, 0, H - 1);
        write_blocking(dc_t::DATA, front_buff, NUM_SEGS * sizeof(seg_t));

        set_disp(true);
    }

    void set_disp(bool on) {
        write_cmd(cmd_t::SET_DISP | (on ? 0x01 : 0x00));
    }

    void clear(seg_t c = 0x00) {
        memset(back_buff, c, NUM_SEGS * sizeof(seg_t));
    }

    inline int get_seg_index(int x, int y) {
        return (y / PAGE_H) * W + x;
    }

    void set_pixel(int x, int y, pen_t c = pen_t::WHITE) {
        if (x < 0 || W <= x || y < 0 || H <= y) return;
        uint8_t mask = 1 << (y % PAGE_H);
        int iseg = get_seg_index(x, y);
        switch (c) {
        case pen_t::BLACK: back_buff[iseg] &= ~mask; break;
        case pen_t::WHITE: back_buff[iseg] |= mask; break;
        }
    }

    void fill_rect(int x, int y, int w, int h, pen_t pen = pen_t::WHITE) {
        fill_rect(Rect(x, y, w, h), pen);
    }

    void fill_rect(Rect rect, pen_t c = pen_t::WHITE) {
        rect = clip_rect(rect, W, H);
        if (rect.w <= 0 || rect.h <= 0) return;
        int x = rect.x;
        int y = rect.y;
        int w = rect.w;
        int h = rect.h;
        int r = rect.r();
        int b = rect.b();

        int first_page = y / PAGE_H;
        int final_page = (b - 1) / PAGE_H;
        seg_t first_seg = ~(seg_t)((1 << (y % PAGE_H)) - 1);
        seg_t final_seg = (1 << (((b + PAGE_H - 1) % PAGE_H) + 1)) - 1;
        
        if (first_page == final_page) {
            first_seg &= final_seg;
        }
        
        for (int p = first_page; p <= final_page; p++) {
            seg_t mask =
                (p == first_page) ? first_seg :
                (p == final_page) ? final_seg :
                ~(seg_t)0;
            seg_t *wr_ptr = &back_buff[p * W + x];

            if (~mask == 0) {
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

    void draw_rect(int x, int y, int w, int h, pen_t pen = pen_t::WHITE) {
        fill_rect(x, y, w + 1, 1);
        fill_rect(x, y + 1, 1, h - 1);
        fill_rect(x + w, y + 1, 1, h - 1);
        fill_rect(x, y + h, w + 1, 1);
    }

    void draw_line(int x0, int y0, int x1, int y1, pen_t pen = pen_t::WHITE) {
        x0 *= fxp12::ONE;
        y0 *= fxp12::ONE;
        x1 *= fxp12::ONE;
        y1 *= fxp12::ONE;
        draw_line_f(x0, y0, x1, y1, pen);
    }

    void draw_line_f(int32_t x0f, int32_t y0f, int32_t x1f, int32_t y1f, pen_t pen = pen_t::WHITE) {
        int32_t dxf = x1f - x0f;
        int32_t dyf = y1f - y0f;
        if (FXP_ABS(dxf) > FXP_ABS(dyf)) {
            int xi = fxp12::to_int(x0f);
            int n = FXP_ABS(fxp12::to_int(x1f) - xi);
            int xi_step = dxf >= 0 ? 1 : -1;
            for (int i = 0; i < n; i++) {
                int yi = fxp12::to_int(y0f + dyf * i / n);
                set_pixel(xi, yi, pen);
                xi += xi_step;
            }
        }
        else {
            int yi = fxp12::to_int(y0f);
            int n = FXP_ABS(fxp12::to_int(y1f) - yi);
            int yi_step = dyf >= 0 ? 1 : -1;
            for (int i = 0; i < n; i++) {
                int xi = fxp12::to_int(x0f + dxf * i / n);
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
        dest_rect = clip_rect(dest_rect, W, H);
        if (dest_rect.w <= 0 || dest_rect.h <= 0) return;
        const int rxf = rectf.w / 2;
        const int ryf = rectf.h / 2;
        const int cxf = rectf.x + rxf;
        const int cyf = rectf.y + ryf;
        const int dx0 = dest_rect.x;
        const int dy0 = dest_rect.y;
        const int dx1 = dest_rect.r();
        const int dy1 = dest_rect.b();
        constexpr int R_MUL = (1 << 4);
        
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

    void draw_bitmap(int x0, int y0, const uint8_t *bitmap) {
        int w = ((int)bitmap[1] << 8) | ((int)bitmap[0]);
        int h = ((int)bitmap[3] << 8) | ((int)bitmap[2]);
        int stride = (w + 7) / 8;
        draw_bitmap(x0, y0, bitmap + 4, 0, 0, w, h, stride);
    }

    int draw_string(const bmpfont::Font &font, int dx0, int dy0, const char* s) {
        int n = strlen(s);
        const char *c = s;
        for (int i = 0; i < n; i++) {
            dx0 += draw_char(font, dx0, dy0, *(c++)) + font.spacing;
        }
        return dx0;
    }

    int draw_char(const bmpfont::Font &font, int dx0, int dy0, char c) {
        if (!font.contains_char(c)) {
            return 0;
        }
        const bmpfont::CharInfo &ci = font.get_char_info(c);
        draw_bitmap(dx0, dy0, font.bitmap + ci.offset, 0, 0, ci.width, font.height, (ci.width + 7) / 8);
        return ci.width;
    }

    void draw_bitmap(int dx0, int dy0, const uint8_t *src, int sx0, int sy0, int w, int h, int sstride) {
        const uint8_t *line_ptr = src + sy0 * sstride + sx0 / 8;
        for (int y = 0; y < h; y++) {
            int dy = dy0 + y;
            const uint8_t *rd_ptr = line_ptr;
            uint8_t sreg;
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

    void commit() {
        memcpy(front_buff, back_buff, NUM_SEGS * sizeof(seg_t));
        num_sent_pages = 0;
    }

    void service() {
        if (num_sent_pages < NUM_PAGES) {
            write_cmd(cmd_t::SET_COL_ADDR, 0, W - 1);
            write_cmd(cmd_t::SET_PAGE_ADDR, curr_page, curr_page);
            //write_dma_start(dc_t::DATA, front_buff + W * curr_page, sizeof(seg_t) * W);
            write_blocking(dc_t::DATA, front_buff + W * curr_page, sizeof(seg_t) * W);
            num_sent_pages += 1;
            if (num_sent_pages < NUM_PAGES) {
                curr_page = (curr_page + 1) % NUM_PAGES;
            }
            else {
                curr_page = 0;
            }
        }
    }

    void write_cmd(const uint8_t cmd) {
        const uint8_t buf[] = { cmd };
        write_blocking(dc_t::CMD, buf, 1);
    }

    void write_cmd(const uint8_t cmd, const uint8_t param0) {
        const uint8_t buf[] = { cmd, param0 };
        write_blocking(dc_t::CMD, buf, 2);
    }

    void write_cmd(const uint8_t cmd, const uint8_t param0, const uint8_t param1) {
        const uint8_t buf[] = { cmd, param0, param1 };
        write_blocking(dc_t::CMD, buf, 3);
    }

    void write_blocking(const dc_t dc, const void *src, const size_t size_in_bytes) {
        write_dma_complete();
        spi_select(dc);
        spi_write_blocking(spi, (const uint8_t*)src, size_in_bytes);
        spi_deselect();
    }

    void write_dma_start(const dc_t dc, const void *src, const size_t size_in_bytes) {
        write_dma_complete();
        spi_select(dc);
        dma_channel_configure(dma_ch, &dma_cfg, &spi_get_hw(spi)->dr, src, size_in_bytes, true); 
    }

    void write_dma_complete() {
        if (is_dma_busy()) {
            dma_channel_wait_for_finish_blocking(dma_ch);
        }
        spi_deselect();
    }

    bool is_dma_busy() {
        return dma_channel_is_busy(dma_ch);
    }

    void spi_select(dc_t dc) {
        gpio_put(PIN_DC, dc);
        gpio_put(PIN_CS_N, false);
        sleep_us(10);
    }

    void spi_deselect() {
        sleep_us(10);
        gpio_put(PIN_CS_N, true);
        sleep_us(10);
    }
};

}

#endif
