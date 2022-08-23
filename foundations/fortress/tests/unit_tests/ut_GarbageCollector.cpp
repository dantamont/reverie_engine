#include <gtest/gtest.h>

#include "fortress/system/memory/GGarbageCollector.h"
namespace rev {

class TestObject {
private:
    Uint64_t m_testMember{ 7 };
};

TEST(GarbageCollectorTests, Test) {
    auto* myTestPointer = new TestObject();
    GarbageCollector<TestObject>& gc = GarbageCollector<TestObject>::Instance();
    gc.deferredDelete(myTestPointer, 0); // 0 ms
    EXPECT_EQ(gc.pointerCount(), 1);
    gc.update();
    EXPECT_EQ(gc.pointerCount(), 0);
}

} // End rev namespace
