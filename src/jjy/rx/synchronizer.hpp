#ifndef JJY_RX_SYNCHRONIZER
#define JJY_RX_SYNCHRONIZER

#include <stdint.h>
#include <math.h>

#include "pico/stdlib.h"

#include "jjy/common.hpp"
#include "lazy_timer.hpp"

namespace jjy::rx {

static constexpr int NUM_PHASE_CANDS = 10;

typedef struct {
    int32_t phase;
    int score;
    bool valid;
} phase_cand_t;

typedef struct {
    phase_cand_t phase_cands[NUM_PHASE_CANDS];
    int32_t phase_cursor;
    int32_t phase_offset;
    bool phase_locked;
    int32_t phase_lock_progress;
    int32_t bit_det_quality;
    int32_t bit_det_progress;
    bool bit_det_ok;
    bool out_enable;
    jjybit_t out_value;
} sync_status_t;

class Synchronizer {
private:
    static constexpr int CAND_EXPIRE_TIME_MS = 1200;
    static constexpr int LOCK_WAIT_TIME_MS = 1024 * 5;
    static constexpr int SCORE_MAX = 10;
    static constexpr uint32_t BITDET_SREG_INITVAL = 0x55555555;
    static constexpr int BITDET_NUM_SLOTS = 6;
    static constexpr int BITDET_OK_THRESH = 10;

    static constexpr int BITDET_QTY_HISTORY_SIZE = 6;

    int32_t bitdet_qty_history[BITDET_QTY_HISTORY_SIZE];
    int bitdet_qty_history_index = 0;

    uint8_t bitdet_ok_history[BITDET_OK_THRESH];
    int bitdet_ok_history_index = 0;
    int bitdet_ok_history_count = 0;

    LazyTimer<uint32_t, 1000> cand_update_timer;
    LazyTimer<uint32_t, CAND_EXPIRE_TIME_MS, false> cand_expire_timer;
    LazyTimer<uint32_t, LOCK_WAIT_TIME_MS, false> lock_wait_timer;

public:
    sync_status_t status;

    uint32_t t_last_ms = 0;
    uint8_t last_in = 0;

    int phase_last_cand_index = -1;

    int32_t bitdet_last_slot;
    int bitdet_hi_count = 0;
    int bitdet_lo_count = 0;
    uint32_t bitdet_sreg = BITDET_SREG_INITVAL;
    int bitdet_ok_count = 0;

    void init(uint32_t t_now_ms) {
        t_last_ms = t_now_ms;
        last_in = 0;

        cand_update_timer.start(t_now_ms);
        cand_expire_timer.set_expired();
        lock_wait_timer.start(t_now_ms);
        phase_last_cand_index = -1;

        bitdet_last_slot = 0;
        bitdet_hi_count = 0;
        bitdet_lo_count = 0;
        bitdet_sreg = BITDET_SREG_INITVAL;
        bitdet_ok_count = 0;

        for (int i = 0; i < BITDET_QTY_HISTORY_SIZE; i++) {
            bitdet_qty_history[i] = 0;
        }
        bitdet_qty_history_index = 0;

        for (int i = 0; i < BITDET_OK_THRESH; i++) {
            bitdet_ok_history[i] = 0;
        }
        bitdet_ok_history_index = 0;
        bitdet_ok_history_count = 0;

        for (int i = 0; i < NUM_PHASE_CANDS; i++) {
            status.phase_cands[i].valid = false;
            status.phase_cands[i].phase = 0;
        }
        status.phase_offset = 0;
        status.phase_cursor = (t_now_ms % 1000) * PHASE_PERIOD / 1000;
        status.phase_locked = false;
        status.phase_lock_progress = 0;
        status.bit_det_quality = 0;
        status.bit_det_progress = 0;
        status.bit_det_ok = false;
    }

    bool process(uint32_t t_now_ms, uint8_t in, jjybit_t *out) {
        uint32_t t_delta = t_now_ms - t_last_ms;
        t_last_ms = t_now_ms;
        if (t_delta == 0) return false;

        int32_t phase_ms = t_now_ms % 1000;
        int32_t phase = phase_ms * PHASE_PERIOD / 1000;
        status.phase_cursor = phase;

        bool cand_upd = cand_update_timer.is_expired(t_now_ms);

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
                cand_expire_timer.set_expired();
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
                cand_expire_timer.set_expired();
            }
            else if (phase_last_cand_index == best_index) {
                cand_expire_timer.start(t_now_ms);
                status.phase_offset = best_phase;
            }
            else {
                cand_expire_timer.set_expired();
                status.phase_offset = best_phase;
            }
            phase_last_cand_index = best_index;
        }

        if (cand_expire_timer.is_expired(t_now_ms)) {
            // 位相候補期限切れ
            lock_wait_timer.start(t_now_ms);
            status.phase_lock_progress = 0;
        }
        else {
            // ビット位相ロック判定
            status.phase_locked = lock_wait_timer.is_expired(t_now_ms);
            int32_t elapsed_ms = lock_wait_timer.elapsed(t_now_ms);
            if (status.phase_locked) {
                status.phase_lock_progress = ONE;    
            }
            else {
                status.phase_lock_progress = JJY_CLIP(0, ONE, elapsed_ms * ONE / LOCK_WAIT_TIME_MS);
            }
        }

        status.out_enable = detect_bit(in, phase, out);
        status.out_value = *out;

        return status.out_enable;
    }

    bool detect_bit(uint8_t in, int32_t phase, jjybit_t *out) {
        int32_t bit_phase = phase_add(phase, -status.phase_offset);
        int slot =
            (bit_phase < PHASE_PERIOD * 20 / 100) ? 0 :
            (bit_phase < PHASE_PERIOD * 35 / 100) ? 1 :
            (bit_phase < PHASE_PERIOD * 50 / 100) ? 2 :
            (bit_phase < PHASE_PERIOD * 65 / 100) ? 3 :
            (bit_phase < PHASE_PERIOD * 80 / 100) ? 4 :
            5; 

        bool slot_changed = slot != bitdet_last_slot;
        bool slot_unexp_change = slot_changed && (slot != (bitdet_last_slot + 1) % BITDET_NUM_SLOTS);
        bitdet_last_slot = slot;

        bool out_enable = false;
        jjybit_t out_value =  jjybit_t::ERROR;
        if (!slot_changed) {
            if (in) bitdet_hi_count += 1;
            else bitdet_lo_count += 1;
        }
        else if (slot_unexp_change) {
            bitdet_hi_count = 0;
            bitdet_lo_count = 0;
            bitdet_sreg = BITDET_SREG_INITVAL;
            status.bit_det_quality /= 2;
        }
        else {
            uint8_t hi = bitdet_hi_count > bitdet_lo_count ? 1 : 0;
            bitdet_sreg <<= 1;
            bitdet_sreg |= hi;
            bitdet_sreg &= (1 << BITDET_NUM_SLOTS) - 1;

            out_enable = (slot == 0);
            if (out_enable) {
                switch (bitdet_sreg) {
                case 0b100000: out_value = jjybit_t::MARKER; break;
                case 0b111000: out_value = jjybit_t::ONE; break;
                case 0b111110: out_value = jjybit_t::ZERO; break;
                default: out_value = jjybit_t::ERROR; break;
                }

                if (out_value == jjybit_t::ERROR) {
                    bitdet_ok_count = 0;
                } 
                else {
                    bitdet_ok_count = JJY_MIN(BITDET_OK_THRESH, bitdet_ok_count + 1);
                }

                bitdet_ok_history[bitdet_ok_history_index] = (out_value == jjybit_t::ERROR) ? 0 : 1;
                bitdet_ok_history_index = (bitdet_ok_history_index + 1) % BITDET_OK_THRESH; 
                bitdet_ok_history_count = JJY_MIN(BITDET_OK_THRESH, bitdet_ok_history_count + 1);
            }

            if (slot == 0 || slot == 1 || slot == 3 || slot == 5) {
                bitdet_qty_history[bitdet_qty_history_index] = JJY_ABS(bitdet_hi_count - bitdet_lo_count) * ONE / (bitdet_hi_count + bitdet_lo_count);
                bitdet_qty_history_index = (bitdet_qty_history_index + 1) % BITDET_QTY_HISTORY_SIZE; 

                int32_t qty_ave = 0;
                for (int i = 0; i < BITDET_QTY_HISTORY_SIZE; i++) {
                    qty_ave += bitdet_qty_history[i];
                }
                qty_ave /= BITDET_QTY_HISTORY_SIZE;
                
                int32_t ok_rate = 0;
                if (bitdet_ok_history_count > 0) {
                    for (int i = 0; i < BITDET_OK_THRESH; i++) {
                        ok_rate += bitdet_ok_history[i];
                    }
                    ok_rate = ok_rate * jjy::ONE / bitdet_ok_history_count;
                }

                status.bit_det_quality = qty_ave * ok_rate / jjy::ONE;
            }

            bitdet_hi_count = 0;
            bitdet_lo_count = 0;
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

        if (!status.phase_locked) {
            //bitdet_hi_count = 0;
            //bitdet_lo_count = 0;
            //bitdet_sreg = 0;
            //bitdet_quality_accum = 0;
            //status.bit_det_quality = 0;
            bitdet_ok_count = 0;
            status.bit_det_ok = false;
            status.bit_det_progress = 0;
        }

        *out = out_value;
        return out_enable && status.phase_locked;
    }

    void add_edge(int32_t phase) {
        constexpr int score_add = 3;
        constexpr int32_t NEAR_THRESH = PHASE_PERIOD / 20;

        // 位相が近い既知のエッジを探す
        bool found = false;
        for (int i = 0; i < NUM_PHASE_CANDS; i++) {
            phase_cand_t &cand = status.phase_cands[i];
            if (!cand.valid) continue;
            int32_t diff = phase_diff(phase, cand.phase);
            if (JJY_ABS(diff) < NEAR_THRESH) {
                phase_follow(&cand.phase, phase, ONE / (2 + cand.score * 8 / SCORE_MAX));
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
                    int32_t diff = phase_diff(cand_b.phase, cand_a.phase);
                    if (JJY_ABS(diff) >= NEAR_THRESH) continue;;
                    if (JJY_ABS(diff) < JJY_ABS(nearest_diff)) {
                        nearest_ia = ia;
                        nearest_ib = ib;
                        nearest_diff = diff;
                    }
                }
            }
            if (nearest_ia >= 0 && nearest_ib >= 0) {
                phase_cand_t &cand_a = status.phase_cands[nearest_ia];
                phase_cand_t &cand_b = status.phase_cands[nearest_ib];
                cand_a.phase = phase_add(cand_a.phase, nearest_diff * cand_b.score / (cand_a.score + cand_b.score));
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
