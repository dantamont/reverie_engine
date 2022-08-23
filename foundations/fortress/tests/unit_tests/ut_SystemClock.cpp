#include <gtest/gtest.h>

#include <thread>
#include "fortress/time/GSystemClock.h"

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(SystemClockTest, TimePasses) {
    Uint32_t sleepTime = 100000;
    std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
    std::cout << "Elapsed time: " << SystemClock::GetApplicationTimeMicroseconds() << "\n";

    EXPECT_GE(SystemClock::GetApplicationTimeMicroseconds(), sleepTime);

    Uint64_t timeWhenTestWrittenMicroseconds = 1639884120526 * 1000;
    Uint64_t utcTime = SystemClock::GetUtcTimeMicroseconds();
    EXPECT_GE(utcTime, timeWhenTestWrittenMicroseconds);

}

} /// End rev namespace