#include <gtest/gtest.h>

#include "fortress/numeric/GFloat16.h"

namespace rev {

TEST(Float16, Conversions) {
    Float16_t myFloat = 1.1;
    Float32_t diff = abs(Float32_t(myFloat) - 1.1);
    EXPECT_LE(diff, 1e-3);
    EXPECT_EQ((Int32_t)myFloat, 1);
}

} // End rev namespace
