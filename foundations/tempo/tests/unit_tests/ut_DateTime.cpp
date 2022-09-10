#include <gtest/gtest.h>
#include "time/GDateTime.h"
#include "fortress/time/GStopwatchTimer.h"

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(DateTimeTest, TimeDifference) {

    Time time1{ 1, 2, 3, 12000 };
    Time time2{ 1, 2, 3, 3000 };
    Date date1{ 2022, 11, 12 };
    Date date2{ 2022, 11, 13 };

    DateTime dt1{ time1, date1 };
    DateTime dt2{ time2, date1 };

    Float64_t timeDiffMs = dt1.getTimeToMs(dt2);

    EXPECT_EQ(timeDiffMs, -9);
}

TEST(DateTimeTest, TimingTimeTest) {

    Time time1{ 1, 2, 3, 12000 };
    Date date1{ 2022, 11, 12 };
    DateTime dt1{ time1, date1 };

    //StopwatchTimer timer;
    //const Int32_t numSteps = 1000000;
    //timer.start();
    //for (Uint32_t i = 0; i < numSteps; i++) {
    //    dt1.daysFromCivil(2000, 11, 12);
    //}
    //timer.stop();

    //double numDays = dt1.daysFromCivil(1970, 1, 4);
    //EXPECT_EQ(numDays, 3);
}



TEST(DateTimeTest, FormatString) {

    Time time1{ 1, 2, 3, 12000 };
    Date date1{ 2022, 11, 12 };

    DateTime dt1{ time1, date1 };

    std::string formattedString = dt1.toString();

    EXPECT_EQ(formattedString, "2022-11-12 01:02:03.012");
}

TEST(DateTimeTest, Now) {

    DateTime now = DateTime::Now();
    std::time_t now2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char* cNow2 = std::asctime(std::gmtime(&now2));

    // Format datetime to match std::asctime output, but remove millisecond string
    GString now1Str = now.toString("%a %b %e %H:%M:%S %Y\n");
    now1Str.replace(".000", "");

    EXPECT_EQ(now1Str, cNow2);
}

} /// End rev namespace