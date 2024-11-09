#pragma once

#include <stdint.h>
#include <string.h>

#include "shapoco/ring_scope.hpp"
#include "shapoco/peak_hold.hpp"
#include "shapoco/ring_history.hpp"

#include "shapoco/jjy/common.hpp"
#include "shapoco/jjy/dc_bias.hpp"
#include "shapoco/jjy/agc.hpp"
#include "shapoco/jjy/anti_chattering.hpp"

#include "shapoco/jjy/rx/common.hpp"
#include "shapoco/jjy/rx/diff_detector.hpp"

#include "shapoco/lazy_timer.hpp"

#define USE_NEW_AGC // todo: 削除

namespace shapoco::jjy::rx {

typedef struct {
public:
    uint32_t timestamp_ms;
    uint32_t det_delay_ms;
    uint32_t anti_chat_delay_ms;

    int32_t agc_gain;

    int32_t adc_amplitude_raw;
    int32_t adc_amplitude_peak;
    int32_t adc_min;
    int32_t adc_max;
    uint8_t hyst_dig_out;
    uint8_t digital_out;
    int32_t signal_quarity;

    int32_t det_anl_out_raw;
    int32_t det_anl_out_norm;

    bool beat_detected;
    int32_t det_anl_out_beat_det;

    int32_t det_anl_out_base;
    int32_t det_anl_out_peak;

    void init(uint32_t t_now_ms, uint32_t det_delay_ms, uint32_t anti_chat_delay_ms) {
        timestamp_ms = t_now_ms;
        this->det_delay_ms = det_delay_ms;
        this->anti_chat_delay_ms = anti_chat_delay_ms;
        agc_gain = 0;
        adc_amplitude_raw = 0;
        adc_amplitude_peak = 0;
        adc_min = 0;
        adc_max = 0;
        hyst_dig_out = 0;
        det_anl_out_raw = 0;
        digital_out = 0;
        signal_quarity = 0;
        det_anl_out_base = 0;
        det_anl_out_peak = 0;
        det_anl_out_norm = 0;

        beat_detected = false;
        det_anl_out_beat_det = 0;
    }
} rf_status_t;

#ifndef USE_NEW_AGC
class Agc_Deprecated {
public:
    static constexpr int32_t AMPLITUDE_MAX = (1 << (PREC - 1)) - 1;
    static constexpr int32_t AMPLITUDE_MIN = -(1 << (PREC - 1));
    static constexpr uint32_t AMP_HISTORY_STEP_MS = 50;
    static constexpr int AMP_HISTORY_SIZE = (1000 + AMP_HISTORY_STEP_MS - 1) / AMP_HISTORY_STEP_MS;
    static constexpr int32_t GAIN_MIN = 1 << (PREC - 2);
    static constexpr int32_t GAIN_MAX = 64 << PREC;
    static constexpr int32_t GOAL_AMPLITUDE = ONE / (1 << 3);

    int32_t offset;
    int32_t adc_amplitude_raw;
    int32_t adc_amplitude_peak;
    int32_t gain;

private:
    LazyTimer<uint32_t, AMP_HISTORY_STEP_MS> gain_update_timer;
    RingHistory<int32_t, AMP_HISTORY_SIZE> amp_history;
    int32_t accum_amp_peak;

public:
    void init(uint32_t t_now_ms) {
        offset = ONE / 2;
        gain = ONE;
        accum_amp_peak = GOAL_AMPLITUDE;
        amp_history.clear(GOAL_AMPLITUDE);
        gain_update_timer.start(t_now_ms);
        adc_amplitude_peak = GOAL_AMPLITUDE;
        adc_amplitude_raw = GOAL_AMPLITUDE;
    }

    void process(const uint32_t t_now_ms, const uint16_t *in_buff, int32_t *out_buff, const uint32_t size) {
        int32_t in_sum = 0;
        int32_t amp_sum = 0;
        for (int i = 0; i < size; i++) {
            int32_t in = in_buff[i];
            in_sum += in;

            // オフセットの減算
            int32_t out = in - offset;

            // AGC
            amp_sum += JJY_ABS(out);
            out_buff[i] = JJY_CLIP(AMPLITUDE_MIN, AMPLITUDE_MAX, out * gain / ONE);
        }

        // オフセットの再計算
        offset = in_sum / size;

        // ADC 振幅計算
        adc_amplitude_raw = amp_sum / size;
        accum_amp_peak = JJY_MAX(accum_amp_peak, adc_amplitude_raw);
        
        if (gain_update_timer.is_expired(t_now_ms)) {
            update_gain();
        }
    }

    void update_gain() {
        amp_history.push(accum_amp_peak);
        adc_amplitude_peak = amp_history.max();
        accum_amp_peak = 0;

        // AGC ゲイン更新
        int32_t goal_gain = GOAL_AMPLITUDE * ONE / adc_amplitude_peak;
        gain += (goal_gain - gain) / (1 << 4);
        gain = JJY_CLIP(GAIN_MIN, GAIN_MAX, gain);
    }
    
};
#endif

class AntiChattering {
public:
    static constexpr int ANTI_CHAT_CYCLES = 3; // チャタリング除去の強さ
    uint8_t anti_chat_sreg;
    uint8_t anti_chat_out;
    uint8_t hyst_out;

private:

public:
    void init(uint32_t t_now_ms) {
        hyst_out = 0;
        anti_chat_sreg = 0;
        anti_chat_out = 0;
    }
    
    uint8_t process(uint32_t t_now_ms, int32_t thresh, int32_t hysteresis, int32_t in) {
        // ヒステリシス
        int32_t biased_thresh = (anti_chat_sreg & 1) ? thresh - hysteresis : thresh + hysteresis;
        hyst_out = (in > biased_thresh) ? 1 : 0;

        // チャタリング除去
        anti_chat_sreg = (anti_chat_sreg << 1) & ((1 << ANTI_CHAT_CYCLES) - 1);
        anti_chat_sreg |= hyst_out;
        if (anti_chat_sreg == 0) {
            anti_chat_out = 0;
        }
        else if (anti_chat_sreg == ((1 << ANTI_CHAT_CYCLES) - 1)) {
            anti_chat_out = 1;
        }
        return anti_chat_out;
    }
};

class Binarizer {
public:
    static constexpr uint32_t PEAK_HISTORY_STEP_MS = 100;
    static constexpr int PEAK_HISTORY_SIZE = (1000 + PEAK_HISTORY_STEP_MS - 1) / PEAK_HISTORY_STEP_MS;

    static constexpr int32_t HYSTERESIS_RATIO = ONE / 10;

    static constexpr uint32_t BEAT_DET_AMP_SCOPE_PERIOD_MS = 100;
    static constexpr uint32_t BEAT_DET_AMP_SCOPE_RESO_MS = 20;
    static constexpr uint32_t BEAT_DET_PERIOD_MS = 3000;
    static constexpr int BEAT_DET_EDGE_THRESH = 20;

    static constexpr uint32_t PULSE_WIDTH_LIMIT_MS = 1000; // 最大パルス幅

    static constexpr int QUARITY_HISTORY_SIZE = 100;
    static constexpr int QUARITY_HISTORY_STEP_MS = 1000 / QUARITY_HISTORY_SIZE;

    int32_t hysteresis;
    int32_t thresh;
    
    uint8_t beat_det_last_in_dig;
    int beat_det_edge_count;
    int32_t beat_amp;
    int32_t beat_amp_hold;
    bool beat_detected;
    int32_t beat_det_out;

    uint8_t hyst_out;
    uint8_t pulse_width_limited_out;

    int32_t det_base;
    int32_t det_peak;

    int32_t quarity;
    
private:
    int32_t accum_base;
    int32_t accum_peak;
    RingHistory<int32_t, PEAK_HISTORY_SIZE> base_history;
    RingHistory<int32_t, PEAK_HISTORY_SIZE> peak_history;

    AntiChattering anti_chat_before_smooth;
    AntiChattering anti_chat_after_smooth;

    LazyTimer<uint32_t, PEAK_HISTORY_STEP_MS> thresh_update_timer;
    RingScope<int32_t, uint32_t, ONE / 2, BEAT_DET_AMP_SCOPE_PERIOD_MS, BEAT_DET_AMP_SCOPE_RESO_MS> beat_amp_scope;
    PeakHold<int32_t> beat_amp_peak_hold;
    LazyTimer<uint32_t, BEAT_DET_PERIOD_MS> beat_det_timer;
    LazyTimer<uint32_t, PULSE_WIDTH_LIMIT_MS, false> pulse_width_limit_timer;

    int32_t accum_quality_value;
    int accum_quality_count;
    LazyTimer<uint32_t, QUARITY_HISTORY_STEP_MS> quality_history_update_timer;
    RingHistory<int32_t, QUARITY_HISTORY_SIZE> quality_history;

public:
    void init(uint32_t t_now_ms) {
        base_history.clear(ONE / 2);
        peak_history.clear(ONE / 2);

        thresh = ONE / 2;
        hysteresis = (ONE / 4) * HYSTERESIS_RATIO / ONE;

        accum_base = 0x7fffffff;
        accum_peak = 0;

        beat_det_last_in_dig = 0;
        beat_det_edge_count = 0;
        beat_amp_scope.clear(t_now_ms);
        beat_amp = 0;
        beat_amp_peak_hold.clear();
        beat_amp_hold = 0;
        beat_detected = false;
        beat_det_out = 0;

        pulse_width_limited_out = 0;

        thresh_update_timer.start(t_now_ms);
        beat_det_timer.start(t_now_ms);
        pulse_width_limit_timer.set_expired();

        accum_quality_value = 0;
        accum_quality_count = 0;
        quality_history_update_timer.start(t_now_ms);
        quality_history.clear(0);
        quarity = 0;
    }

    uint8_t process(uint32_t t_now_ms, int32_t in) {
        accum_base = JJY_MIN(accum_base, in);
        accum_peak = JJY_MAX(accum_peak, in);

        // うなりの振幅検出
        beat_amp_scope.write(t_now_ms, in);
        beat_amp = beat_amp_scope.total_amplitude();
        beat_amp_peak_hold.enter(beat_amp);
        
        // うなりのエッジ検出
        anti_chat_before_smooth.process(t_now_ms, thresh, hysteresis, in);
        hyst_out = anti_chat_before_smooth.hyst_out;
        bool edge = hyst_out != beat_det_last_in_dig;
        beat_det_last_in_dig = hyst_out;
        if (edge) beat_det_edge_count += 1;

        // うなり検出判定
        if (beat_det_timer.is_expired(t_now_ms)) {
            beat_detected = beat_det_edge_count >= BEAT_DET_EDGE_THRESH;
            beat_det_edge_count = 0;
            beat_amp_hold = beat_amp_peak_hold.amplitude();
            beat_amp_peak_hold.clear();
        }
        
        // うなりを検出した場合はうなりの振幅を検波後の値として使用する
        if (beat_detected) {
            in = det_base + beat_amp * (det_peak - det_base) / beat_amp_hold;
            //in = (det_peak + det_base) / 2 + (beat_amp - beat_amp_hold / 2) * 3 / 2;
        }
        beat_det_out = in;

        // チャタリング除去
        uint8_t anti_chat_out = anti_chat_after_smooth.process(t_now_ms, thresh, hysteresis, in);

        // パルス幅制限
        if (!anti_chat_out) {
            pulse_width_limited_out = 0;
            pulse_width_limit_timer.start(t_now_ms);
        }
        else if (pulse_width_limit_timer.is_expired(t_now_ms)) {
            pulse_width_limited_out = 0;
        }
        else {
            pulse_width_limited_out = 1;
        }

        // スレッショルド更新
        if (thresh_update_timer.is_expired(t_now_ms)) {
            update_thresh();
        }

        // 信号品質測定
        int32_t range = det_peak - det_base;
        int32_t thresh = (det_peak + det_base) / 2;
        int32_t qtyA = JJY_CLIP(0, jjy::ONE, JJY_ABS(in - thresh) * jjy::ONE / (range / 2));
        int32_t qtyB = JJY_CLIP(0, jjy::ONE, range * ONE / det_peak);
        accum_quality_value += qtyA * qtyB / ONE;
        accum_quality_count += 1;
        if (quality_history_update_timer.is_expired(t_now_ms)) {
            quality_history.push(accum_quality_value / accum_quality_count);
            quarity = quality_history.ave();
            accum_quality_value = 0;
            accum_quality_count = 0;
        }

        return pulse_width_limited_out;
    }

    void update_thresh() {
        base_history.push(accum_base);
        peak_history.push(accum_peak);
        accum_base = 0x7fffffff;
        accum_peak = 0;

        // レベルのピーク値を取得
        det_base = base_history.min();
        det_peak = peak_history.max();

        // スレッショルド更新
        thresh = (det_base + det_peak) / 2;
        hysteresis = (det_peak - det_base) * HYSTERESIS_RATIO / ONE;
    }
};

class Rf {
public:
    const int det_delay_ms;
    const int anti_chat_delay_ms;
#ifdef USE_NEW_AGC
    DcBias<DETECTION_OUTPUT_SPS, ONE / 2> pre_bias;
    Agc<DETECTION_OUTPUT_SPS, ONE / 2, ONE / 10, ONE * 100> pre_agc;
#else
    Agc_Deprecated agc;
#endif
    DifferentialDetector det;
    Binarizer bin;

private:
    rf_status_t status;
    int32_t agc_out[DETECTION_BLOCK_SIZE];
    int agc_out_size = 0;

public:
    Rf() :
        det_delay_ms(1000 * DETECTION_BLOCK_SIZE / DETECTION_INPUT_SPS), 
        anti_chat_delay_ms(det_delay_ms * (AntiChattering::ANTI_CHAT_CYCLES - 1)) {}
    
    void init(freq_t freq, const uint32_t t_now_ms) {
#ifdef USE_NEW_AGC
        pre_bias.reset();
        pre_agc.reset();
#else
        agc.init(t_now_ms);
#endif
        det.init(freq, t_now_ms);
        bin.init(t_now_ms);
        status.init(t_now_ms, det_delay_ms, anti_chat_delay_ms);
    }

    uint8_t process(const uint32_t t_now_ms, const uint16_t *in) {
#ifdef USE_NEW_AGC
        preBiasAgc(t_now_ms, in);
#else
        // AGC
        agc.process(t_now_ms, in, agc_out, DETECTION_BLOCK_SIZE);
#endif

        // 検波器
        int32_t det_anl_out_raw = det.process(t_now_ms, agc_out);

        // フィルター
        uint8_t out = bin.process(t_now_ms, det_anl_out_raw);

        // ステータス値更新
        status.timestamp_ms = t_now_ms;
#ifndef USE_NEW_AGC
        status.adc_amplitude_raw = agc.adc_amplitude_raw;
        status.adc_amplitude_peak = agc.adc_amplitude_peak;
        status.agc_gain = agc.gain;
#endif
        status.det_anl_out_raw = det_anl_out_raw;
        status.det_anl_out_base = bin.det_base;
        status.det_anl_out_peak = bin.det_peak;
        status.det_anl_out_norm = (det_anl_out_raw - bin.det_base) * ONE / (bin.det_peak - bin.det_base);
        status.det_anl_out_beat_det = (bin.beat_det_out - bin.det_base) * ONE / (bin.det_peak - bin.det_base);
        status.beat_detected = bin.beat_detected;
        status.hyst_dig_out = bin.hyst_out;
        status.signal_quarity = bin.quarity;
        status.digital_out = out;

        return out;
    }

#ifdef USE_NEW_AGC
    void preBiasAgc(uint64_t t_ms, const uint16_t *in) {
        constexpr int32_t HALF_PI = 3.1415926535f * ONE / 2;
        constexpr int BIAS_PREC = 4;

        int32_t ave = 0;
        int32_t amp = 0;
        int32_t bias = JJY_ROUND_DIV(pre_bias.curr_bias, (1 << BIAS_PREC));
        int32_t gain = pre_agc.curr_gain;
        for (int i = 0; i < DETECTION_BLOCK_SIZE; i++) {
            // DC オフセット
            ave += in[i];
            int32_t biased = in[i] - bias;
            
            // AGC
            amp += JJY_ABS(biased);
            int32_t gained = biased * gain / ONE;

            agc_out[i] = gained;
        }
        ave = JJY_ROUND_DIV(ave * (1 << BIAS_PREC), DETECTION_BLOCK_SIZE);
        amp = JJY_ROUND_DIV(amp, DETECTION_BLOCK_SIZE);
        amp = JJY_ROUND_DIV(amp * HALF_PI, ONE);

        pre_bias.process(ave);
        pre_agc.process(amp);
        
        status.adc_amplitude_raw = amp;
        status.adc_amplitude_peak = pre_agc.amplitude_peak;
        status.agc_gain = pre_agc.curr_gain;
    }
#endif

    const rf_status_t &get_status() const {
        return (const rf_status_t &)status;
    }
};

}
