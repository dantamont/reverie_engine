#include <gtest/gtest.h>
#include "fortress/layer/framework/GSingleton.h"
#include "fortress/numeric/GSizedTypes.h"

namespace rev {

/// @brief Test singleton class
class MyTestSingleton : public SingletonInterface<MyTestSingleton> {
public:
    Int32_t m_myInt = 4;

    static Int32_t s_count;

protected:

    MyTestSingleton() {
        s_count++;
    }
};

Int32_t MyTestSingleton::s_count = 0;


class MyTestSingleton2 : public SingletonInterface<MyTestSingleton2, false> {
public:
    Int32_t m_myInt = 7;

    static Int32_t s_count;

protected:

    MyTestSingleton2() {
        s_count++;
    }
};

Int32_t MyTestSingleton2::s_count = 0;


/// Tests
TEST(Singleton, Construction) {
    MyTestSingleton& singleton = MyTestSingleton::Instance();
    singleton = MyTestSingleton::Instance();
    singleton = MyTestSingleton::Instance();
    singleton = MyTestSingleton::Instance();

    EXPECT_EQ(singleton.m_myInt, 4);
    EXPECT_EQ(singleton.s_count, 1);

    // Check that singleton can't be accessed before creation if flagged to require Create()
    bool failed = false;
    try {
        MyTestSingleton2& singleton2 = MyTestSingleton2::Instance();
    }
    catch (std::logic_error& err) {
        failed = true;
    }
    EXPECT_EQ(failed, true);

    MyTestSingleton2::Create();
    MyTestSingleton2& singleton2 = MyTestSingleton2::Instance();

    EXPECT_EQ(singleton2.m_myInt, 7);
    EXPECT_EQ(singleton2.s_count, 1);
}




} /// End rev namespace