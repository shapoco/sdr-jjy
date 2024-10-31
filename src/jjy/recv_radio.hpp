#ifndef JJY_RF_RECEIVER_HPP
#define JJY_RF_RECEIVER_HPP

#include <stdint.h>
#include <math.h>
#include <string.h>

#include "pico/stdlib.h"

#include "jjy/common.hpp"

namespace jjy {

static constexpr uint32_t DMA_SIZE = 1000; // ADC DMA サイズ
static constexpr int DET_RESO = 1 << 3; // 直交検波の解像度

typedef struct {
public:
    uint32_t adc_offset;
    uint32_t agc_amp_raw;
    uint32_t adc_level;
    int32_t adc_min;
    int32_t adc_max;
    uint32_t det_level_raw;
    uint8_t raw_signal;
    uint8_t stable_signal;
    uint32_t quarity_raw;

    uint32_t det_threshold;
    uint32_t det_signal_base;
    uint32_t det_signal_peak;

    const float det_level() const { return (float)det_level_raw / (1 << PREC); }
    const float signal_level() const { return (float)(det_level_raw - det_signal_base) / (det_signal_peak - det_signal_base); }
    const float agc_amp() const { return (float)agc_amp_raw / (1 << PREC); }
    const float quarity() const { return (float)quarity_raw / (1 << PREC); }
} rf_status_t;

class Detector {
public:
    static constexpr int32_t ADC_SIGNED_MAX = (1 << (PREC - 1)) - 1;
    static constexpr int32_t ADC_SIGNED_MIN = -(1 << (PREC - 1));

    static constexpr uint32_t AGC_HIST_DEPTH = 20;
    static constexpr uint32_t AGC_HIST_STEP_MS = 1000 / AGC_HIST_DEPTH;

    static constexpr uint32_t AGC_AMP_MIN = 1 << (PREC - 2);
    static constexpr uint32_t AGC_AMP_MAX = 1000 << PREC;

    static constexpr uint32_t DET_HYST_RATIO = (1 << PREC) * 1 / 20;

    static constexpr int ANTI_CHAT_CYCLES = 3; // チャタリング除去の強さ
    static constexpr uint32_t PULSE_WIDTH_LIMIT_MS = 1000; // 最大パルス幅
private:
    int32_t sin_table[DET_RESO];

    int phase;
    uint32_t accum_adc_level;
    uint32_t accum_det_base;
    uint32_t accum_det_peak;

    int history_index = 0;
    uint32_t history_adc_level[AGC_HIST_DEPTH];
    uint32_t history_det_base[AGC_HIST_DEPTH];
    uint32_t history_det_peak[AGC_HIST_DEPTH];

    uint32_t threshold;
    uint32_t hysteresis;
    uint8_t anti_chat_sreg;
    uint8_t stable_signal;

    uint32_t t_one_limit_ms;
    uint32_t t_next_agc_upd_ms;

    rf_status_t status;

public:
    Detector() {
        // sinテーブル
        for (int i = 0; i < DET_RESO; i++) {
            sin_table[i] = round(sin(i * 2 * M_PI / DET_RESO) * (1 << (PREC - 1)));
        }
    }

    void init() {
        phase = 0;
        history_index = 0;
        memset(history_adc_level, 0, sizeof(uint32_t) * AGC_HIST_DEPTH);
        memset(history_det_base, 0, sizeof(uint32_t) * AGC_HIST_DEPTH);
        memset(history_det_peak, 0, sizeof(uint32_t) * AGC_HIST_DEPTH);
        accum_adc_level = 0;
        accum_det_base = 0x7fffffff;
        accum_det_peak = 0;
        threshold = 1 << (PREC - 1);
        hysteresis = (threshold * DET_HYST_RATIO) >> PREC;
        anti_chat_sreg = 0;
        stable_signal = 0;

        status.adc_offset = (1 << PREC) / 2;
        status.agc_amp_raw = 0;
        status.adc_level = 0;
        status.det_level_raw = 0;
        status.raw_signal = 0;
        status.stable_signal = 0;
        status.det_signal_base = 0;
        status.det_signal_peak = 0;
        
        uint64_t t_now_ms = to_ms_since_boot(get_absolute_time());
        t_next_agc_upd_ms = t_now_ms + 1000;
        t_one_limit_ms = t_now_ms;
    }

    void detect(uint32_t t_now_ms, const uint16_t *dma_buff) {
        uint32_t tmp_adc_accum = 0;
        uint32_t tmp_abs_accum = 0;
        uint32_t tmp_det_accum = 0;
        int32_t tmp_adc_min = (1 << (PREC * 2));
        int32_t tmp_adc_max = -(1 << (PREC * 2));
        for (int i = 0; i < DMA_SIZE; i++) {
            uint16_t adc_raw = dma_buff[i];
            tmp_adc_accum += adc_raw;

            // オフセットの減算
            int32_t adc_signed = (int32_t)adc_raw - (int32_t)status.adc_offset;
            tmp_adc_min = JJY_MIN(tmp_adc_min, adc_signed);
            tmp_adc_max = JJY_MAX(tmp_adc_max, adc_signed);

            // AGC
            tmp_abs_accum += JJY_ABS(adc_signed);
            adc_signed = (adc_signed * (int32_t)status.agc_amp_raw) / (1 << PREC);
            adc_signed = JJY_CLIP(ADC_SIGNED_MIN, ADC_SIGNED_MAX, adc_signed);

            // 直交検波
            int32_t sin = sin_table[phase];
            int32_t cos = sin_table[(phase + DET_RESO / 4) % DET_RESO];
            int32_t u = (sin * adc_signed) >> PREC;
            int32_t v = (cos * adc_signed) >> PREC;
            tmp_det_accum += (uint32_t)fast_sqrt(u * u + v * v);
            phase = (phase + 1) % DET_RESO;
        }

        // 振幅
        status.adc_min = tmp_adc_min;
        status.adc_max = tmp_adc_max;

        // オフセットの再計算
        status.adc_offset = tmp_adc_accum / DMA_SIZE;

        // ADC 振幅計算
        accum_adc_level = JJY_MAX(accum_adc_level, tmp_abs_accum / DMA_SIZE);

        // 直交検波後の信号レベル
        status.det_level_raw = tmp_det_accum / DMA_SIZE;
        accum_det_base = JJY_MIN(accum_det_base, status.det_level_raw);
        accum_det_peak = JJY_MAX(accum_det_peak, status.det_level_raw);

        // ヒステリシス
        const uint32_t biased_thresh =
            (anti_chat_sreg & 1) ? threshold - hysteresis : threshold + hysteresis;
        status.raw_signal = (status.det_level_raw >= biased_thresh) ? 1 : 0;

        // チャタリング除去
        anti_chat_sreg = (anti_chat_sreg << 1) & ((1 << ANTI_CHAT_CYCLES) - 1);
        if (status.raw_signal) anti_chat_sreg |= 1;
        if (anti_chat_sreg == 0) {
            stable_signal = 0;
        }
        else if (anti_chat_sreg == ((1 << ANTI_CHAT_CYCLES) - 1)) {
            stable_signal = 1;
        }

        // パルス幅制限
        if (!stable_signal) {
            status.stable_signal = 0;
            t_one_limit_ms = t_now_ms + PULSE_WIDTH_LIMIT_MS;
        }
        else if (t_now_ms < t_one_limit_ms) {
            status.stable_signal = 1;
        }
        else {
            status.stable_signal = 0;
        }

        if (t_now_ms >= t_next_agc_upd_ms) {
            t_next_agc_upd_ms += AGC_HIST_STEP_MS;

            // AGC用信号レベル履歴の更新
            history_adc_level[history_index] = accum_adc_level;
            history_det_base[history_index] = accum_det_base;
            history_det_peak[history_index] = accum_det_peak;
            if (history_index < AGC_HIST_DEPTH - 1) {
                history_index++;
            }
            else {
                history_index = 0;
            }
            accum_adc_level = 0;
            accum_det_base = 0x7fffffff;
            accum_det_peak = 0;

            // レベルのピーク値を取得
            uint32_t adc_level = 0;
            uint32_t det_base = 0x7fffffff;
            uint32_t det_peak = 0;
            for (int i = 0; i < AGC_HIST_DEPTH; i++) {
                adc_level = JJY_MAX(adc_level, history_adc_level[i]);
                det_base = JJY_MIN(det_base, history_det_base[i]);
                det_peak = JJY_MAX(det_peak, history_det_peak[i]);
            }

            // AGC ゲイン更新
            constexpr int GOAL_LEVEL = 1 << (PREC - 3);
            int32_t amp_goal = (GOAL_LEVEL << PREC) / adc_level;
            status.agc_amp_raw += (amp_goal - (int32_t)status.agc_amp_raw) / (1 << 4);
            status.agc_amp_raw = JJY_CLIP(AGC_AMP_MIN, AGC_AMP_MAX, status.agc_amp_raw);

            // スレッショルド更新
            threshold = det_base + (det_peak - det_base) / 2;
            hysteresis = ((det_peak - det_base) * DET_HYST_RATIO) >> PREC;

            // ステータス値更新
            status.adc_level = adc_level;
            status.det_threshold = threshold;
            status.det_signal_base = det_base;
            status.det_signal_peak = det_peak;
            status.quarity_raw = ((det_peak - det_base) << PREC) / det_peak;
        }

    }

    /// @brief 検波器の状態を取得
    const rf_status_t &get_status() const { return (const rf_status_t &)status; }
};

}

#endif
