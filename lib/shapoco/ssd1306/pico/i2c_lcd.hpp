#pragma once

#include <stdint.h>
#include <string.h>

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "shapoco/fixed12.hpp"
#include "shapoco/graphics/graphics.hpp"
#include "shapoco/ssd1306/pico/driver_base.hpp"
#include "shapoco/ssd1306/ssd1306.hpp"

namespace shapoco::ssd1306::pico {

template<int W, int H, int I2C_INDEX, int PIN_SDA, int PIN_SCL, uint8_t I2C_ADDR = 0x3c, uint32_t I2C_FREQ = 400 * 1000>
class I2cLcd : public DriverBase<W, H> {
private:
    using base = DriverBase<W, H>;

    i2c_inst_t * const i2c;
    int dma_ch = 0;
    dma_channel_config dma_cfg;

    uint8_t txBuff[W + 2];

public:
    I2cLcd() : i2c(I2C_INDEX == 0 ? i2c0 : i2c1) { }

    void init() override {
        gpio_init(PIN_SDA);
        gpio_init(PIN_SCL);

        gpio_pull_up(PIN_SDA);
        gpio_pull_up(PIN_SCL);

        i2c_init(i2c, I2C_FREQ);
        gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
        gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);

        dma_ch = dma_claim_unused_channel(true);
        dma_cfg = dma_channel_get_default_config(dma_ch);
        channel_config_set_read_increment(&dma_cfg, true);
        channel_config_set_write_increment(&dma_cfg, false);
        channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_8);
        channel_config_set_dreq(&dma_cfg, i2c_get_dreq(i2c, true));

        base::init();
    }

    void writeBlocking(dc_t dc, const void *src, size_t sizeInBytes) override {
        writeDataDmaComplete();
        size_t txBuffSize;
        if (dc == dc_t::CMD) {
            const uint8_t *srcBytes = (const uint8_t *)src;
            for (size_t i = 0; i < sizeInBytes; i++) {
                txBuff[i * 2 + 0] = 0x80;
                txBuff[i * 2 + 1] = srcBytes[i];
            }
            txBuffSize = sizeInBytes * 2;
        }
        else {
            txBuff[0] = 0x40;
            memcpy(txBuff + 1, src, sizeInBytes);
            txBuffSize = sizeInBytes + 1;
        }
        i2c_write_blocking(i2c, I2C_ADDR, txBuff, txBuffSize, false);
    }

    void writeDataDmaStart(dc_t dc, const void *src, size_t sizeInBytes) override {
        writeDataDmaComplete();
        i2c_hw_t *hw = i2c_get_hw(i2c);
        hw->enable = 0;
        hw->tar = I2C_ADDR;
        hw->enable = 1;
        dma_channel_configure(dma_ch, &dma_cfg, &(hw->data_cmd), src, sizeInBytes, true); 
    }

    void writeDataDmaComplete() override {
        if (isDmaBusy()) {
            dma_channel_wait_for_finish_blocking(dma_ch);
        }
    }

    bool isDmaBusy() override {
        return dma_channel_is_busy(dma_ch);
    }
};

}
