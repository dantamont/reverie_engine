#include "time/GDateTime.h"
#include "time/extern/date/date.h"
#include "units/GUnits.h"

#include <ctime>

namespace rev {

DateTime DateTime::Now()
{
    return DateTime::FromTimePoint(std::chrono::system_clock::now());
}

DateTime DateTime::FromTimePoint(const TimePoint& tp)
{
    std::tm myTm;
    TimePointToTm(tp, myTm);
    return DateTime(Time(myTm.tm_hour, myTm.tm_min, myTm.tm_sec, 0), Date(myTm.tm_year + 1900, myTm.tm_mon + 1, myTm.tm_mday));
}

DateTime::DateTime()
{
}

DateTime::DateTime(const Time& time, const Date& date):
    m_time(time),
    m_date(date)
{
}

DateTime::~DateTime()
{
}

Float64_t DateTime::getTimeToMs(const DateTime& other) const
{
    // Get a duration
    TimePoint finish = other.toTimePoint();
    TimePoint start = toTimePoint();
    std::chrono::duration<double, std::micro> elapsed = finish - start;
    return Units::Convert<TimeUnits::kMicroseconds, TimeUnits::kMilliseconds>(elapsed.count());
}

std::string DateTime::toString(const char* fmt) const
{
    return date::format(fmt, std::chrono::time_point_cast<std::chrono::milliseconds>(toTimePoint()));
}

DateTime::TimePoint DateTime::toTimePoint() const
{
    static Int32_t s_monthOffset = -1;
    static Int32_t s_yearOffset = -1900;
    std::tm tm = {
        m_time.m_second,
        m_time.m_minute,
        m_time.m_hour,
        m_date.m_day,
        m_date.m_month + s_monthOffset,
        m_date.m_year + s_yearOffset,
    };
    tm.tm_isdst = -1; // Use DST value from local time zone
    TimePoint tp = std::chrono::system_clock::from_time_t(MakeGmTime(&tm)); // Get UTC time, std::mktime is local
    tp += std::chrono::microseconds(m_time.m_microSeconds);
    return tp;
}

time_t DateTime::MakeGmTime(std::tm const* t)
{
    Int32_t year = t->tm_year + 1900;
    Int32_t month = t->tm_mon; // 0-11
    if (month > 11)
    {
        year += month / 12;
        month %= 12;
    }
    else if (month < 0)
    {
        int years_diff = (11 - month) / 12;
        year -= years_diff;
        month += 12 * years_diff;
    }
    Int32_t days_since_epoch = DaysFromCivil(year, month + 1, t->tm_mday);

    return 60 * (60 * (24L * days_since_epoch + t->tm_hour) + t->tm_min) + t->tm_sec;
}





// End namespaces        
}