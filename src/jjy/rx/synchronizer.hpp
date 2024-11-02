#ifndef JJY_RX_SYNCHRONIZER
#define JJY_RX_SYNCHRONIZER

#include <stdint.h>
#include <math.h>

#include "pico/stdlib.h"

#include "jjy/common.hpp"

namespace jjy::rx {

static constexpr int NUM_PHASE_CANDS = 10;
static constexpr int LOCK_WAIT_TIME_MS = 1024 * 3;

typedef struct {
    int32_t phase;
    int score;
    bool valid;
} phase_cand_t;

typedef struct {
    phase_cand_t phase_cands[NUM_PHASE_CANDS];
    int32_t phase;
    bool locked;
    int32_t lock_progress;
} sync_status_t;

class Synchronizer {
public:
    sync_status_t status;
    uint8_t in_last = 0;
    uint32_t t_last_ms = 0;
    uint32_t t_next_cand_upd;
    uint32_t t_phase_expire;
    uint32_t t_last_unlock;
    int last_cand_index = -1;

private:
    static constexpr int WEIGHT_MAX = 10;

public:
    void init() {
        in_last = 0;
        for (int i = 0; i < NUM_PHASE_CANDS; i++) {
            status.phase_cands[i].valid = false;
            status.phase_cands[i].phase = 0;
        }
        status.phase = 0;
        status.locked = false;
        status.lock_progress = 0;

        uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());;
        t_last_ms = t_now_ms;
        t_next_cand_upd = t_now_ms;
        t_last_unlock = t_now_ms;
        t_phase_expire = 0;
    }

    bool synchronize(uint32_t t_now_ms, uint8_t in, uint8_t *out) {
        uint32_t t_delta = t_now_ms - t_last_ms;
        t_last_ms = t_now_ms;

        int32_t phase = (t_now_ms % 1000) * PHASE_PERIOD / 1000;

        // エッジの減衰
        bool cand_upd = t_now_ms > t_next_cand_upd;
        if (cand_upd) {
            t_next_cand_upd = JJY_MAX(t_now_ms + 500, t_next_cand_upd + 1000);
        }

        if (cand_upd) {
            int num_valid_cand = 0;
            for (int i = 0; i < NUM_PHASE_CANDS; i++) {
                phase_cand_t &cand = status.phase_cands[i];
                if (!cand.valid) continue;
                cand.valid = (--cand.score > 0);
                if (cand.valid) num_valid_cand += 1;
            }
            if (num_valid_cand == 0) {
                t_phase_expire = 0;
            }
        }

        // エッジ検出
        bool rise = !!in && !in_last;
        in_last = in;
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
                t_phase_expire = 0;
            }
            else if (last_cand_index == best_index) {
                t_phase_expire = t_now_ms + 1200;
                status.phase = best_phase;
            }
            else {
                t_phase_expire = 0;
                status.phase = best_phase;
            }
            last_cand_index = best_index;
        }

        // Locked 判定
        bool phase_valid = t_now_ms < t_phase_expire;
        if (phase_valid) {
            int32_t elapsed_ms = t_now_ms - t_last_unlock;
            status.locked = elapsed_ms >= LOCK_WAIT_TIME_MS;
            if (status.locked) {
                status.lock_progress = ONE;    
            }
            else {
                status.lock_progress = JJY_CLIP(0, ONE, elapsed_ms * ONE / LOCK_WAIT_TIME_MS);
            }
        }
        else {
            t_last_unlock = t_now_ms;
            status.lock_progress = 0;
        }

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
                cand.phase = (cand.phase + PHASE_PERIOD + phase_diff / (2 + cand.score * 8 / WEIGHT_MAX)) % PHASE_PERIOD;
                cand.score = JJY_MIN(WEIGHT_MAX, cand.score + score_add);
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
                cand_a.score = JJY_MIN(WEIGHT_MAX, cand_a.score + cand_b.score);
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
