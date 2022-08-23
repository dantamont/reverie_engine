#include <gtest/gtest.h>

#include "fortress/math/GMath.h"
#include "fortress/time/GStopwatchTimer.h"

namespace rev {

namespace math {
    /// @note This is obsolete, as modern compilers make sqrt very fast :,)
    /// @brief A fast, floating-point or double precision inverse square root
    /// @see http://rrrola.wz.cz/inv_sqrt.html
    /// @see For the double-precision magic number, see https://cs.uwaterloo.ca/~m32rober/rsqrt.pdf
    template<class T>
    T invSqrt(T number) {
        if constexpr (std::is_same_v<T, float>)
        {
            union { float f; uint32_t u; } y = { number };
            y.u = 0x5F1FFFF9ul - (y.u >> 1);
            return 0.703952253f * y.f * (2.38924456f - number * y.f * y.f);
        }
        else if constexpr (std::is_same_v<T, double>) {
            double y = number;
            double x2 = y * 0.5;
            std::int64_t i;
            memcpy(&i, &y, sizeof(double));
            i = 0x5fe6eb50c7b537a9 - (i >> 1); // the magic number
            memcpy(&y, &i, sizeof(double));
            y = y * (1.5 - (x2 * y * y));   // 1st iteration
            //      y  = y * ( 1.5 - ( x2 * y * y ) );   // 2nd iteration, this can be removed
            return y;
        }
        else {
            invSqrt((float)number);
        }
    }
}


TEST(MathTests, InverseSqrt) {

    // Calculate average error
    static constexpr uint32_t numErrorRuns = 1e5;
    static constexpr uint32_t numTimedRuns = 1e6;
    std::vector<float> expectedRuns;
    expectedRuns.reserve(numTimedRuns);
    std::vector<float> actualRuns;
    actualRuns.reserve(numTimedRuns);

    StopwatchTimer timer1;
    StopwatchTimer timer2;

    timer1.start();
    for (uint32_t i = 1; i < numTimedRuns; i++) {
        float myExpectedFloat = 1.0f / sqrt(float(i));
    }
    float time1 = timer1.getElapsed<float>();

    timer2.start();
    for (uint32_t i = 1; i < numTimedRuns; i++) {
        float myActualFloat = math::invSqrt(float(i));
    }
    float time2 = timer2.getElapsed<float>();

    for (uint32_t i = 1; i < numErrorRuns; i++) {
        float myExpectedFloat = 1.0f / sqrt(float(i));
        expectedRuns.push_back(myExpectedFloat);
    }

    for (uint32_t i = 1; i < numErrorRuns; i++) {
        float myActualFloat = math::invSqrt(float(i));
        actualRuns.push_back(myActualFloat);
    }

    float sum = 0;
    for (uint32_t i = 0; i < actualRuns.size(); i++) {
        float err = (expectedRuns[i] - actualRuns[i]);
        err *= err;
        sum += err;
    }
    float err = sum / numErrorRuns;

    // Check that values match within a tolerance
    EXPECT_LE(err, 2e-6);

    // Check that custom function is actually faster
    //EXPECT_LE(time2, time1);
}

TEST(MathTests, InverseSqrtDouble) {

    // Calculate average error
    static constexpr uint32_t numRuns = 1e5;
    std::vector<double> expectedRuns;
    expectedRuns.reserve(numRuns);
    std::vector<double> actualRuns;
    actualRuns.reserve(numRuns);

    StopwatchTimer timer1;
    StopwatchTimer timer2;

    timer1.start();
    for (uint32_t i = 1; i < numRuns; i++) {
        double myExpectedFloat = 1.0f / sqrt(double(i));
    }
    float time1 = timer1.getElapsed<float>();

    timer2.start();
    for (uint32_t i = 1; i < numRuns; i++) {
        double myActualFloat = math::invSqrt(double(i));
    }
    float time2 = timer2.getElapsed<float>();

    for (uint32_t i = 1; i < numRuns; i++) {
        double myExpectedFloat = 1.0f / sqrt(double(i));
        expectedRuns.push_back(myExpectedFloat);
    }

    for (uint32_t i = 1; i < numRuns; i++) {
        double myActualFloat = math::invSqrt(double(i));
        actualRuns.push_back(myActualFloat);
    }

    double sum = 0;
    for (uint32_t i = 0; i < actualRuns.size(); i++) {
        double err = (expectedRuns[i] - actualRuns[i]);
        err *= err;
        sum += err;
    }
    double err = sum / numRuns;

    // Check that values match within a tolerance
    EXPECT_LE(err, 2e-7);

    // Check that custom function is actually faster
    //EXPECT_LE(time2, time1);
}

} // End rev namespace
