#pragma once

#include "fortress/time/GDateTime.h"
#include "fortress/time/GStopwatchTimer.h"

namespace rev {

/// @class SystemClock
/// @brief Class for tracking system time
/// @details For managing system time
class SystemClock {
private:
    using clock = std::chrono::steady_clock; ///< Often equivalent to std::chrono::high_resolution clock

    static const StopwatchTimer s_applicationTimer; ///< Timer that begins on program execution

public:

    /// @brief Get the current time elapsed from Utc, in microseconds
    static Uint64_t GetUtcTimeMicroseconds();

    /// @brief Get elapsed time in microseconds since application start
    static Uint64_t GetApplicationTimeMicroseconds();
};


} // End rev namespaces
