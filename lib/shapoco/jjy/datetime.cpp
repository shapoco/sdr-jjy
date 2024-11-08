
#include "shapoco/jjy/datetime.hpp"

namespace shapoco::jjy {

bool isLeapYear(int year) {
    return (year % 400 == 0) || (year % 100 != 0 && year % 4 == 0);
}

bool getNumDaysInMonth(int month, bool leapYear, int *out) {
    switch(month) {
    case MONTH_OFFSET + 0: *out = 31; return true;
    case MONTH_OFFSET + 1: *out = leapYear ? 29 : 28; return true;
    case MONTH_OFFSET + 2: *out = 31; return true;
    case MONTH_OFFSET + 3: *out = 30; return true;
    case MONTH_OFFSET + 4: *out = 31; return true;
    case MONTH_OFFSET + 5: *out = 30; return true;
    case MONTH_OFFSET + 6: *out = 31; return true;
    case MONTH_OFFSET + 7: *out = 31; return true;
    case MONTH_OFFSET + 8: *out = 30; return true;
    case MONTH_OFFSET + 9: *out = 31; return true;
    case MONTH_OFFSET + 10: *out = 30; return true;
    case MONTH_OFFSET + 11: *out = 31; return true;
    default: *out = 0; return false;
    }
}

bool separateDayOfYear(int dayOfYear, bool leapYear, int *month, int *day) {
    int d = dayOfYear - DAY_OFFSET;
    if (d < 0) return false;
    for (int m = MONTH_OFFSET; m < MONTH_OFFSET + 12; m++) {
        int daysInMonth;
        if (!getNumDaysInMonth(m, leapYear, &daysInMonth)) {
            return false;
        }
        if (d < daysInMonth) {
            *month = m;
            *day = DAY_OFFSET + d;
            return true;
        }
        d -= daysInMonth;
    }
    return false;
}

}
