#pragma once

#include <chrono>
#include <string>

#include "fortress/types/GSizedTypes.h"
#include "fortress/time/GDate.h"
#include "fortress/time/GTime.h"

namespace rev {

/// @brief DateTime class
/// @detailed Class for handling dates and times
/// @todo Consider storing as just a single double, with seconds since 1970..
class DateTime {
protected:

    typedef std::chrono::system_clock::time_point TimePoint;

public:

    /// @brief Return the current datetime
    /// @return 
    static DateTime Now();

    /// @brief Return datetime from a time point
    static DateTime FromTimePoint(const TimePoint& tp);

    DateTime();
    DateTime(const Time& time, const Date& date);
	~DateTime();

    Float64_t getTimeToMs(const DateTime& other) const;

    /// @brief  Return a formatted string of the datetime
    /// @param[in] fmt The string format, defaults to date and ISO 8601 time format
    /// @see https://www.cplusplus.com/reference/ctime/strftime/
    /// @return the formatted string
    std::string toString(const char* fmt = "%F %T") const;

protected:

    /// @brief  Convert to a chrono time point
    /// @return 
    TimePoint toTimePoint() const;

    /// @brief Convert from a chrono time point to a tm struct;
    template <class Duration>
    static void TimePointToTm(const std::chrono::time_point<std::chrono::system_clock, Duration>& tp, std::tm& tm)
    {
        typedef std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<24>>> days;
        // t is time duration since 1970-01-01
        Duration t = tp.time_since_epoch();
        // d is days since 1970-01-01
        days d = RoundDown<days>(t);
        // t is now time duration since midnight of day d
        t -= d;

        // break d down into year/month/day
        int year;
        unsigned month;
        unsigned day;
        std::tie(year, month, day) = CivilFromDays(d.count());

        // Start filling in the tm with calendar info
        tm = { 0 };
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_wday = WeekdayFromDays(d.count());
        tm.tm_yday = d.count() - DaysFromCivil(year, 1, 1);

        // Fill in the time
        tm.tm_hour = std::chrono::duration_cast<std::chrono::hours>(t).count();
        t -= std::chrono::hours(tm.tm_hour);
        tm.tm_min = std::chrono::duration_cast<std::chrono::minutes>(t).count();
        t -= std::chrono::minutes(tm.tm_min);
        tm.tm_sec = std::chrono::duration_cast<std::chrono::seconds>(t).count();
    }

private:

    /// @brief Returns number of days since civil 1970-01-01.  Negative values indicate
    //    days prior to 1970-01-01.
    /// @details Preconditions:  y-m-d represents a date in the civil (Gregorian) calendar
    //                 m is in [1, 12]
    //                 d is in [1, last_day_of_month(y, m)]
    //                 y is "approximately" in
    //                   [numeric_limits<Int>::min()/366, numeric_limits<Int>::max()/366]
    //                 Exact range of validity is:
    //                 [civil_from_days(numeric_limits<Int>::min()),
    //                  civil_from_days(numeric_limits<Int>::max()-719468)]
    /// @see https://howardhinnant.github.io/date_algorithms.html#days_from_civil
    template <class Int>
    static constexpr Int DaysFromCivil(Int y, unsigned m, unsigned d) noexcept
    {
        static_assert(std::numeric_limits<unsigned>::digits >= 18, "This algorithm has not been ported to a 16 bit unsigned integer");
        static_assert(std::numeric_limits<Int>::digits >= 20, "This algorithm has not been ported to a 16 bit signed integer");
        y -= m <= 2;
        const Int era = (y >= 0 ? y : y - 399) / 400;
        const unsigned yoe = static_cast<unsigned>(y - era * 400);      // [0, 399]
        const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;  // [0, 365]
        const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;         // [0, 146096]
        return era * 146097 + static_cast<Int>(doe) - 719468;
    }

    /// @brief Returns year/month/day triple in civil calendar
    /// @details Preconditions:  z is number of days since 1970-01-01 and is in the range:
    ///                   [numeric_limits<Int>::min(), numeric_limits<Int>::max()-719468].
    template <class Int>
    static constexpr std::tuple<Int, unsigned, unsigned> CivilFromDays(Int z) noexcept
    {
        static_assert(std::numeric_limits<unsigned>::digits >= 18, "This algorithm has not been ported to a 16 bit unsigned integer");
        static_assert(std::numeric_limits<Int>::digits >= 20, "This algorithm has not been ported to a 16 bit signed integer");
        z += 719468;
        const Int era = (z >= 0 ? z : z - 146096) / 146097;
        const unsigned doe = static_cast<unsigned>(z - era * 146097);          // [0, 146096]
        const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;  // [0, 399]
        const Int y = static_cast<Int>(yoe) + era * 400;
        const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);                // [0, 365]
        const unsigned mp = (5 * doy + 2) / 153;                                   // [0, 11]
        const unsigned d = doy - (153 * mp + 2) / 5 + 1;                             // [1, 31]
        const unsigned m = mp < 10 ? mp + 3 : mp - 9;                            // [1, 12]
        return std::tuple<Int, unsigned, unsigned>(y + (m <= 2), m, d);
    }

    /// @brief Returns day of week in civil calendar [0, 6] -> [Sun, Sat]
    /// @details Preconditions:  z is number of days since 1970-01-01 and is in the range:
    ///                   [numeric_limits<Int>::min(), numeric_limits<Int>::max()-4].
    /// @see https://howardhinnant.github.io/date_algorithms.html#weekday_from_days
    template <class Int>
    static constexpr unsigned WeekdayFromDays(Int z) noexcept
    {
        return static_cast<unsigned>(z >= -4 ? (z + 4) % 7 : (z + 5) % 7 + 6);
    }

    /// @brief For converting negative system_clock::time_points, to use in place of a duration_cast
    /// @tparam To The output chrono time duration unit, e.g. days, std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<24>>>
    /// @tparam Rep The arithmetic representation of the given duration. e.g. int, float
    /// @tparam Period A std::ratio representing the tick period of the duration
    /// @param d the duration
    /// @return the rounded down duration
    /// @example If your system_clock::time_point contains a duration of seconds(-1), and you convert that to days, 
    ///       you want days to be -1 (midnight 1969-12-31), not 0 (midnight 1970-01-01).
    ///       // time_point -> tp
    ///       int tp2 = round_down<days>(time_point.time_since_epoch()).count();

    template <class To, class Rep, class Period>
    static To RoundDown(const std::chrono::duration<Rep, Period>& d)
    {
        To t = std::chrono::duration_cast<To>(d);
        if (t > d) {
            --t;
        }
        return t;
    }

    /// @brief Convert a broken-down time structure with UTC time to a simple time representation.
    /// @details This function does not modify broken-down time structure as BSD timegm() does.
    static time_t MakeGmTime(std::tm const* t);

    Date m_date; ///< The date
    Time m_time; ///< The time
};



} // End rev namespaces
