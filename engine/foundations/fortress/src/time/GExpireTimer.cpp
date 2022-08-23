#include "fortress/time/GExpireTimer.h"
#include "fortress/time/GSleep.h"
#include <sstream>
#include <thread>

namespace rev {

ExpireTimer::ExpireTimer(Uint64_t expireTimeUs):
    StopwatchTimer(),
    m_expireTimeMicroseconds(expireTimeUs)
{
    assert(expireTimeUs < std::numeric_limits<Uint32_t>::max() && "SleepInUs does not support Uint64_t");
}

Int64_t ExpireTimer::timeUntilExpirationMicroseconds() const
{
    return Int64_t(m_expireTimeMicroseconds) - Int64_t(getElapsedMicroseconds());
}

void ExpireTimer::waitUntilExpired() const
{
    //Sleep::BusySleep(std::chrono::microseconds(timeUntilExpirationMicroseconds()));
    if (0 != m_expireTimeMicroseconds) {
        Sleep::PreciseSleepUs(timeUntilExpirationMicroseconds());
    }
}

// End namespaces        
}