#ifndef SHAPOCO_JJY_DATETIME_HPP
#define SHAPOCO_JJY_DATETIME_HPP

#include <stdint.h>

#include "shapoco/jjy/common.hpp"

namespace jjy {

enum class JjyDayOfWeek : uint8_t {
    SUNDAY = 0,
    MONDAY = 1,
    TUESDAY = 2,
    WDNESDAY = 3,
    THURSDAY = 4,
    FRIDAY = 5,
    SATURDAY = 6,
};

enum class JjyLeapSecondType : uint8_t {
    SKIP = 0,
    INSERT = 1,
};

struct ParseResut {
    static constexpr uint32_t EMPTY = (1 << 0);
    static constexpr uint32_t BAD_MARKER = (1 << 1);
    static constexpr uint32_t BAD_POSITION_MARKER = (1 << 2);
    static constexpr uint32_t BAD_SECOND = (1 << 3);
    static constexpr uint32_t BAD_MINUTE = (1 << 4);
    static constexpr uint32_t BAD_HOUR = (1 << 5);
    static constexpr uint32_t BAD_DAY_OR_YEAR = (1 << 6);
    static constexpr uint32_t BAD_HOUR_PARITY = (1 << 7);
    static constexpr uint32_t BAD_MINUTE_PARITY = (1 << 8);
    static constexpr uint32_t BAD_YEAR_OF_CENTURY = (1 << 9);
    static constexpr uint32_t BAD_DAY_OF_WEEK = (1 << 10);
    static constexpr uint32_t BAD_LS1 = (1 << 11);
    static constexpr uint32_t BAD_LS2 = (1 << 12);
    static constexpr uint32_t BAD_RESERVED_BITS = (1 << 13);
    uint32_t flags = 0;
    bool success() const { return flags == 0; }
};

static constexpr int MONTH_OFFSET = 1;
static constexpr int DAY_OFFSET = 1;

bool isLeapYear(int year);
bool getNumDaysInMonth(int month, bool leapYear, int *out);
bool separateDayOfYear(int dayOfYear, bool leapYear, int *month, int *day);

struct JjyDateTime {

    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hours;
    uint8_t minutes;
    uint8_t second;
    JjyDayOfWeek day_of_week;
    bool leap_second_at_end_of_month;
    JjyLeapSecondType leap_second_type;

    ParseResut parse(const jjybit_t *bits, int year_offset) {
        ParseResut rslt;
        bool err;

        if (bits[0] != jjybit_t::MARKER) rslt.flags |= ParseResut::BAD_MARKER;
        if (bits[9] != jjybit_t::MARKER) rslt.flags |= ParseResut::BAD_POSITION_MARKER;
        if (bits[19] != jjybit_t::MARKER) rslt.flags |= ParseResut::BAD_POSITION_MARKER;
        if (bits[29] != jjybit_t::MARKER) rslt.flags |= ParseResut::BAD_POSITION_MARKER;
        if (bits[39] != jjybit_t::MARKER) rslt.flags |= ParseResut::BAD_POSITION_MARKER;
        if (bits[49] != jjybit_t::MARKER) rslt.flags |= ParseResut::BAD_POSITION_MARKER;
        if (bits[59] != jjybit_t::MARKER) rslt.flags |= ParseResut::BAD_POSITION_MARKER;

        err = false;
        minutes =
            readInt(bits, 1, 3, 0, 5, &err) * 10 + 
            readInt(bits, 4, 5, 0, 9, &err);
        if (err) rslt.flags |= ParseResut::BAD_MINUTE;

        err = false;
        hours = 
            readInt(bits, 10, 4, 0, 2, &err) * 10 + 
            readInt(bits, 14, 5, 0, 9, &err);
        err |= (hours < 0 || 23 < hours);
        if (err) rslt.flags |= ParseResut::BAD_HOUR;

        err = false;
        year = year_offset +
            readInt(bits, 41, 4, 0, 9, &err) * 10 +
            readInt(bits, 45, 4, 0, 9, &err);
        if (err) rslt.flags |= ParseResut::BAD_YEAR_OF_CENTURY;
        bool leapYear = isLeapYear(year);

        err = false;
        int day_of_year = 
            readInt(bits, 20, 4, 0, 3, &err) * 100 + 
            readInt(bits, 24, 5, 0, 9, &err) * 10 +
            readInt(bits, 30, 4, 0, 9, &err);
        int numDays = leapYear ? 366 : 365;
        int m, d;
        err |=
            (day_of_year < 1 || numDays < day_of_year) ||
            !separateDayOfYear(day_of_year, leapYear, &m, &d) ||
            (bits[34] != jjybit_t::ZERO) ||
            (bits[35] != jjybit_t::ZERO);
        if (err) rslt.flags |= ParseResut::BAD_DAY_OR_YEAR;
        month = m;
        day = d;

        if (!checkEvenParity(bits, 12, 7, 36)) rslt.flags |= ParseResut::BAD_HOUR_PARITY;
        if (!checkEvenParity(bits, 1, 8, 37)) rslt.flags |= ParseResut::BAD_MINUTE_PARITY;

        err = false;
        day_of_week = (JjyDayOfWeek)readInt(bits, 50, 3, 0, 6, &err);
        if (err) rslt.flags |= ParseResut::BAD_DAY_OF_WEEK;
        
        err = false;
        leap_second_at_end_of_month = readInt(bits, 53, 1, 0, 1, &err);
        if (err) rslt.flags |= ParseResut::BAD_LS1;

        err = false;
        leap_second_type = readInt(bits, 54, 1, 0, 1, &err) == 0 ? JjyLeapSecondType::SKIP : JjyLeapSecondType::INSERT;
        if (err) rslt.flags |= ParseResut::BAD_LS2;

        err = false;
        readInt(bits, 38, 1, 0, 1, &err);
        readInt(bits, 40, 1, 0, 1, &err);
        readInt(bits, 55, 4, 0, 0, &err);
        if (err) rslt.flags |= ParseResut::BAD_RESERVED_BITS;

        second = 0;

        return rslt;
    }

    uint32_t readInt(const jjybit_t *bits, int offset, int width, uint32_t minVal, uint32_t maxVal, bool *err) {
        uint32_t val = 0;
        for (int i = 0; i < width; i++) {
            val <<= 1;
            switch (bits[offset++]) {
            case jjybit_t::ZERO: /* do nothing */ break;
            case jjybit_t::ONE: val |= 1; break;
            default: *err = true; break;
            }
        }
        if (val < minVal || maxVal < val) *err = true;
        return val;
    }

    bool checkEvenParity(const jjybit_t *bits, int data_offset, int data_width, int parity_offset) {
        bool err = false;
        uint32_t data = readInt(bits, data_offset, data_width, 0, (1 << data_width) - 1, &err);
        uint32_t parity = readInt(bits, parity_offset, 1, 0, 1, &err);
        data = ((data >> 16) ^ data) & 0xffff;
        data = ((data >> 8) ^ data) & 0xff;
        data = ((data >> 4) ^ data) & 0xf;
        data = ((data >> 2) ^ data) & 0x3;
        data = ((data >> 1) ^ data) & 0x1;
        return !err && ((data ^ parity) == 0);
    }

    void addSecond(int s) {
        s += second; second = s % 60; s /= 60;
        s += minutes; minutes = s % 60; s /= 60;
        s += hours; hours = s % 24; s /= 24;
        
        s += (day - DAY_OFFSET);
        while (true) {
            int leapYear = isLeapYear(year);
            int daysInMonth;
            getNumDaysInMonth(month, leapYear, &daysInMonth);
            if (s < daysInMonth) {
                day = s + DAY_OFFSET;
                break;
            }
            day -= daysInMonth;
            if (month < 12 - MONTH_OFFSET) {
                month += 1;
            }
            else {
                month = MONTH_OFFSET;
                year += 1;
            }
        }

    }
};

}

#endif

