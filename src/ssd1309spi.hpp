#ifndef SSD1309_SPI_HPP
#define SSD1309_SPI_HPP

#include <stdint.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"

template<int W, int H, int SPI_INDEX, uint32_t SPI_FREQ, int PIN_RES_N, int PIN_CS_N, int PIN_DC, int PIN_SCLK, int PIN_MOSI>
class Ssd1309Spi {
public:
    static constexpr int NUM_PAGES = H / 8;

    static_assert(
        (W == 128 && H == 64) ||
        (W == 128 && H == 32));

    spi_inst_t* spi;
    int dma_ch;
    dma_channel_config dma_cfg;

    uint8_t vram[W * H];

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

public:

    Ssd1309Spi() : spi(SPI_INDEX == 0 ? spi0 : spi1) { }

    void init() {
        gpio_init(PIN_RES_N);
        gpio_init(PIN_CS_N);
        gpio_init(PIN_DC);

        gpio_put(PIN_RES_N, false);
        gpio_put(PIN_CS_N, true);
        gpio_put(PIN_DC, dc_t::CMD);

        gpio_set_dir(PIN_RES_N, GPIO_OUT);
        gpio_set_dir(PIN_CS_N, GPIO_OUT);
        gpio_set_dir(PIN_DC, GPIO_OUT);

        // Enable SPI at 1 MHz and connect to GPIOs
        spi_init(spi, SPI_FREQ);
        gpio_set_function(PIN_SCLK, GPIO_FUNC_SPI);
        gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

        // Make the SPI pins available to picotool
        //bi_decl(bi_2pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PIN_MOSI, PIN_SCLK, GPIO_FUNC_SPI));
        // Make the CS pin available to picotool
        //bi_decl(bi_1pin_with_name(PIN_CS_N, "SPI CS"));

        dma_ch = dma_claim_unused_channel(true);
        dma_cfg = dma_channel_get_default_config(dma_ch);
        channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_8);
        channel_config_set_dreq(&dma_cfg, spi_get_dreq(spi, true));
        
        sleep_ms(100); // check
        gpio_put(PIN_RES_N, true);
        sleep_ms(100); // check

        write_cmd(cmd_t::SET_DISP);
        write_cmd(cmd_t::SET_MEM_MODE, 0);
        write_cmd(cmd_t::SET_DISP_START_LINE);
        write_cmd((cmd_t)(cmd_t::SET_SEG_REMAP | 0x01));
        write_cmd(cmd_t::SET_MUX_RATIO, H - 1);
        write_cmd((cmd_t)(cmd_t::SET_COM_OUT_DIR | 0x08));
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
        write_cmd((cmd_t)(cmd_t::SET_DISP | 0x01));

        sleep_ms(100);
        write_cmd(cmd_t::SET_COL_ADDR, 0, 7);
        write_cmd(cmd_t::SET_PAGE_ADDR, 0, 0);
        uint8_t test_data[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
        write_blocking(dc_t::DATA, test_data, 8);
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

    void write_blocking(const dc_t dc, const uint8_t *src, const uint32_t length) {
        write_dma_complete();
        gpio_put(PIN_DC, dc);
        spi_select();
        spi_write_blocking(spi, src, length);
        spi_deselect();
    }

    void write_dma_start(const dc_t dc, const uint8_t *src, const uint32_t length) {
        write_dma_complete();
        dma_channel_configure(dma_ch, &dma_cfg,
            &spi_get_hw(spi)->dr, // write address
            src, // read address
            length, // element count (each element is of size transfer_data_size)
            false // don't start yet
        ); 
        gpio_put(PIN_DC, dc);
        spi_select();
        dma_channel_start(dma_ch);
    }

    void write_dma_complete() {
        if (dma_channel_is_busy(dma_ch)) {
            dma_channel_wait_for_finish_blocking(dma_ch);
        }
        spi_deselect();
    }

    void spi_select() {
        gpio_put(PIN_CS_N, false);
    }

    void spi_deselect() {
        gpio_put(PIN_CS_N, true);
    }
};

#endif
