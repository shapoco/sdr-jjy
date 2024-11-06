#ifndef JJY_RX_RADIO_HPP
#define JJY_RX_RADIO_HPP

#include <stdint.h>
#include <math.h>
#include <string.h>

#include "pico/stdlib.h"

#include "shapoco/ring_scope.hpp"
#include "shapoco/peak_hold.hpp"

#include "jjy/common.hpp"

#include "lazy_timer.hpp"
#include "ring_stat.hpp"

using namespace shapoco; // todo: 削除

namespace jjy::rx {

static constexpr uint32_t DET_RESO = 1 << 3;   
static constexpr uint32_t DET_SPS = DET_RESO * 60000;

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

class Agc {
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
    RingStat<int32_t, AMP_HISTORY_SIZE> amp_history;
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

    void init(freq_t freq, uint32_t t_now_ms) {
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
    RingStat<int32_t, PEAK_HISTORY_SIZE> base_history;
    RingStat<int32_t, PEAK_HISTORY_SIZE> peak_history;

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
    RingStat<int32_t, QUARITY_HISTORY_SIZE> quality_history;

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
        int32_t qty = JJY_CLIP(0, jjy::ONE, JJY_ABS(in - thresh) * jjy::ONE / (range / 2));
        accum_quality_value += qty;
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
        if (base_history.size() > 0) {
            det_base = base_history.min();
            det_peak = peak_history.max();
        }

        // スレッショルド更新
        thresh = (det_base + det_peak) / 2;
        hysteresis = (det_peak - det_base) * HYSTERESIS_RATIO / ONE;
    }
};

class Rf {
public:
    const int dma_size;
    const int det_delay_ms;
    const int anti_chat_delay_ms;
    Agc agc;
    QuadDetector det;
    Binarizer bin;

private:
    rf_status_t status;
    int32_t * const agc_out;
    int agc_out_size = 0;

public:
    Rf(uint32_t dma_size) :
        dma_size(dma_size),
        det_delay_ms(1000 * dma_size / DET_SPS), 
        anti_chat_delay_ms(det_delay_ms * (AntiChattering::ANTI_CHAT_CYCLES - 1)),
        agc_out(new int32_t[dma_size]) {}
    
    ~Rf() { delete[] agc_out; }

    void init(freq_t freq, const uint32_t t_now_ms) {
        agc.init(t_now_ms);
        det.init(freq, t_now_ms);
        bin.init(t_now_ms);
        status.init(t_now_ms, det_delay_ms, anti_chat_delay_ms);
    }

    uint8_t process(const uint32_t t_now_ms, const uint16_t *samples) {
        // AGC
        agc.process(t_now_ms, samples, agc_out, dma_size);

        // 直交検波
        int32_t det_anl_out_raw = det.process(t_now_ms, agc_out, dma_size);

        // フィルター
        uint8_t out = bin.process(t_now_ms, det_anl_out_raw);

        // ステータス値更新
        status.timestamp_ms = t_now_ms;
        status.adc_amplitude_raw = agc.adc_amplitude_raw;
        status.adc_amplitude_peak = agc.adc_amplitude_peak;
        status.agc_gain = agc.gain;
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

    const rf_status_t &get_status() const {
        return (const rf_status_t &)status;
    }
};

}

#endif
