#ifndef JJY_RX_RADIO_HPP
#define JJY_RX_RADIO_HPP

#include <stdint.h>
#include <math.h>
#include <string.h>

#include "pico/stdlib.h"

#include "jjy/common.hpp"

#include "lazy_timer.hpp"

namespace jjy::rx {

static constexpr uint32_t DET_RESO = 1 << 3;   
static constexpr uint32_t DET_SPS = DET_RESO * 60000;

typedef struct {
public:
    int32_t agc_gain;

    int32_t adc_amplitude_raw;
    int32_t adc_amplitude_peak;
    int32_t adc_min;
    int32_t adc_max;
    uint8_t hyst_dig_out;
    uint8_t digital_out;
    int32_t quarity;

    int32_t det_anl_out_raw;
    int32_t det_anl_out_norm;

    int32_t det_anl_out_base;
    int32_t det_anl_out_peak;

    void init() {
        agc_gain = 0;
        adc_amplitude_raw = 0;
        adc_amplitude_peak = 0;
        adc_min = 0;
        adc_max = 0;
        hyst_dig_out = 0;
        det_anl_out_raw = 0;
        digital_out = 0;
        quarity = 0;
        det_anl_out_base = 0;
        det_anl_out_peak = 0;
        det_anl_out_norm = 0;
    }
} rf_status_t;

class Agc {
public:
    static constexpr int32_t AMPLITUDE_MAX = (1 << (PREC - 1)) - 1;
    static constexpr int32_t AMPLITUDE_MIN = -(1 << (PREC - 1));
    static constexpr int HISTORY_SIZE = 20;
    static constexpr uint32_t HISTORY_STEP_MS = 1000 / HISTORY_SIZE;
    static constexpr int32_t GAIN_MIN = 1 << (PREC - 2);
    static constexpr int32_t GAIN_MAX = 64 << PREC;
    static constexpr int32_t GOAL_AMPLITUDE = ONE / (1 << 3);

    int32_t offset;
    int32_t adc_amplitude_raw;
    int32_t adc_amplitude_peak;
    int32_t gain;

private:
    LazyTimer<uint32_t> gain_update_timer;
    int32_t amp_history[HISTORY_SIZE];
    int32_t amp_peak;
    int history_index = 0;

public:
    Agc() : gain_update_timer(HISTORY_STEP_MS) { }

    void init() {
        offset = ONE / 2;
        gain = ONE;
        amp_peak = GOAL_AMPLITUDE;
        for (int i = 0; i < HISTORY_SIZE; i++) {
            amp_history[i] = GOAL_AMPLITUDE;
        }
        history_index = 0;
        gain_update_timer.start(to_ms_since_boot(get_absolute_time()));
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
        amp_peak = JJY_MAX(amp_peak, adc_amplitude_raw);
        
        if (gain_update_timer.is_expired(t_now_ms)) {
            update_gain();
        }
    }

    void update_gain() {
        amp_history[history_index] = amp_peak;
        if (history_index < HISTORY_SIZE - 1) {
            history_index++;
        }
        else {
            history_index = 0;
        }
        amp_peak = 0;
        
        adc_amplitude_peak = 0;
        for (int i = 0; i < HISTORY_SIZE; i++) {
            adc_amplitude_peak = JJY_MAX(adc_amplitude_peak, amp_history[i]);
        }

        // AGC ゲイン更新
        int32_t goal_gain = GOAL_AMPLITUDE * ONE / adc_amplitude_peak;
        gain += (goal_gain - gain) / (1 << 4);
        gain = JJY_CLIP(GAIN_MIN, GAIN_MAX, gain);
    }
    
};

class QuadDetector {
public:
    static constexpr int RESO_60KHZ = DET_RESO;
    static constexpr int RESO_40KHZ = DET_RESO * 3 / 2;
    static constexpr int PERIOD = lcm(RESO_60KHZ, RESO_40KHZ);

    freq_t freq = freq_t::EAST_40KHZ;

private:
    int32_t sin_table_40kHz[PERIOD];
    int32_t sin_table_60kHz[PERIOD];
    int phase;

public:
    QuadDetector() {
        // sinテーブル
        for (int i = 0; i < PERIOD; i++) {
            sin_table_40kHz[i] = round(sin(i * 2 * M_PI / RESO_40KHZ) * (1 << (PREC - 1)));
        }
        for (int i = 0; i < PERIOD; i++) {
            sin_table_60kHz[i] = round(sin(i * 2 * M_PI / RESO_60KHZ) * (1 << (PREC - 1)));
        }
    }

    void init(freq_t freq) {
        this->freq = freq;
        phase = 0;
    }

    int32_t process(uint32_t t_now_ms, const int32_t *in, int size) {
        int32_t sum = 0;
        for (int i = 0; i < size; i++) {
            int32_t sin, cos;
            if (freq == freq_t::EAST_40KHZ) {
                sin = sin_table_40kHz[phase];
                cos = sin_table_40kHz[(phase + RESO_40KHZ / 4) % RESO_40KHZ];
            }
            else {
                sin = sin_table_60kHz[phase];
                cos = sin_table_60kHz[(phase + RESO_60KHZ / 4) % RESO_60KHZ];
            }
            int32_t u = (sin * in[i]) >> PREC;
            int32_t v = (cos * in[i]) >> PREC;
            sum += (uint32_t)fast_sqrt(u * u + v * v);
            phase = (phase + 1) % DET_RESO;
        }
        return sum / size;
    }
};

class Binarizer {
public:
    static constexpr int HISTORY_SIZE = 20;
    static constexpr uint32_t HISTORY_STEP_MS = 1000 / HISTORY_SIZE;

    static constexpr int32_t HYSTERESIS_RATIO = ONE / 10;

    static constexpr int ANTI_CHAT_CYCLES = 3; // チャタリング除去の強さ
    static constexpr uint32_t PULSE_WIDTH_LIMIT_MS = 1000; // 最大パルス幅

    int32_t thresh;
    int32_t hysteresis;
    uint8_t anti_chat_sreg;

    uint8_t anti_chat_out;
    uint8_t pulse_width_limited_out;
    uint8_t hyst_out;

    int32_t det_base;
    int32_t det_peak;
    
private:
    int32_t accum_base;
    int32_t accum_peak;
    int32_t base_history[HISTORY_SIZE];
    int32_t peak_history[HISTORY_SIZE];
    int history_index = 0;

    LazyTimer<uint32_t> thresh_update_timer;
    LazyTimer<uint32_t, false> pulse_width_limit_timer;

public:

    Binarizer() : thresh_update_timer(HISTORY_STEP_MS), pulse_width_limit_timer(PULSE_WIDTH_LIMIT_MS) { }

    void init(void) {
        for (int i = 0; i < HISTORY_SIZE; i++) {
            base_history[i] = ONE / 2;
            peak_history[i] = ONE / 2;
        }
        history_index = 0;

        hysteresis = thresh * HYSTERESIS_RATIO / ONE;
        thresh = ONE / 2;
        hyst_out = 0;

        accum_base = ONE / 2;
        accum_peak = ONE / 2;

        anti_chat_sreg = 0;
        anti_chat_out = 0;
        pulse_width_limited_out = 0;

        uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());
        thresh_update_timer.start(t_now_ms);
        pulse_width_limit_timer.start(t_now_ms, PULSE_WIDTH_LIMIT_MS);
    }

    uint8_t process(uint32_t t_now_ms, int32_t in) {
        accum_base = JJY_MIN(accum_base, in);
        accum_peak = JJY_MAX(accum_peak, in);
        
        // ヒステリシス
        int32_t biased_thresh = (anti_chat_sreg & 1) ? thresh - hysteresis : thresh + hysteresis;
        hyst_out = (in >= biased_thresh) ? 1 : 0;

        // チャタリング除去
        anti_chat_sreg = (anti_chat_sreg << 1) & ((1 << ANTI_CHAT_CYCLES) - 1);
        anti_chat_sreg |= hyst_out;
        if (anti_chat_sreg == 0) {
            anti_chat_out = 0;
        }
        else if (anti_chat_sreg == ((1 << ANTI_CHAT_CYCLES) - 1)) {
            anti_chat_out = 1;
        }

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

        return pulse_width_limited_out;
    }

    void update_thresh() {
        base_history[history_index] = accum_base;
        peak_history[history_index] = accum_peak;
        if (history_index < HISTORY_SIZE - 1) {
            history_index++;
        }
        else {
            history_index = 0;
        }
        accum_base = 0x7fffffff;
        accum_peak = 0;

        // レベルのピーク値を取得
        det_base = 0x7fffffff;
        det_peak = 0;
        for (int i = 0; i < HISTORY_SIZE; i++) {
            det_base = JJY_MIN(det_base, base_history[i]);
            det_peak = JJY_MAX(det_peak, peak_history[i]);
        }

        // スレッショルド更新
        thresh = det_base + (det_peak - det_base) / 2;
        hysteresis = ((det_peak - det_base) * HYSTERESIS_RATIO) >> PREC;

    }
};

class Rf {
public:
    Agc agc;
    QuadDetector det;
    Binarizer bin;

private:
    rf_status_t status;
    int32_t *agc_out = nullptr;
    int agc_out_size = 0;

public:
    void init(freq_t freq) {
        agc.init();
        det.init(freq);
        bin.init();
        status.init();
    }

    uint8_t detect(const uint32_t t_now_ms, const uint16_t *samples, const uint32_t size) {
        if (!agc_out || agc_out_size < size) {
            if (agc_out) delete[] agc_out;
            agc_out = new int32_t[size];
        }

        // AGC
        agc.process(t_now_ms, samples, agc_out, size);

        // 直交検波
        int32_t det_anl_out_raw = det.process(t_now_ms, agc_out, size);

        // フィルター
        uint8_t out = bin.process(t_now_ms, det_anl_out_raw);

        // ステータス値更新
        status.adc_amplitude_raw = agc.adc_amplitude_raw;
        status.adc_amplitude_peak = agc.adc_amplitude_peak;
        status.agc_gain = agc.gain;
        status.det_anl_out_raw = det_anl_out_raw;
        status.det_anl_out_base = bin.det_base;
        status.det_anl_out_peak = bin.det_peak;
        status.det_anl_out_norm = (det_anl_out_raw - bin.det_base) * ONE / (bin.det_peak - bin.det_base);
        status.hyst_dig_out = bin.hyst_out;
        status.quarity = ((bin.det_peak - bin.det_base) * ONE) / bin.det_peak;
        status.digital_out = out;

        return out;
    }

    /// @brief 検波器の状態を取得
    const rf_status_t &get_status() const {
        return (const rf_status_t &)status;
    }
};

}

#endif
