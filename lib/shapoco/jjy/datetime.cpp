
#include "shapoco/jjy/datetime.hpp"

namespace shapoco::jjy {

bool isLeapYear(int year) {
    return (year % 400 == 0) || (year % 100 != 0 && year % 4 == 0);
}

bool tryGetNumDaysInMonth(int month, bool leapYear, int *out) {
    switch(month - MONTH_OFFSET) {
    case 0: *out = 31; return true;
    case 1: *out = leapYear ? 29 : 28; return true;
    case 2: *out = 31; return true;
    case 3: *out = 30; return true;
    case 4: *out = 31; return true;
    case 5: *out = 30; return true;
    case 6: *out = 31; return true;
    case 7: *out = 31; return true;
    case 8: *out = 30; return true;
    case 9: *out = 31; return true;
    case 10: *out = 30; return true;
    case 11: *out = 31; return true;
    default: return false;
    }
}

bool tryGetDayOfYear(int month, int day, bool leapYear, int *out) {
    int daysInMonth;
    if (!tryGetNumDaysInMonth(month, leapYear, &daysInMonth)) return false;
    if (day < 1 || daysInMonth < day) return false;

    day -= DAY_OFFSET;
    int leapDay = leapYear ? 1 : 0;
    switch(month - MONTH_OFFSET) {
    case 0: *out = DAY_OF_YEAR_JAN_DAY1 + day; return true;
    case 1: *out = DAY_OF_YEAR_FEB_DAY1 + day; return true;
    case 2: *out = DAY_OF_YEAR_MAR_DAY1 + day + leapDay; return true;
    case 3: *out = DAY_OF_YEAR_APR_DAY1 + day + leapDay; return true;
    case 4: *out = DAY_OF_YEAR_MAY_DAY1 + day + leapDay; return true;
    case 5: *out = DAY_OF_YEAR_JUN_DAY1 + day + leapDay; return true;
    case 6: *out = DAY_OF_YEAR_JUL_DAY1 + day + leapDay; return true;
    case 7: *out = DAY_OF_YEAR_AUG_DAY1 + day + leapDay; return true;
    case 8: *out = DAY_OF_YEAR_SEP_DAY1 + day + leapDay; return true;
    case 9: *out = DAY_OF_YEAR_OCT_DAY1 + day + leapDay; return true;
    case 10: *out = DAY_OF_YEAR_NOV_DAY1 + day + leapDay; return true;
    case 11: *out = DAY_OF_YEAR_DEC_DAY1 + day + leapDay; return true;
    default: return false;
    }
}

bool trySeparateDayOfYear(int dayOfYear, bool leapYear, int *month, int *day) {
    if (dayOfYear < 1 || getNumDaysInYear(leapYear) < dayOfYear) return false;

    int leapDay = leapYear ? 1 : 0;

    if (dayOfYear < DAY_OF_YEAR_MAR_DAY1 + leapDay) {
        if (dayOfYear < DAY_OF_YEAR_FEB_DAY1) {
            *month = MONTH_OFFSET + 0; *day = DAY_OFFSET - DAY_OF_YEAR_JAN_DAY1 + dayOfYear;
        }
        else {
            *month = MONTH_OFFSET + 1; *day = DAY_OFFSET - DAY_OF_YEAR_FEB_DAY1 + dayOfYear;
        }
    }
    else {
        dayOfYear -= leapDay;
        if (dayOfYear < DAY_OF_YEAR_AUG_DAY1) {
            if (dayOfYear < DAY_OF_YEAR_MAY_DAY1) {
                if (dayOfYear < DAY_OF_YEAR_APR_DAY1) {
                    *month = MONTH_OFFSET + 2; *day = DAY_OFFSET - DAY_OF_YEAR_MAR_DAY1 + dayOfYear;
                }
                else {
                    *month = MONTH_OFFSET + 3; *day = DAY_OFFSET - DAY_OF_YEAR_APR_DAY1 + dayOfYear;
                }
            }
            else {
                if (dayOfYear < DAY_OF_YEAR_JUN_DAY1) {
                    *month = MONTH_OFFSET + 4; *day = DAY_OFFSET - DAY_OF_YEAR_MAY_DAY1 + dayOfYear;
                }
                else if (dayOfYear < DAY_OF_YEAR_JUL_DAY1) {
                    *month = MONTH_OFFSET + 5; *day = DAY_OFFSET - DAY_OF_YEAR_JUN_DAY1 + dayOfYear;
                }
                else {
                    *month = MONTH_OFFSET + 6; *day = DAY_OFFSET - DAY_OF_YEAR_JUL_DAY1 + dayOfYear;
                }
            }
        }
        else {
            if (dayOfYear < DAY_OF_YEAR_OCT_DAY1) {
                if (dayOfYear < DAY_OF_YEAR_SEP_DAY1) {
                    *month = MONTH_OFFSET + 7; *day = DAY_OFFSET - DAY_OF_YEAR_AUG_DAY1 + dayOfYear;
                }
                else {
                    *month = MONTH_OFFSET + 8; *day = DAY_OFFSET - DAY_OF_YEAR_SEP_DAY1 + dayOfYear;
                }
            }
            else {
                if (dayOfYear < DAY_OF_YEAR_NOV_DAY1) {
                    *month = MONTH_OFFSET + 9; *day = DAY_OFFSET - DAY_OF_YEAR_OCT_DAY1 + dayOfYear;
                }
                else if (dayOfYear < DAY_OF_YEAR_DEC_DAY1) {
                    *month = MONTH_OFFSET + 10; *day = DAY_OFFSET - DAY_OF_YEAR_NOV_DAY1 + dayOfYear;
                }
                else {
                    *month = MONTH_OFFSET + 11; *day = DAY_OFFSET - DAY_OF_YEAR_DEC_DAY1 + dayOfYear;
                }
            }
        }
    }

    return true;
}

}
