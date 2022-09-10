#pragma once

#include "fortress/numeric/GSizedTypes.h"

namespace rev {

/// @brief DateTime class
/// @detailed Class for handling dates and times
/// @todo Implement as Date and Time classes.
class Date {
public:

    Date();

    template<typename YearType, typename ShortType>
    Date(YearType year, ShortType month, ShortType day) :
        m_year(Uint16_t(year)),
        m_month(Uint8_t(month)),
        m_day(Uint8_t(day))
    {
    }

	~Date();

protected:
    friend class DateTime;

    Uint16_t m_year{ Uint16_t(1970) };
    Uint8_t m_month{ Uint8_t(1) }; ///< From 1-12
    Uint8_t m_day{ Uint8_t(1) }; ///< From 1 until a max of 31
};



} // End rev namespaces
