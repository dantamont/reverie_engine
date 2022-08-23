#include <gtest/gtest.h>
#include "fortress/types/GSizedTypes.h"
#include "fortress/conversions/GUnits.h"
#include "fortress/conversions/GQuantity.h"
#include "fortress/math/GMath.h"

namespace rev {

TEST(UnitsTest, Conversions) {

    // Test a few unit conversions as a proof of concept
    // TODO: Test all possible combinations of units
    size_t numSeconds = Units::ConvertToSI<TimeUnits::kDays>(1);
    EXPECT_EQ(numSeconds, 86400);

    size_t numMilliSeconds = Units::Convert<TimeUnits::kDays, TimeUnits::kMilliseconds>(1);
    EXPECT_EQ(numMilliSeconds, 86400000);

    size_t numMicroSeconds = Units::Convert<TimeUnits::kDays, TimeUnits::kMicroseconds>(size_t(1));
    EXPECT_EQ(numMicroSeconds, 86400000000);

    size_t numMinutes = Units::Convert<TimeUnits::kDays, TimeUnits::kMinutes>(1);
    EXPECT_EQ(numMinutes, 1440);

    //size_t numMinutes1 = Units::Convert_impl<TimeUnits, TimeUnits::kSeconds, TimeUnits::kMinutes>(60);
    //assert_(numMinutes1== 1);

    //size_t numMinutes1 = Units::Convert<TimeUnits::kSeconds, AngularUnits::kDegrees>(60);

    size_t numMinutes2 = Units::Convert<TimeUnits::kSeconds, TimeUnits::kMinutes>(60);
    EXPECT_EQ(numMinutes2, 1);

    /// @note There are precision issues with compile-time multiplication if the desired output is an integer
    /// For example, the below will fail:
    //size_t numDays = Units::Convert<TimeUnits::kMinutes, TimeUnits::kDays>(2880);
    //EXPECT_EQ(numDays, 2);

    double numDays = Units::Convert<TimeUnits::kMinutes, TimeUnits::kDays>(2880);
    EXPECT_DOUBLE_EQ(numDays, 2);

    double numDaysd = Units::Convert<TimeUnits::kMinutes, TimeUnits::kDays>(2880.0);
    EXPECT_DOUBLE_EQ(numDaysd, 2);

    double rads = 1.0;
    double degs = Units::Convert<AngularUnits::kRadians, AngularUnits::kDegrees>(rads);
    EXPECT_EQ(math::fuzzyIsNull(degs - 180 / 3.14159265358979323846), true);


    double meters = 100.0;
    bool same = std::is_same_v<DistanceUnits, AngularUnits>;
    EXPECT_EQ(same, false);
    double kmeters = Units::Convert<DistanceUnits::kMeters, DistanceUnits::kKilometers>(meters);
    EXPECT_EQ(math::fuzzyIsNull(kmeters - 0.1), true);


    // Test a Quantity conversion
    //Quantity_impl<TimeUnits, TimeUnits::kSeconds> secsi(60);
    //auto minutesi = secsi.to<TimeUnits, TimeUnits::kMinutes>();
    //assert_(minutesi.value() == 1.0);

    Quantity<TimeUnits::kSeconds> secs(60);
    auto timeUnits = secs.units();
    auto minutes = secs.to<TimeUnits::kMinutes>();
    EXPECT_EQ(minutes.value(),  1.0);

}


} /// End rev namespace
