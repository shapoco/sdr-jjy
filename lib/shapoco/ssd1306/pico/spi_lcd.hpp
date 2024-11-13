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
#include "shapoco/ssd1306/pico/driver_base.hpp"
#include "shapoco/ssd1306/ssd1306.hpp"

namespace shapoco::ssd1306::pico {

template<int W, int H, int SPI_INDEX, int PIN_RES_N, int PIN_CS_N, int PIN_DC, int PIN_SCLK, int PIN_MOSI, uint32_t SPI_FREQ = 1000 * 1000>
class SpiLcd : public DriverBase<W, H> {
private:
    using base = DriverBase<W, H>;

    spi_inst_t * const spi;
    int dma_ch = 0;
    dma_channel_config dma_cfg;

public:
    SpiLcd() : spi(SPI_INDEX == 0 ? spi0 : spi1) { }

    void init() override {
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

        sleep_ms(100); // check
        gpio_put(PIN_RES_N, true);
        sleep_ms(100); // check

        dma_ch = dma_claim_unused_channel(true);
        dma_cfg = dma_channel_get_default_config(dma_ch);
        channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_8);
        channel_config_set_dreq(&dma_cfg, spi_get_dreq(spi, true));

        base::init();
    }

    void writeBlocking(dc_t dc, const void *src, size_t size_in_bytes) override {
        writeDataDmaComplete();
        spiSelect(dc);
        spi_write_blocking(spi, (const uint8_t*)src, size_in_bytes);
        spiDeselect();
    }

    void writeDataDmaStart(dc_t dc, const void *src, size_t size_in_bytes) override {
        writeDataDmaComplete();
        spiSelect(dc);
        dma_channel_configure(dma_ch, &dma_cfg, &spi_get_hw(spi)->dr, src, size_in_bytes, true); 
    }

    void writeDataDmaComplete() override {
        if (isDmaBusy()) {
            dma_channel_wait_for_finish_blocking(dma_ch);
        }
        spiDeselect();
    }

    bool isDmaBusy() override {
        return dma_channel_is_busy(dma_ch);
    }

    void spiSelect(dc_t dc) {
        gpio_put(PIN_DC, dc);
        gpio_put(PIN_CS_N, false);
        sleep_us(10);
    }

    void spiDeselect() {
        sleep_us(10);
        gpio_put(PIN_CS_N, true);
        sleep_us(10);
    }
};

}
