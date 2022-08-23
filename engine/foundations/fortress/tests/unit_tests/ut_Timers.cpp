#include <gtest/gtest.h>

#include <thread>
#include "fortress/time/GStopwatchTimer.h"
#include "fortress/time/GExpireTimer.h"

namespace rev {

TEST(TimerTests, StopwatchTimePasses) {
    // Create and run timer
    StopwatchTimer timer;
    timer.start();

    // Run timer for a second to confirm that it works
    Uint32_t timeToWait = 1;
    std::this_thread::sleep_for(std::chrono::seconds(timeToWait));
    EXPECT_GE(timer.getElapsed<double>(), timeToWait);
}

TEST(TimerTests, ExpireTimerExpires) {
    // Create and run timer
    Uint64_t expireTime = 1e6;
    ExpireTimer timer(expireTime);
    timer.start();

    // Run timer for a second to confirm that it works
    Uint64_t timeToWait = expireTime;
    timer.waitUntilExpired();
    EXPECT_GE(timer.getElapsedMicroseconds(), timeToWait);
}

} /// End rev namespace