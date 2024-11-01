#ifndef JJY_RX_SYNCHRONIZER
#define JJY_RX_SYNCHRONIZER

#include <stdint.h>

#include "pico/stdlib.h"

#include "jjy/common.hpp"

namespace jjy::rx {

static constexpr int NUM_PHASE_CANDS = 10;
static constexpr uint32_t PHASE_PERIOD = 1 << PREC;

typedef struct {
    uint32_t phase;
    int score;
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

        uint32_t phase = (t_now_ms % 1000) * PHASE_PERIOD / 1000;

        // エッジの減衰
        if (t_now_ms > t_next_cand_upd) {
            t_next_cand_upd = JJY_MAX(t_now_ms + 500, t_next_cand_upd + 1000);
            for (int i = 0; i < NUM_PHASE_CANDS; i++) {
                phase_cand_t &cand = status.phase_cands[i];
                if (!cand.valid) continue;
                cand.valid = (--cand.score > 0);
            }
        }

        // エッジ検出
        bool rise = !!in && !in_last;
        in_last = in;
        if (rise) add_edge(phase);

        return false;
    }

    void add_edge(uint32_t phase) {
        constexpr int score_add = 3;

        // 位相が近い既知のエッジを探す
        bool found = false;
        for (int i = 0; i < NUM_PHASE_CANDS; i++) {
            phase_cand_t &cand = status.phase_cands[i];
            if (!cand.valid) continue;
            int phase_diff = (phase + PHASE_PERIOD - cand.phase) % PHASE_PERIOD;
            if (JJY_ABS(phase_diff) <= PHASE_PERIOD / 2) {
                cand.phase = (cand.phase + PHASE_PERIOD + phase_diff / 10) % PHASE_PERIOD;
                cand.score = JJY_MIN(NUM_PHASE_CANDS, cand.score + score_add);
                found = true;
                break;
            }
        }

        if (!found) {
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
