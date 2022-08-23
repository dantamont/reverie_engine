#pragma once

#include <type_traits>

#include "fortress/types/GSizedTypes.h"

namespace rev {

/// @brief DateTime class
/// @detailed Class for handling dates and times
/// @todo Implement as Date and Time classes.
class Time {
public:

    Time();

    template<typename ShortType, typename LongType>
    Time(ShortType hour, ShortType minute, ShortType second, LongType microSeconds):
        m_hour(static_cast<Uint8_t>(hour)),
        m_minute(static_cast<Uint8_t>(minute)),
        m_second(static_cast<Uint8_t>(second)),
        m_microSeconds(static_cast<Uint32_t>(microSeconds))
    {
        static_assert(std::is_arithmetic_v<ShortType> && std::is_arithmetic_v<LongType>, 
            "Arithmetic type required");
    }

	~Time();

    Float64_t getFractionalSeconds() const;

protected:
    friend class DateTime;

    Uint8_t m_hour{ Uint8_t(0) };   ///< The hour since midnight (0-23)
    Uint8_t m_minute{ Uint8_t(0) }; ///< The minute (0-59)
    Uint8_t m_second{ Uint8_t(0) }; ///< The second (0-60), to allow for a positive leap second
    Uint32_t m_microSeconds{ 0 }; ///< The number of micro-seconds
};



} // End rev namespaces
