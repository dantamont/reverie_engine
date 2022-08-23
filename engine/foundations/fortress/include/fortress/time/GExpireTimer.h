#pragma once

// Internal
#include "fortress/time/GStopwatchTimer.h"

namespace rev {

/// @brief Timer for checking if a certain amount of time has elapsed
class ExpireTimer: public StopwatchTimer {
public:

    ExpireTimer(Uint64_t expireTimeUs);

    /// @brief Return the time until expiration
    /// @return The time until expiration. Will be negative if expired
    Int64_t timeUntilExpirationMicroseconds() const;

    /// @brief Return the expire time of the itmer
    Uint64_t expireTimeMicroseconds() const { return m_expireTimeMicroseconds; }

    /// @brief Return whether or not the timer is expired
    bool isExpired() const { return getElapsedMicroseconds() >= m_expireTimeMicroseconds; }

    /// @brief Wait until expired
    void waitUntilExpired() const;

private:

    Uint64_t m_expireTimeMicroseconds{ 0 }; ///< The expiration time in microseconds
};


} // End rev namespaces
