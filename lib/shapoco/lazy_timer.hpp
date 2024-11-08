#ifndef LAZY_TIMER_HPP
#define LAZY_TIMER_HPP

template<typename T, T INITIAL_PERIOD, bool AUTO_LOOP = true>
class LazyTimer {
public:
    T t_next = 0;
    T period = INITIAL_PERIOD;

    void set_period(T period) {
        this->period = period;
    }

    void start(T t_now, T phase = 0) {
        t_next = t_now + period - phase;
    }

    void set_expired() {
        t_next = 0;
    }

    bool is_expired(T t_now) {
        bool expired = t_now >= t_next;
        if (AUTO_LOOP && expired) {
            t_next += period;
            if (t_next < t_now) {
                t_next = t_now + 1;
            }
        }
        return expired;
    }

    T elapsed(T t_now) const {
        T t_start = t_next - period;
        return t_now - t_start;
    }

    T phase(T t_now) const {
        T t_phase = elapsed();
        if (t_phase >= period) {
            do { t_phase -= period; } while (t_phase >= period);
        }
        return t_phase;
    }
};

#endif
