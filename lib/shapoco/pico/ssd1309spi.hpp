#pragma once

#include <stdint.h>
#include <string.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "shapoco/fixed12.hpp"
#include "shapoco/graphics/graphics.hpp"
#include "shapoco/graphics/ssd130x/ssd130x.hpp"

namespace shapoco::pico {

using namespace shapoco::graphics;
using namespace shapoco::graphics::ssd130x;

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

template<int W, int H, int SPI_INDEX, uint32_t SPI_FREQ, int PIN_RES_N, int PIN_CS_N, int PIN_DC, int PIN_SCLK, int PIN_MOSI>
class Ssd1309Spi {
public:

    static constexpr int PAGE_H = 8;
    static constexpr int NUM_PAGES = (H + PAGE_H - 1) / PAGE_H;
    static constexpr int NUM_PLANES = 2;
    static constexpr int NUM_SEGS = W * NUM_PAGES;

    static_assert(
        (W == 128 && H == 64) ||
        (W == 128 && H == 32));

    Screen screen;

private:
    spi_inst_t* spi = NULL;
    int dma_ch = 0;
    dma_channel_config dma_cfg;
    int curr_page = 0;
    int num_sent_pages = 0;

public:
    Ssd1309Spi() : spi(SPI_INDEX == 0 ? spi0 : spi1), screen(W, H) { }

    void init() {
        curr_page = 0;
        num_sent_pages = NUM_PAGES;

        screen.clear();

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
        write_blocking(dc_t::DATA, screen.data, NUM_SEGS * sizeof(seg_t));

        set_disp(true);
    }

    void set_disp(bool on) {
        write_cmd(cmd_t::SET_DISP | (on ? 0x01 : 0x00));
    }

    void commit(const ssd130x::Screen &back_buff) {
        memcpy(screen.data, back_buff.data, NUM_SEGS * sizeof(seg_t));
        num_sent_pages = 0;
    }

    void service() {
        if (num_sent_pages < NUM_PAGES) {
            write_cmd(cmd_t::SET_COL_ADDR, 0, W - 1);
            write_cmd(cmd_t::SET_PAGE_ADDR, curr_page, curr_page);
            //write_dma_start(dc_t::DATA, front_buff.back_buff + W * curr_page, sizeof(seg_t) * W);
            write_blocking(dc_t::DATA, screen.data + W * curr_page, sizeof(seg_t) * W);
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
