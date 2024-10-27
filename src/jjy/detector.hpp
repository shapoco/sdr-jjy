#ifndef JJY_DETECTOR_HPP
#define JJY_DETECTOR_HPP

#include <stdint.h>
#include <math.h>

#include "pico/stdlib.h"

#include "jjy/utils.hpp"

namespace jjy {

static constexpr int PREC = 12;

static constexpr int DET_RESO = 1 << 3;

static constexpr int ANTI_CHATTERING_DELAY = 3;
static constexpr uint32_t PULSE_WIDTH_LIMIT_MS = 1000;

static constexpr uint32_t DET_PERIOD = 1000;

typedef struct {
public:
    uint32_t adc_offset;
    uint32_t agc_amp_raw;
    uint32_t adc_level;
    uint8_t raw_signal;
    uint8_t stabled_signal;
    bool agc_updated;
    uint32_t quarity_raw;

    uint32_t rcv_level_base;
    uint32_t rcv_level_peak;

    const float agc_amp() const { return (float)agc_amp_raw / (1 << PREC); }
    const float quarity() const { return (float)quarity_raw / (1 << PREC); }
} detector_status_t;

class Detector {
public:
    static constexpr int32_t ADC_SIGNED_MAX = (1 << (PREC - 1)) - 1;
    static constexpr int32_t ADC_SIGNED_MIN = -(1 << (PREC - 1));

    static constexpr uint32_t AGC_AMP_MIN = 1 << (PREC - 2);
    static constexpr uint32_t AGC_AMP_MAX = 1000 << PREC;

    static constexpr uint32_t DET_HYST_RATIO = (1 << PREC) * 1 / 20;

private:
    int32_t sine_table[DET_RESO];

    int phase;
    uint32_t adc_level;
    uint32_t threshold;
    uint32_t hysteresis;
    uint8_t anti_chattering_buff;
    uint32_t level_base;
    uint32_t level_peak;
    uint8_t stabled_signal;
    uint32_t t_one_limit_ms;
    uint32_t t_next_agc_upd_ms;

    detector_status_t status;

public:
    void init() {

        phase = 0;

        threshold = (1 << PREC);
        hysteresis = ((1 << PREC) * DET_HYST_RATIO) >> PREC;

        anti_chattering_buff = 0;

        adc_level = 0;
        level_base = 0x7fffffff;
        level_peak = 0;

        stabled_signal = 0;

        status.adc_offset = (1 << PREC) / 2;
        status.agc_amp_raw = (1 << PREC);
        status.adc_level = 0;
        status.raw_signal = 0;
        status.stabled_signal = 0;
        status.rcv_level_base = 0;
        status.rcv_level_peak = 0;
        
        // Setup sine table
        for (int i = 0; i < DET_RESO; i++) {
            sine_table[i] = round(sin(i * 2 * M_PI / DET_RESO) * (1 << (PREC - 1)));
        }

        uint64_t t_now_ms = to_ms_since_boot(get_absolute_time());
        t_next_agc_upd_ms = t_now_ms + 1000;
        t_one_limit_ms = t_now_ms;
    }

    void detect(const uint16_t *dma_buff) {
        uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());

        uint32_t tmp_adc_accum = 0;
        uint32_t tmp_abs_accum = 0;
        uint32_t tmp_det_accum = 0;
        for (int i = 0; i < DET_PERIOD; i++) {
            uint16_t adc_raw = dma_buff[i];
            tmp_adc_accum += adc_raw;

            // Offset
            int32_t adc_signed = (int32_t)adc_raw - (int32_t)status.adc_offset;

            // AGC
            tmp_abs_accum += abs(adc_signed);
            adc_signed = (adc_signed * status.agc_amp_raw) >> PREC;
            adc_signed = clip(ADC_SIGNED_MIN, ADC_SIGNED_MAX, adc_signed);

            // Quad Detection
            int32_t sin = sine_table[phase];
            int32_t cos = sine_table[(phase + DET_RESO / 4) % DET_RESO];
            int32_t u = (sin * adc_signed) >> PREC;
            int32_t v = (cos * adc_signed) >> PREC;
            tmp_det_accum += (uint32_t)fast_sqrt(u * u + v * v);
            phase = (phase + 1) % DET_RESO;
        }

        // Bias
        status.adc_offset = tmp_adc_accum / DET_PERIOD;

        // AGC
        adc_level = max(adc_level, tmp_abs_accum / DET_PERIOD);

        // Quad Detection
        uint32_t det_level = tmp_det_accum / DET_PERIOD;
        level_base = min(level_base, det_level);
        level_peak = max(level_peak, det_level);

        // Hysteresis
        const uint32_t biased_thresh =
            (anti_chattering_buff & 1) ? threshold - hysteresis : threshold + hysteresis;
        status.raw_signal = (det_level >= biased_thresh) ? 1 : 0;

        // Anti-chattering
        anti_chattering_buff = (anti_chattering_buff << 1) & ((1 << ANTI_CHATTERING_DELAY) - 1);
        if (status.raw_signal) anti_chattering_buff |= 1;
        if (anti_chattering_buff == 0) {
            stabled_signal = 0;
        }
        else if (anti_chattering_buff == ((1 << ANTI_CHATTERING_DELAY) - 1)) {
            stabled_signal = 1;
        }

        // Pulse Width Limit
        if (!stabled_signal) {
            status.stabled_signal = 0;
            t_one_limit_ms = t_now_ms + PULSE_WIDTH_LIMIT_MS;
        }
        else if (t_now_ms < t_one_limit_ms) {
            status.stabled_signal = 1;
        }
        else {
            status.stabled_signal = 0;
        }

        status.agc_updated = t_now_ms >= t_next_agc_upd_ms;
        if (status.agc_updated) {
            // Update AGC
            constexpr int GOAL_LEVEL = 1 << (PREC - 3);
            int32_t amp_goal = (GOAL_LEVEL << PREC) / adc_level;
            status.agc_amp_raw +=  (amp_goal - (int32_t)status.agc_amp_raw) / 4;
            status.agc_amp_raw = clip(AGC_AMP_MIN, AGC_AMP_MAX, status.agc_amp_raw);

            // Update Detection Threshold
            threshold = level_base + (level_peak - level_base) / 2;
            hysteresis = ((level_peak - level_base) * DET_HYST_RATIO) >> PREC;

            // Status Latch
            status.rcv_level_base = level_base;
            status.rcv_level_peak = level_peak;
            status.adc_level = adc_level;
            status.quarity_raw = ((level_peak - level_base) << PREC) / level_peak;

            t_next_agc_upd_ms += 1000;
            adc_level = 0;
            level_base = 0x7fffffff;
            level_peak = 0;
        }

    }

    const detector_status_t &read_status() const { return (const detector_status_t &)status; }
};

}

#endif
