#include <gtest/gtest.h>

#include "fortress/containers/GColor.h"
#include "fortress/containers/math/GVector.h"

namespace rev {

TEST(ColorTest, Construction) {
    std::vector<Real_t> vec = { 1.0, 0.5, 0.25, 0.0 };
    std::vector<int> vec2 = { 100, 200, 215, 75 };
    Vector3 vec3(0.5, 0.2, 0.1);
    Vector4 vec4(0.5, 0.2, 0.1, 0.3);

    Color color1;
    Color color2(vec);
    Color color3(vec2);
    Color color4(vec3);
    Color color5(vec4);
    Color color6(1, 2, 3, 4);


    EXPECT_EQ(color1.redF(), color1.greenF());
    EXPECT_EQ(color1.blueF(), color1.alphaF());
    EXPECT_EQ(color1.alphaF(), 0.0F);
    EXPECT_EQ(color2.redF(), vec[0]);
    EXPECT_EQ(color2.greenF(), vec[1]);
    EXPECT_EQ(color2.blueF(), vec[2]);
    EXPECT_EQ(color2.alphaF(), vec[3]);
    EXPECT_EQ(color3.red(), vec2[0]);
    EXPECT_EQ(color3.green(), vec2[1]);
    EXPECT_EQ(color3.blue(), vec2[2]);
    EXPECT_EQ(color3.alpha(), vec2[3]);
    EXPECT_NE(color4, color5);
    EXPECT_EQ(color6.red(), 1);
    EXPECT_EQ(color6.green(), 2);
    EXPECT_EQ(color6.blue(), 3);
    EXPECT_EQ(color6.alpha(), 4);
}

TEST(ColorTest, ToVector) {
    Color color1(1, 2, 3, 4);
    Color color2((Real_t)0.1, (Real_t)0.2, (Real_t)0.3, (Real_t)0.4);
    Vector4 vec = color2.toVector<Real_t, 4>();
    std::vector<int> vec2 = color1.toStdVector<int>();
    std::vector<int> vec3{1, 2, 3, 4};
    EXPECT_EQ(vec, Vector4(0.1, 0.2, 0.3, 0.4));
    EXPECT_EQ(vec2, vec3);
}

} // End rev namespace
