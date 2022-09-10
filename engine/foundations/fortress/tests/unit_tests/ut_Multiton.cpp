#include <gtest/gtest.h>
#include "fortress/layer/framework/GMultiton.h"
#include "fortress/numeric/GSizedTypes.h"

namespace rev {

/// @brief Test multiton class
class MyTestMultiton : public MultitonInterface<MyTestMultiton> {
public:
    Int32_t m_myInt{0};

    static Int32_t s_count;

protected:

    MyTestMultiton():
        m_myInt(s_count)
    {
        s_count++;
    }
};

Int32_t MyTestMultiton::s_count = 0;


/// Tests
TEST(Multiton, Construction) {
    Int32_t index0;
    Int32_t index1;
    MyTestMultiton& multiton = MyTestMultiton::Create(index0);
    MyTestMultiton& multiton1 = MyTestMultiton::Instance(index0);

    EXPECT_EQ(index0, 0);
    EXPECT_EQ(multiton.m_myInt, 0);
    EXPECT_EQ(multiton1.m_myInt, 0);
    EXPECT_EQ(multiton.s_count, 1);

    MyTestMultiton& multiton2 = MyTestMultiton::Create(index1);
    EXPECT_EQ(index1, 1);
    EXPECT_EQ(multiton2.m_myInt, 1);
    EXPECT_EQ(multiton.m_myInt, 0);
    EXPECT_EQ(multiton.s_count, 2);

    // Test that can't access instance if out of range
    bool failed = false;
    try {
        MyTestMultiton& multiton = MyTestMultiton::Instance(2);
    }
    catch (std::logic_error& err) {
        failed = true;
    }
    EXPECT_EQ(failed, true);
}




} /// End rev namespace