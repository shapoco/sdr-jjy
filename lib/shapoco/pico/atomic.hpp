#pragma once

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"

namespace shapoco::pico {

template<typename T>
class atomic {
private:
    T value;
    mutex_t mtx;

public:
    atomic() {
        mutex_init(&mtx);
    }

    void store(const T v) {
        mutex_enter_blocking(&mtx);
        this->value = v;
        mutex_exit(&mtx);
    }

    T load() {
        mutex_enter_blocking(&mtx);
        const T ret = this->value;
        mutex_exit(&mtx);
        return ret;
    }
};

}
