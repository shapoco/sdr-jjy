#ifndef JJY_RX_SYNCHRONIZER
#define JJY_RX_SYNCHRONIZER

#include <stdint.h>
#include <math.h>

#include "pico/stdlib.h"

#include "jjy/common.hpp"

namespace jjy::rx {

static constexpr int NUM_PHASE_CANDS = 10;

typedef struct {
    int32_t phase;
    int score;
    bool valid;
} phase_cand_t;

typedef struct {
    phase_cand_t phase_cands[NUM_PHASE_CANDS];
    int32_t phase;
    bool phase_locked;
    int32_t phase_lock_progress;
    int32_t bit_det_quality;
    int32_t bit_det_progress;
    bool bit_det_ok;
} sync_status_t;

class Synchronizer {
private:
    static constexpr int LOCK_WAIT_TIME_MS = 1024 * 5;
    static constexpr int SCORE_MAX = 10;
    static constexpr int BITDET_NUM_SLOTS = 10;
    static constexpr int BITDET_OK_THRESH = 5;

public:
    sync_status_t status;

    uint32_t t_last_ms = 0;
    uint8_t last_in = 0;

    uint32_t phase_t_next_cand_upd;
    uint32_t phase_t_last_unlock;
    uint32_t phase_t_expire;
    int phase_last_cand_index = -1;

    int32_t bitdet_last_slot;
    int bitdet_hi_count = 0;
    int bitdet_lo_count = 0;
    uint32_t bitdet_sreg = 0;
    int bitdet_ok_count = 0;
    int32_t bitdet_quality_accum = 0;

    void init() {
        uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());;

        t_last_ms = t_now_ms;
        last_in = 0;

        phase_t_next_cand_upd = t_now_ms;
        phase_t_last_unlock = t_now_ms;
        phase_t_expire = 0;
        phase_last_cand_index = -1;

        bitdet_last_slot = 0;
        bitdet_hi_count = 0;
        bitdet_lo_count = 0;
        bitdet_sreg = 0;
        bitdet_ok_count = 0;
        bitdet_quality_accum = 0;

        for (int i = 0; i < NUM_PHASE_CANDS; i++) {
            status.phase_cands[i].valid = false;
            status.phase_cands[i].phase = 0;
        }
        status.phase = 0;
        status.phase_locked = false;
        status.phase_lock_progress = 0;
        status.bit_det_quality = 0;
        status.bit_det_progress = 0;
        status.bit_det_ok = false;
    }

    bool synchronize(uint32_t t_now_ms, uint8_t in, jjybit_t *out) {
        uint32_t t_delta = t_now_ms - t_last_ms;
        t_last_ms = t_now_ms;

        int32_t phase_ms = t_now_ms % 1000;
        int32_t phase = phase_ms * PHASE_PERIOD / 1000;

        bool cand_upd = t_now_ms > phase_t_next_cand_upd;
        if (cand_upd) {
            phase_t_next_cand_upd = JJY_MAX(t_now_ms + 500, phase_t_next_cand_upd + 1000);
        }

        // エッジの減衰
        if (cand_upd) {
            int num_valid_cand = 0;
            for (int i = 0; i < NUM_PHASE_CANDS; i++) {
                phase_cand_t &cand = status.phase_cands[i];
                if (!cand.valid) continue;
                cand.valid = (--cand.score > 0);
                if (cand.valid) num_valid_cand += 1;
            }
            if (num_valid_cand == 0) {
                phase_t_expire = 0;
            }
        }

        // エッジ検出
        bool rise = !!in && !last_in;
        last_in = in;
        if (rise) add_edge(phase);

        if (cand_upd || rise) {
            // 候補から位相を選ぶ
            int best_index = -1;
            int best_score = -1;
            int32_t best_phase = 0;
            for (int i = 0; i < NUM_PHASE_CANDS; i++) {
                phase_cand_t &cand = status.phase_cands[i];
                if (!cand.valid) continue;
                if (cand.score >= best_score) {
                    best_index = i;
                    best_score = cand.score;
                    best_phase = cand.phase;
                }
            }
            if (best_index < 0) {
                phase_t_expire = 0;
            }
            else if (phase_last_cand_index == best_index) {
                phase_t_expire = t_now_ms + 1200;
                status.phase = best_phase;
            }
            else {
                phase_t_expire = 0;
                status.phase = best_phase;
            }
            phase_last_cand_index = best_index;
        }

        // ビット位相ロック判定
        if (t_now_ms < phase_t_expire) {
            int32_t elapsed_ms = t_now_ms - phase_t_last_unlock;
            status.phase_locked = elapsed_ms >= LOCK_WAIT_TIME_MS;
            if (status.phase_locked) {
                status.phase_lock_progress = ONE;    
            }
            else {
                status.phase_lock_progress = JJY_CLIP(0, ONE, elapsed_ms * ONE / LOCK_WAIT_TIME_MS);
            }
        }
        else {
            phase_t_last_unlock = t_now_ms;
            status.phase_lock_progress = 0;
        }

        return detect_bit(in, phase, out);
    }

    bool detect_bit(uint8_t in, int32_t phase, jjybit_t *out) {
        // ビット検出
        int32_t bit_phase = phase_add(phase, -status.phase);
        int32_t slot = bit_phase * BITDET_NUM_SLOTS / PHASE_PERIOD;
        bool slot_changed = bitdet_last_slot != slot;
        bitdet_last_slot = slot;

        if (!status.phase_locked) {
            bitdet_hi_count = 0;
            bitdet_lo_count = 0;
            bitdet_sreg = 0;
            bitdet_quality_accum = 0;
            status.bit_det_quality = 0;
            status.bit_det_progress = 0;
            status.bit_det_ok = false;
            return false;
        }

        bool out_enable = false;
        if (!slot_changed) {
            if (in) bitdet_hi_count += 1;
            else bitdet_lo_count += 1;
        }
        else {
            uint8_t hi = bitdet_hi_count > bitdet_lo_count ? 1 : 0;
            bitdet_sreg <<= 1;
            bitdet_sreg |= hi;
            bitdet_sreg &= (1 << BITDET_NUM_SLOTS) - 1;

            bitdet_quality_accum += (hi ? bitdet_hi_count : bitdet_lo_count) * ONE / (bitdet_hi_count + bitdet_lo_count);

            out_enable = (slot == 0);
            jjybit_t out_value;
            if (out_enable) {
                switch (bitdet_sreg) {
                case 0b1100000000: out_value = jjybit_t::MARKER; break;
                case 0b1111100000: out_value = jjybit_t::ONE; break;
                case 0b1111111100: out_value = jjybit_t::ZERO; break;
                default: out_value = jjybit_t::ERROR; break;
                }
                status.bit_det_quality = bitdet_quality_accum / BITDET_NUM_SLOTS;
                if (out_value == jjybit_t::ERROR) {
                    bitdet_ok_count = 0;
                } 
                else {
                    bitdet_ok_count = JJY_MIN(BITDET_OK_THRESH, bitdet_ok_count + 1);
                }

            }
            *out = out_value;

            bitdet_hi_count = 0;
            bitdet_lo_count = 0;
            bitdet_quality_accum = 0;
        }

        status.bit_det_ok = bitdet_ok_count >= BITDET_OK_THRESH;
        if (status.bit_det_ok) {
            status.bit_det_progress = ONE;
        }
        else if (bitdet_ok_count == 0) {
            status.bit_det_progress = 0;
        }
        else {
            status.bit_det_progress = (((bitdet_ok_count - 1) * PHASE_PERIOD) + bit_phase) / (BITDET_OK_THRESH - 1);
        }

        return out_enable;
    }

    void add_edge(int32_t phase) {
        constexpr int score_add = 3;
        constexpr int32_t NEAR_THRESH = PHASE_PERIOD / 20;

        // 位相が近い既知のエッジを探す
        bool found = false;
        for (int i = 0; i < NUM_PHASE_CANDS; i++) {
            phase_cand_t &cand = status.phase_cands[i];
            if (!cand.valid) continue;
            int32_t phase_diff = calc_phase_diff(phase, cand.phase);
            if (JJY_ABS(phase_diff) < NEAR_THRESH) {
                cand.phase = (cand.phase + PHASE_PERIOD + phase_diff / (2 + cand.score * 8 / SCORE_MAX)) % PHASE_PERIOD;
                cand.score = JJY_MIN(SCORE_MAX, cand.score + score_add);
                found = true;
                break;
            }
        }

        if (found) {
            // 近接したエッジ同士を統合する
            int nearest_ia = -1, nearest_ib = -1;
            int nearest_diff = PHASE_PERIOD;
            for (int ia = 0; ia < NUM_PHASE_CANDS - 1; ia++) {
                phase_cand_t &cand_a = status.phase_cands[ia];
                if (!cand_a.valid) continue;
                for (int ib = ia + 1; ib < NUM_PHASE_CANDS; ib++) {
                    phase_cand_t &cand_b = status.phase_cands[ib];
                    if (!cand_b.valid) continue;
                    int32_t phase_diff = calc_phase_diff(cand_b.phase, cand_a.phase);
                    if (JJY_ABS(phase_diff) >= NEAR_THRESH) continue;;
                    if (JJY_ABS(phase_diff) < JJY_ABS(nearest_diff)) {
                        nearest_ia = ia;
                        nearest_ib = ib;
                        nearest_diff = phase_diff;
                    }
                }
            }
            if (nearest_ia >= 0 && nearest_ib >= 0) {
                phase_cand_t &cand_a = status.phase_cands[nearest_ia];
                phase_cand_t &cand_b = status.phase_cands[nearest_ib];
                cand_a.phase += nearest_diff * cand_b.score / (cand_a.score + cand_b.score);
                cand_a.score = JJY_MIN(SCORE_MAX, cand_a.score + cand_b.score);
                cand_b.valid = false;
                cand_b.score = 0;
            }
        }
        else {
            // 見つからなかったら新しいエッジとして登録
            for (int i = 0; i < NUM_PHASE_CANDS; i++) {
                phase_cand_t &cand = status.phase_cands[i];
                if (!cand.valid) {
                    cand.valid = true;
                    cand.score = score_add;
                    cand.phase = phase;
                    break;
                }
            }
        }
    }

    /// @brief 検波器の状態を取得
    const sync_status_t &get_status() const {
        return (const sync_status_t &)status;
    }
};

}

#endif
