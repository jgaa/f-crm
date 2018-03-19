#include "src/utility.h"


time_t ToTime(const QDate &date)
{
    struct std::tm tm = {};

    tm.tm_year = date.year() - 1900;
    tm.tm_mon = date.month() -1;
    tm.tm_mday = date.day();

    return std::mktime(&tm);
}
