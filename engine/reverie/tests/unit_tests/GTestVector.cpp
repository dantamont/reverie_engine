///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GTestVector.h"
#include <src/core/geometry/GVector.h>

namespace rev{
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void VectorTest::asJson()
{
    auto vec = rev::Vector<float, 9>({ 20, 10, 2, 1, 3, 45, 610, 7, 5 });
    QJsonArray array = vec.asJson().toArray();

    assert_(array[0].toInt() == 20);
    assert_(array[1].toInt() == 10);
    assert_(array[2].toInt() == 2);
    assert_(array[3].toInt() == 1);
    assert_(array[4].toInt() == 3);
    assert_(array[5].toInt() == 45);
}

void VectorTest::loadFromJson()
{
    auto vec = rev::Vector<float, 9>({ 20, 10, 2, 1, 3, 45, 610, 7, 5 });
    QJsonArray array = vec.asJson().toArray();
    vec = rev::Vector<float, 9>();
    vec.loadFromJson(array);
    assert_(vec[0] == 20);
    assert_(vec[1] == 10);
    assert_(vec[2] == 2);
    assert_(vec[3] == 1);
    assert_(vec[4] == 3);
    assert_(vec[5] == 45);
    assert_(vec[6] == 610);
}

void VectorTest::construction()
{
    auto vec = rev::Vector<float, 9>( {20, 10, 2, 1, 3, 45, 610, 7, 5});
}

void VectorTest::index()
{
    auto vec = rev::Vector<float, 9>({ 20, 10, 2, 1, 3, 45, 610, 7, 5 });
    std::vector<double> testVec = { 20, 10, 2, 1, 3, 45, 610, 7, 5 };

    for (int i = 0; i < vec.size(); i++) {
        assert_(vec[i] == testVec[i]);
    }
}

void VectorTest::assignment()
{
    auto a = rev::Vector<float, 9>({ 20, 10, 2, 1, 3, 45, 610, 7, 5 });
    auto b = rev::Vector<float, 9>({35, 45, 20});

    a = b;

    assert_(a == b);
}

void VectorTest::addition()
{
    auto a = rev::Vector<float, 3>({ 20, 10, 2});
    auto b = rev::Vector<float, 3>({ 7, 1, 8 });

    auto c = rev::Vector<float, 3>({ 27, 11, 10});

    assert_(a + b == c);

    a += b;
    assert_(a == c);
}

void VectorTest::subraction()
{
    auto a = rev::Vector<float, 3>({ 20, 10, 2 });
    auto b = rev::Vector<float, 3>({ 7, 1, 8 });

    auto c = rev::Vector<float, 3>({ 13, 9, -6 });

    assert_(a - b == c);

    a -= b;
    assert_(a == c);
}

void VectorTest::multiplication()
{
    auto a = rev::Vector<float, 3>({ 20, 10, 2 });
    auto b = rev::Vector<float, 3>({ 7, 1, 8 });

    auto c = rev::Vector<float, 3>({ 140, 10, 16 });

    assert_(a * b == c);

    a *= b;
    assert_(a == c);

    auto d = rev::Vector<float, 3>({ -140, -10, -16 });
    assert_(-c == d);
}

void VectorTest::division()
{
    auto a = rev::Vector<float, 3>({ 20, 10, 2 });
    double b = 2;

    auto c = rev::Vector<float, 3>({ 10, 5, 1 });

    assert_(a / b == c);
}

void VectorTest::dot()
{
    auto a = rev::Vector<float, 3>({ 20, 10, 2 });
    auto b = rev::Vector<float, 3>({ 7, 1, 8 });

    double c = 166;

    assert_(a.dot(b) == c);
}

void VectorTest::length()
{
    auto a = rev::Vector<float, 3>({ 3, 4, 5 });

    assert_(a.length() == sqrt(50.0f));
}

void rev::VectorTest::perform()
{
    asJson();
    loadFromJson();
    construction();
    index();
    assignment();
    addition();
    subraction();
    multiplication();
    division();
    dot();
    length();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
