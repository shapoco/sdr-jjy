#ifndef DMA_ADC_HPP
#define DMA_ADC_HPP

#include <stdint.h>

#include "hardware/dma.h"
#include "hardware/adc.h"

static constexpr uint32_t ADC_CLK_FREQ = 48e6;

template<int PIN, int FREQ, int DMA_SIZE>
class DmaAdc {
public:
    static constexpr uint32_t ADC_DIVISOR = ADC_CLK_FREQ / FREQ;
    
    static_assert(ADC_DIVISOR == 0 || ADC_DIVISOR >= 96);

private:
    uint16_t dma_buff[DMA_SIZE];
    uint dma_chan;
    dma_channel_config cfg;
    uint32_t pin;

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
        dma_chan = dma_claim_unused_channel(true);
        cfg = dma_channel_get_default_config(this->dma_chan);
        channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
        channel_config_set_read_increment(&cfg, false);
        channel_config_set_write_increment(&cfg, true);
        channel_config_set_dreq(&cfg, DREQ_ADC);
    }

    void run() {
        adc_run(true);
    }

    const uint16_t* read() {
        dma_channel_configure(dma_chan, &cfg,
            dma_buff,       // dst
            &adc_hw->fifo,  // src
            DMA_SIZE,       // transfer count
            true            // start immediately
        );
        dma_channel_wait_for_finish_blocking(dma_chan);
        return dma_buff;
    }

};

#endif
