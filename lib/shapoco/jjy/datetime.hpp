#pragma once

#include <stdint.h>

#include "shapoco/common.hpp"
#include "shapoco/fixed12.hpp"
#include "shapoco/jjy/common.hpp"

namespace shapoco::jjy {

static constexpr int MONTH_OFFSET = 1;
static constexpr int DAY_OFFSET = 1;

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

static constexpr int DAY_OF_YEAR_JAN_DAY1 = DAY_OFFSET; // 1月1日の元日からの日数
static constexpr int DAY_OF_YEAR_FEB_DAY1 = DAY_OFFSET + 31; // 2月1日の元日からの日数
static constexpr int DAY_OF_YEAR_MAR_DAY1 = DAY_OFFSET + 59; // 3月1日の元日からの日数
static constexpr int DAY_OF_YEAR_APR_DAY1 = DAY_OFFSET + 90; // 4月1日の元日からの日数
static constexpr int DAY_OF_YEAR_MAY_DAY1 = DAY_OFFSET + 120; // 5月1日の元日からの日数
static constexpr int DAY_OF_YEAR_JUN_DAY1 = DAY_OFFSET + 151; // 6月1日の元日からの日数
static constexpr int DAY_OF_YEAR_JUL_DAY1 = DAY_OFFSET + 181; // 7月1日の元日からの日数
static constexpr int DAY_OF_YEAR_AUG_DAY1 = DAY_OFFSET + 212; // 8月1日の元日からの日数
static constexpr int DAY_OF_YEAR_SEP_DAY1 = DAY_OFFSET + 243; // 9月1日の元日からの日数
static constexpr int DAY_OF_YEAR_OCT_DAY1 = DAY_OFFSET + 273; // 10月1日の元日からの日数
static constexpr int DAY_OF_YEAR_NOV_DAY1 = DAY_OFFSET + 304; // 11月1日の元日からの日数
static constexpr int DAY_OF_YEAR_DEC_DAY1 = DAY_OFFSET + 334; // 12月1日の元日からの日数

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

bool isLeapYear(int year);

SHPC_STATIC_INLINE int getNumDaysInYear(int year) {
    return isLeapYear(year) ? 366 : 365;
}

bool tryGetNumDaysInMonth(int month, bool leapYear, int *out);
SHPC_STATIC_INLINE int getNumDaysInMonth(int month, bool leapYear) {
    int ret = -1;
    tryGetNumDaysInMonth(month, leapYear, &ret);
    return ret;
}

bool tryGetDayOfYear(int month, int day, bool leapYear, int *out);
SHPC_STATIC_INLINE int getDayOfYear(int month, int day, bool leapYear) {
    int ret = -1;
    tryGetDayOfYear(month, day, leapYear, &ret);
    return ret;

}

bool trySeparateDayOfYear(int dayOfYear, bool leapYear, int *month, int *day);
SHPC_STATIC_INLINE bool trySeparateDayOfYear(int dayOfYear, bool leapYear, uint8_t *month, uint8_t *day) {
    int m = 0, d = 0;
    bool ret = trySeparateDayOfYear(dayOfYear, leapYear, &m, &d);
    *month = m;
    *day = d;
    return ret;
}

struct JjyDateTime {

    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t millis;
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
        minute =
            readInt(bits, 1, 3, 0, 5, &err) * 10 + 
            readInt(bits, 4, 5, 0, 9, &err);
        if (err) rslt.flags |= ParseResut::BAD_MINUTE;

        err = false;
        hour = 
            readInt(bits, 10, 4, 0, 2, &err) * 10 + 
            readInt(bits, 14, 5, 0, 9, &err);
        err |= (hour < 0 || 23 < hour);
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
        err |=
            (day_of_year < 1 || numDays < day_of_year) ||
            !trySeparateDayOfYear(day_of_year, leapYear, &month, &day) ||
            (bits[34] != jjybit_t::ZERO) ||
            (bits[35] != jjybit_t::ZERO);
        if (err) rslt.flags |= ParseResut::BAD_DAY_OR_YEAR;

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
        millis = 0;

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

    void addSecondsSelf(int64_t s) {
        addMillisSelf(s * 1000);
    }

    void addMillisSelf(int64_t ms) {
        int64_t carry = ms;

        if (carry == 0) return;
        carry += millis; millis = shapoco::cyclicNorm(carry, 1000); carry /= 1000;

        if (carry == 0) return;
        carry += second; second = shapoco::cyclicNorm(carry, 60); carry /= 60;

        if (carry == 0) return;
        carry += minute; minute = shapoco::cyclicNorm(carry, 60); carry /= 60;

        if (carry == 0) return;
        carry += hour; hour = shapoco::cyclicNorm(carry, 24); carry /= 24;

        if (carry == 0) return;
        bool leapYear = isLeapYear(year);
        carry += getDayOfYear(month, day, leapYear) - DAY_OFFSET;

        if (carry >= 0) {
            int daysInYear;
            while (carry >= (daysInYear = (leapYear ? 366 : 365))) {
                carry -= daysInYear;
                year += 1;
                leapYear = isLeapYear(year);
            }
        }
        else {
            do {
                year -= 1; 
                leapYear = isLeapYear(year);
                carry += leapYear ? 366 : 365;
            } while (carry < 0);
        }

        trySeparateDayOfYear(DAY_OFFSET + carry, leapYear, &month, &day);
    }
};

}
