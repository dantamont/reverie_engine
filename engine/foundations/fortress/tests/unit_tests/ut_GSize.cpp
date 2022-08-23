#include <gtest/gtest.h>

#include "fortress/containers/math/GSize.h"
#include "fortress/containers/math/GVector.h"

namespace rev {

TEST(GSizeTest, Construction) {
    GSize size;
    GSize size2(100, 200);

    EXPECT_EQ(size.width(), -1);
    EXPECT_EQ(size.height(), -1);
    EXPECT_EQ(size.isNull(), false);
    EXPECT_EQ(size.isEmpty(), true);

    EXPECT_EQ(size2.width(), 100);
    EXPECT_EQ(size2.height(), 200);
    EXPECT_EQ(size2.isNull(), false);
    EXPECT_EQ(size2.isEmpty(), false);
}

TEST(GSizeTest, ToVector2) {
    GSize size(100, 200);
    Vector2i vec = size.toVector2();

    EXPECT_EQ(vec[0], size.width());
    EXPECT_EQ(vec[1], size.height());
}

} // End rev namespace
