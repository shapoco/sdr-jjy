#ifndef DMA_ADC_HPP
#define DMA_ADC_HPP

#include <stdint.h>

#include "hardware/dma.h"
#include "hardware/adc.h"

static constexpr uint32_t ADC_CLK_FREQ = 48e6;

template<int PIN, int SPS, int DMA_SIZE>
class DmaAdc {
public:
    uint32_t pin;
    uint32_t dma_ch;

private:
    static constexpr uint32_t ADC_DIVISOR = ADC_CLK_FREQ / SPS;
    static_assert(ADC_DIVISOR >= 96);
    static constexpr int NUM_BANKS = 2;
    dma_channel_config dma_cfg;
    uint16_t dma_buff[DMA_SIZE * NUM_BANKS];
    int bank = 0;

public:
    void init() {
        // Setup ADC
        adc_gpio_init(PIN);
        adc_select_input(0);
        adc_fifo_setup(
            true,   // Write each completed conversion to the sample FIFO
            true,   // Enable DMA data request (DREQ)
            1,      // DREQ (and IRQ) asserted when at least 1 sample present
            false,  // We won't see the ERR bit because of 8 bit reads; disable.
            false   // no shift
        );
        adc_set_clkdiv(ADC_DIVISOR - 1);

        // Setup DMA
        dma_ch = dma_claim_unused_channel(true);
        dma_cfg = dma_channel_get_default_config(this->dma_ch);
        channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
        channel_config_set_read_increment(&dma_cfg, false);
        channel_config_set_write_increment(&dma_cfg, true);
        channel_config_set_dreq(&dma_cfg, DREQ_ADC);
    }

    void run() {
        adc_run(true);
        start_dma();
    }

    const uint16_t* read() {
        const uint16_t *result = finish_dma();
        start_dma();
        return result;
    }

private:
    void start_dma() {
        uint16_t *buff = dma_buff + bank * DMA_SIZE;
        dma_channel_configure(dma_ch, &dma_cfg,
            buff,           // dst
            &adc_hw->fifo,  // src
            DMA_SIZE,       // transfer count
            true            // start immediately
        );
    }

    const uint16_t* finish_dma() {
        dma_channel_wait_for_finish_blocking(dma_ch);
        const uint16_t *result = dma_buff + bank * DMA_SIZE;
        bank = (bank + 1) % NUM_BANKS;
        return result;
    }
};

#endif
