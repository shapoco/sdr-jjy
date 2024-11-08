#include <stdio.h>
#include "stdint.h"
#include "math.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"

#include "fixed12.hpp"
#include "atomic.hpp"
#include "dma_adc.hpp"
#include "jjy/jjy.hpp"

#include "jjymon.hpp"
#include "core1.hpp"

#define ENABLE_STDOUT (1)

static constexpr uint32_t SYS_CLK_FREQ = 120 * MHZ;
static constexpr uint32_t ADC_SPS = jjy::rx::DETECTION_INPUT_SPS;
static constexpr uint32_t DMA_SIZE = jjy::rx::PREFERRED_DMA_SIZE;

static constexpr int PIN_ADC_IN = 26;
static constexpr int PIN_LED_OUT = 25;
static constexpr int PIN_SPEAKER_OUT = 28;
static constexpr int PIN_LAMP_OUT = 15;

static constexpr uint32_t SPEAKER_FREQ = 440;
static constexpr uint32_t SPEAKER_SAMPLE_BITS = 16;
static constexpr uint32_t SPEAKER_PWM_PERIOD = 1 << SPEAKER_SAMPLE_BITS;

DmaAdc<PIN_ADC_IN, ADC_SPS, DMA_SIZE> dma_adc;
jjy::rx::Receiver receiver;

atomic<receiver_status_t> glb_receiver_status;

static receiver_status_t sts;

int main() {
    set_sys_clock_khz(SYS_CLK_FREQ / KHZ, true);
    sleep_ms(100);

#if ENABLE_STDOUT
    stdio_init_all();
    sleep_ms(500);
    printf("Start.\n");
    printf("receiver.rf.det_delay_ms = %d\n", receiver.rf.det_delay_ms);
    printf("receiver.rf.anti_chat_delay_ms = %d\n", receiver.rf.anti_chat_delay_ms);
#endif

    // Setup fixed-point library
    fxp12::init_tables();

    // Setup LED pin
    gpio_init(PIN_LED_OUT);
    gpio_set_dir(PIN_LED_OUT, GPIO_OUT);

    // Setup Lamp pin
    gpio_init(PIN_LAMP_OUT);
    gpio_set_dir(PIN_LAMP_OUT, GPIO_OUT);

    // Setup speaker out
    {
        gpio_init(PIN_SPEAKER_OUT);
        gpio_set_dir(PIN_SPEAKER_OUT, GPIO_OUT);
        gpio_set_function(PIN_SPEAKER_OUT, GPIO_FUNC_PWM);

        const float pwm_clkdiv = ((float)SYS_CLK_FREQ / SPEAKER_FREQ) / SPEAKER_PWM_PERIOD;
        pwm_config pwm_cfg = pwm_get_default_config();
        pwm_config_set_clkdiv(&pwm_cfg, pwm_clkdiv);
        pwm_config_set_wrap(&pwm_cfg, SPEAKER_PWM_PERIOD - 1);
        pwm_init(pwm_gpio_to_slice_num(PIN_SPEAKER_OUT), &pwm_cfg, true);
        pwm_set_gpio_level(PIN_SPEAKER_OUT, 0);
    }

    adc_init();
    dma_adc.init();
    sleep_ms(100);

    core1_init();
    multicore_launch_core1(core1_main);

    dma_adc.run();

    uint64_t t_last_us = to_us_since_boot(get_absolute_time());
    uint32_t t_dma_us = 0, t_calc_us = 0;
    uint32_t t_next_print_ms = t_last_us / 1000;

    receiver.init(jjy::EAST_40KHZ, to_ms_since_boot(get_absolute_time()));

    while(true) {

        uint64_t t_now_us = to_us_since_boot(get_absolute_time());
        t_calc_us += t_now_us - t_last_us;
        t_last_us = t_now_us;

        const uint16_t* dma_buff = dma_adc.read();

        t_now_us = to_us_since_boot(get_absolute_time());
        t_dma_us += t_now_us - t_last_us;
        t_last_us = t_now_us;
        
        uint32_t t_now_ms = t_now_us / 1000;

        receiver.process(t_now_ms, dma_buff);
        
        sts.rf = receiver.rf.get_status();
        sts.sync = receiver.sync.get_status();
        sts.dec = receiver.dec.get_status();
        glb_receiver_status.store(sts);

        // Output
        gpio_put(PIN_LED_OUT, sts.rf.hyst_dig_out);
        gpio_put(PIN_LAMP_OUT, !sts.rf.digital_out);
        pwm_set_gpio_level(PIN_SPEAKER_OUT, sts.rf.digital_out ? SPEAKER_PWM_PERIOD / 2 : 0);

        if (t_now_ms >= t_next_print_ms) {
            t_next_print_ms += 1000;

#if ENABLE_STDOUT
            float core0usage = (float)(100 * t_calc_us) / (t_calc_us + t_dma_us);
            float gain = (float)sts.rf.agc_gain / (1 << jjy::PREC);
            float qty = (float)sts.rf.signal_quarity / (1 << jjy::PREC);
            int beat_det = sts.rf.beat_detected ? 1 : 0;
            printf("AdcLv:%3d, AGC:%6.2f, Base/Peak:%4d/%4d, Beat=%d, Qty:%4.2f, Core0Usage:%6.2f%%\n",
                (int)sts.rf.adc_amplitude_peak, gain, (int)sts.rf.det_anl_out_base, (int)sts.rf.det_anl_out_peak, beat_det, qty, core0usage);
#endif

            t_calc_us = 0;
            t_dma_us = 0;
        }


    }

    return 0;
}