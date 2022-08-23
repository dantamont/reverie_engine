#include "fortress/time/GSystemClock.h"

namespace rev {

const StopwatchTimer SystemClock::s_applicationTimer = StopwatchTimer::FromElapsedSeconds(0);

Uint64_t SystemClock::GetUtcTimeMicroseconds()
{
    return std::chrono::duration_cast<std::chrono::duration<Uint64_t, std::micro>>(std::chrono::system_clock::now().time_since_epoch()).count();
}

Uint64_t SystemClock::GetApplicationTimeMicroseconds()
{
    return s_applicationTimer.getElapsedMicroseconds();
}

} // End namespaces        
