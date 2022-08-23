#include <gtest/gtest.h>

#include <thread>
#include "fortress/time/GSleep.h"
#include "fortress/time/GStopwatchTimer.h"
#include "fortress/time/GExpireTimer.h"

namespace rev {

TEST(SleepTests, SleepFor) {
    // Create and run timer
    StopwatchTimer timer;

    // Sleep for half a second and verify that time elapsed
    timer.start();
    double timeToWaitSec = 0.5;
    Sleep::SleepFor(timeToWaitSec * 1e6);
    double elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    // Run timer for 5ms and verify accuracy
    timeToWaitSec = 5e-3;
    timer.restart();
    Sleep::SleepFor(timeToWaitSec * 1e6);
    elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    double err = abs((elapsedTime / timeToWaitSec) - 1.0);
    EXPECT_LE(err, 0.1);

    // Run timer for 1ms and verify accuracy
    timeToWaitSec = 1e-3;
    timer.restart();
    Sleep::SleepFor(timeToWaitSec * 1e6);
    elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    err = abs((elapsedTime / timeToWaitSec) - 1.0);
    EXPECT_LE(err, 0.1);

    // Run timer for 0.1ms and verify accuracy
    timeToWaitSec = 1e-4;
    timer.restart();
    Sleep::SleepFor(timeToWaitSec * 1e6);
    elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    err = abs((elapsedTime / timeToWaitSec) - 1.0);
    EXPECT_LE(err, 0.1);
}

TEST(SleepTests, PreciseSleepUs) {
    // Create and run timer
    StopwatchTimer timer;

    // Sleep for half a second and verify that time elapsed
    timer.start();
    double timeToWaitSec = 0.5;
    Sleep::PreciseSleepUs(timeToWaitSec * 1e6);
    double elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    // Run timer for 5ms and verify accuracy
    timeToWaitSec = 5e-3;
    timer.restart();
    Sleep::PreciseSleepUs(timeToWaitSec * 1e6);
    elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    double err = abs((elapsedTime / timeToWaitSec) - 1.0);
    EXPECT_LE(err, 0.1);

    // Run timer for 1ms and verify accuracy
    timeToWaitSec = 1e-3;
    timer.restart();
    Sleep::PreciseSleepUs(timeToWaitSec * 1e6);
    elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    err = abs((elapsedTime / timeToWaitSec) - 1.0);
    EXPECT_LE(err, 0.1);

    // Run timer for 0.1ms and verify accuracy
    timeToWaitSec = 1e-4;
    timer.restart();
    Sleep::PreciseSleepUs(timeToWaitSec * 1e6);
    elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    err = abs((elapsedTime / timeToWaitSec) - 1.0);
    EXPECT_LE(err, 0.1);
}

TEST(SleepTests, SleepInMs) {
    // Create and run timer
    StopwatchTimer timer;

    // Sleep for half a second and verify that time elapsed
    timer.start();
    double timeToWaitSec = 0.5;
    Sleep::SleepInMs(timeToWaitSec * 1e3);
    double elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    // Run timer for 5ms and verify accuracy
    timeToWaitSec = 5e-3;
    timer.restart();
    Sleep::SleepInMs(timeToWaitSec * 1e3);
    elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    double err = abs((elapsedTime / timeToWaitSec) - 1.0);
    EXPECT_LE(err, 0.1);

    // Run timer for 1ms and verify accuracy
    timeToWaitSec = 1e-3;
    timer.restart();
    Sleep::SleepInMs(timeToWaitSec * 1e3);
    elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    err = abs((elapsedTime / timeToWaitSec) - 1.0);
    EXPECT_LE(err, 0.1);

    // Run timer for 0.1ms and verify accuracy
    timeToWaitSec = 1e-3;
    timer.restart();
    Sleep::SleepInMs(timeToWaitSec * 1e3);
    elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    err = abs((elapsedTime / timeToWaitSec) - 1.0);
    EXPECT_LE(err, 0.1);
}

TEST(SleepTests, SleepInUs) {
    // Create and run timer
    StopwatchTimer timer;

    // Sleep for half a second and verify that time elapsed
    timer.start();
    double timeToWaitSec = 0.5;
    Sleep::SleepInUs(timeToWaitSec * 1e6);
    double elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    // Run timer for 5ms and verify accuracy
    timeToWaitSec = 5e-3;
    timer.restart();
    Sleep::SleepInUs(timeToWaitSec * 1e6);
    elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    double err = abs((elapsedTime / timeToWaitSec) - 1.0);
    EXPECT_LE(err, 0.1);

    // Run timer for 1ms and verify accuracy
    timeToWaitSec = 1e-3;
    timer.restart();
    Sleep::SleepInUs(timeToWaitSec * 1e6);
    elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    err = abs((elapsedTime / timeToWaitSec) - 1.0);
    EXPECT_LE(err, 0.1);

    // Run timer for 0.1ms and verify accuracy
    timeToWaitSec = 1e-4;
    timer.restart();
    Sleep::SleepInUs(timeToWaitSec * 1e6);
    elapsedTime = timer.getElapsed<double>();
    EXPECT_GE(elapsedTime, timeToWaitSec);

    err = abs((elapsedTime / timeToWaitSec) - 1.0);
    EXPECT_LE(err, 0.1);
}


} /// End rev namespace