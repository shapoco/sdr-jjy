#ifndef JJY_RX_SYNCHRONIZER
#define JJY_RX_SYNCHRONIZER

#include <stdint.h>

#include "pico/stdlib.h"

#include "jjy/common.hpp"

namespace jjy::rx {

static constexpr int NUM_PHASE_CANDS = 10;
static constexpr int32_t PHASE_PERIOD = 1 << PREC;

typedef struct {
    int32_t phase;
    int weight;
    bool valid;
} phase_cand_t;

typedef struct {
    phase_cand_t phase_cands[NUM_PHASE_CANDS];
} sync_status_t;

class Synchronizer {
public:
    sync_status_t status;
    uint8_t in_last = 0;
    uint32_t t_last_ms = 0;
    uint32_t t_next_cand_upd;

private:
    static constexpr int WEIGHT_MAX = 10;

public:
    void init() {
        in_last = 0;
        for (int i = 0; i < NUM_PHASE_CANDS; i++) {
            status.phase_cands[i].valid = false;
            status.phase_cands[i].phase = 0;
        }
        uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());;
        t_last_ms = t_now_ms;
        t_next_cand_upd = t_now_ms;
    }

    bool synchronize(uint32_t t_now_ms, uint8_t in, uint8_t *out) {
        uint32_t t_delta = t_now_ms - t_last_ms;
        t_last_ms = t_now_ms;

        int32_t phase = (t_now_ms % 1000) * PHASE_PERIOD / 1000;

        // エッジの減衰
        if (t_now_ms > t_next_cand_upd) {
            t_next_cand_upd = JJY_MAX(t_now_ms + 500, t_next_cand_upd + 1000);
            for (int i = 0; i < NUM_PHASE_CANDS; i++) {
                phase_cand_t &cand = status.phase_cands[i];
                if (!cand.valid) continue;
                cand.valid = (--cand.weight > 0);
            }
        }

        // エッジ検出
        bool rise = !!in && !in_last;
        in_last = in;
        if (rise) add_edge(phase);

        return false;
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
                cand.phase = (cand.phase + PHASE_PERIOD + phase_diff / (2 + cand.weight * 8 / WEIGHT_MAX)) % PHASE_PERIOD;
                cand.weight = JJY_MIN(WEIGHT_MAX, cand.weight + score_add);
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
                cand_a.phase += nearest_diff * cand_b.weight / (cand_a.weight + cand_b.weight);
                cand_a.weight = JJY_MIN(WEIGHT_MAX, cand_a.weight + cand_b.weight);
                cand_b.valid = false;
                cand_b.weight = 0;
            }
        }
        else {
            // 見つからなかったら新しいエッジとして登録
            for (int i = 0; i < NUM_PHASE_CANDS; i++) {
                phase_cand_t &cand = status.phase_cands[i];
                if (!cand.valid) {
                    cand.valid = true;
                    cand.weight = score_add;
                    cand.phase = phase;
                    break;
                }
            }
        }
    }

    static int32_t calc_phase_diff(int32_t a, int32_t b) {
        int32_t diff = a - b;
        if (diff < -PHASE_PERIOD / 2) return diff + PHASE_PERIOD;
        if (diff >= PHASE_PERIOD / 2) return diff - PHASE_PERIOD;
        return diff;
    }

    /// @brief 検波器の状態を取得
    const sync_status_t &get_status() const {
        return (const sync_status_t &)status;
    }
};

}

#endif
