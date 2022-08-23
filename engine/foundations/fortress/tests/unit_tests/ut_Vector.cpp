#include <gtest/gtest.h>
#include "fortress/containers/math/GVector.h"


// Tests
using namespace rev;


TEST(VectorTest, asJson)
{
    auto vec = rev::Vector<float, 9>({ 20, 10, 2, 1, 3, 45, 610, 7, 5 });
    json array = vec;

    EXPECT_EQ(array[0].get<int>() ,  20);
    EXPECT_EQ(array[1].get<int>() ,  10);
    EXPECT_EQ(array[2].get<int>() ,  2);
    EXPECT_EQ(array[3].get<int>() ,  1);
    EXPECT_EQ(array[4].get<int>() ,  3);
    EXPECT_EQ(array[5].get<int>() ,  45);
}

TEST(VectorTest, loadFromJson)
{
    auto vec = rev::Vector<float, 9>({ 20, 10, 2, 1, 3, 45, 610, 7, 5 });
    json array = vec;
    vec = rev::Vector<float, 9>();
    array.get_to(vec);
    EXPECT_EQ(vec[0] ,  20);
    EXPECT_EQ(vec[1] ,  10);
    EXPECT_EQ(vec[2] ,  2);
    EXPECT_EQ(vec[3] ,  1);
    EXPECT_EQ(vec[4] ,  3);
    EXPECT_EQ(vec[5] ,  45);
    EXPECT_EQ(vec[6] ,  610);

    Vector2 myVec2(3, 4);
    json myJson2 = myVec2;
    Vector2 myVec2FromJson = myJson2;
    EXPECT_EQ(myJson2[0], 3);
    EXPECT_EQ(myJson2[1], 4);
    EXPECT_EQ(myVec2, myVec2FromJson);
}

TEST(VectorTest, construction)
{
    auto vec = rev::Vector<float, 9>({ 20, 10, 2, 1, 3, 45, 610, 7, 5 });

    Vector3f myVec(Vector2f(1, 2), 3.F);
    EXPECT_EQ(myVec, Vector3f(1, 2, 3));

    Vector4f myVec2(Vector2f(2, 3), 4.F, 5.F);
    EXPECT_EQ(myVec2, Vector4f(2, 3, 4.F, 5.F));

    Vector4f myVec3(Vector3f(2, 3, 4), 5.F);
    EXPECT_EQ(myVec3, Vector4f(2, 3, 4, 5.F));
}

TEST(VectorTest, index)
{
    auto vec = rev::Vector<float, 9>({ 20, 10, 2, 1, 3, 45, 610, 7, 5 });
    std::vector<double> testVec = { 20, 10, 2, 1, 3, 45, 610, 7, 5 };

    for (int i = 0; i < vec.size(); i++) {
        EXPECT_EQ(vec[i] ,  testVec[i]);
    }
}

TEST(VectorTest, assignment)
{
    auto a = rev::Vector<float, 9>({ 20, 10, 2, 1, 3, 45, 610, 7, 5 });
    auto b = rev::Vector<float, 9>({ 35, 45, 20 });

    a = b;

    EXPECT_EQ(a ,  b);
}

TEST(VectorTest, addition)
{
    auto a = rev::Vector<float, 3>({ 20, 10, 2 });
    auto b = rev::Vector<float, 3>({ 7, 1, 8 });

    auto c = rev::Vector<float, 3>({ 27, 11, 10 });

    EXPECT_EQ(a + b ,  c);

    a += b;
    EXPECT_EQ(a ,  c);
}

TEST(VectorTest, subraction)
{
    auto a = rev::Vector<float, 3>({ 20, 10, 2 });
    auto b = rev::Vector<float, 3>({ 7, 1, 8 });

    auto c = rev::Vector<float, 3>({ 13, 9, -6 });

    EXPECT_EQ(a - b ,  c);

    a -= b;
    EXPECT_EQ(a ,  c);
}

TEST(VectorTest, multiplication)
{
    auto a = rev::Vector<float, 3>({ 20, 10, 2 });
    auto b = rev::Vector<float, 3>({ 7, 1, 8 });

    auto c = rev::Vector<float, 3>({ 140, 10, 16 });

    EXPECT_EQ(a * b ,  c);

    a *= b;
    EXPECT_EQ(a ,  c);

    auto d = rev::Vector<float, 3>({ -140, -10, -16 });
    EXPECT_EQ(-c ,  d);
}

TEST(VectorTest, division)
{
    auto a = rev::Vector<float, 3>({ 20, 10, 2 });
    double b = 2;

    auto c = rev::Vector<float, 3>({ 10, 5, 1 });

    EXPECT_EQ(a / b ,  c);
}

TEST(VectorTest, dot)
{
    auto a = rev::Vector<float, 3>({ 20, 10, 2 });
    auto b = rev::Vector<float, 3>({ 7, 1, 8 });

    double c = 166;

    EXPECT_EQ(a.dot(b) ,  c);
}

TEST(VectorTest, length)
{
    auto a = rev::Vector<float, 3>({ 3, 4, 5 });

    EXPECT_EQ(a.length() ,  sqrt(50.0f));
}

