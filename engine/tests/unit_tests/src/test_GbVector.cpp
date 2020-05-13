///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "test_GbVector.h"
#include "../../grand_blue/src/core/geometry/GbVector.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestVector::asJson()
{
    auto vec = Gb::Vector<float, 9>({ 20, 10, 2, 1, 3, 45, 610, 7, 5 });
    QJsonArray array = vec.asJson().toArray();

    QVERIFY(array[0].toInt() == 20);
    QVERIFY(array[1].toInt() == 10);
    QVERIFY(array[2].toInt() == 2);
    QVERIFY(array[3].toInt() == 1);
    QVERIFY(array[4].toInt() == 3);
    QVERIFY(array[5].toInt() == 45);
}

void TestVector::loadFromJson()
{
    auto vec = Gb::Vector<float, 9>({ 20, 10, 2, 1, 3, 45, 610, 7, 5 });
    QJsonArray array = vec.asJson().toArray();
    vec = Gb::Vector<float, 9>();
    vec.loadFromJson(array);
    QVERIFY(vec[0] == 20);
    QVERIFY(vec[1] == 10);
    QVERIFY(vec[2] == 2);
    QVERIFY(vec[3] == 1);
    QVERIFY(vec[4] == 3);
    QVERIFY(vec[5] == 45);
    QVERIFY(vec[6] == 610);
}

void TestVector::construction()
{
    auto vec = Gb::Vector<float, 9>( {20, 10, 2, 1, 3, 45, 610, 7, 5});
}

void TestVector::index()
{
    auto vec = Gb::Vector<float, 9>({ 20, 10, 2, 1, 3, 45, 610, 7, 5 });
    std::vector<double> testVec = { 20, 10, 2, 1, 3, 45, 610, 7, 5 };

    for (int i = 0; i < vec.size(); i++) {
        QVERIFY(vec[i] == testVec[i]);
    }
}

void TestVector::assignment()
{
    auto a = Gb::Vector<float, 9>({ 20, 10, 2, 1, 3, 45, 610, 7, 5 });
    auto b = Gb::Vector<float, 9>({35, 45, 20});

    a = b;

    QVERIFY(a == b);
}

void TestVector::addition()
{
    auto a = Gb::Vector<float, 3>({ 20, 10, 2});
    auto b = Gb::Vector<float, 3>({ 7, 1, 8 });

    auto c = Gb::Vector<float, 3>({ 27, 11, 10});

    QVERIFY(a + b == c);

    a += b;
    QVERIFY(a == c);
}

void TestVector::subraction()
{
    auto a = Gb::Vector<float, 3>({ 20, 10, 2 });
    auto b = Gb::Vector<float, 3>({ 7, 1, 8 });

    auto c = Gb::Vector<float, 3>({ 13, 9, -6 });

    QVERIFY(a - b == c);

    a -= b;
    QVERIFY(a == c);
}

void TestVector::multiplication()
{
    auto a = Gb::Vector<float, 3>({ 20, 10, 2 });
    auto b = Gb::Vector<float, 3>({ 7, 1, 8 });

    auto c = Gb::Vector<float, 3>({ 140, 10, 16 });

    QVERIFY(a * b == c);

    a *= b;
    QVERIFY(a == c);

    auto d = Gb::Vector<float, 3>({ -140, -10, -16 });
    QVERIFY(-c == d);
}

void TestVector::division()
{
    auto a = Gb::Vector<float, 3>({ 20, 10, 2 });
    double b = 2;

    auto c = Gb::Vector<float, 3>({ 10, 5, 1 });

    QVERIFY(a / b == c);
}

void TestVector::dot()
{
    auto a = Gb::Vector<float, 3>({ 20, 10, 2 });
    auto b = Gb::Vector<float, 3>({ 7, 1, 8 });

    double c = 166;

    QVERIFY(a.dot(b) == c);
}

void TestVector::length()
{
    auto a = Gb::Vector<float, 3>({ 3, 4, 5 });

    QVERIFY(a.length() == sqrt(50.0f));
}
