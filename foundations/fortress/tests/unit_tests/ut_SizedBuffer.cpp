#include <gtest/gtest.h>

#include "fortress/system/GSystemPlatform.h"
#include "fortress/containers/GSizedBuffer.h"

namespace rev {

TEST(SizedBuffer, DefaultConstruction) {
    SizedBuffer<20> myTestBuffer;
    EXPECT_EQ(myTestBuffer.GetMaxSizeBytes(), 20);
}

TEST(SizedBuffer, SetData) {
    SizedBuffer<20> myTestBuffer;
    Uint64_t zero = 0;
    Int64_t nextIndex = myTestBuffer.setData(zero, 0);
    for (Size_t i = 0; i < 8; i++) {
        EXPECT_EQ(myTestBuffer[i], 0);
    }

    Uint64_t ones = 0xfffffffffffffffd; // 8 bytes
    nextIndex = myTestBuffer.setData(ones, nextIndex);

    // Valid results will depend on underlying system endianness
    SystemMonitor::SystemEndianness endianness = SystemMonitor::GetEndianness();
    if (endianness == SystemMonitor::SystemEndianness::kBig) {
        for (Size_t i = 8; i < 15; i++) {
            Uint8_t value = myTestBuffer[i];
            EXPECT_EQ(value, 255);
        }
        EXPECT_EQ(myTestBuffer[15], 253);
    }
    else {
        for (Size_t i = 9; i < 16; i++) {
            Uint8_t value = myTestBuffer[i];
            EXPECT_EQ(value, 255);
        }
        EXPECT_EQ(myTestBuffer[8], 253);
    }
}

} // End rev namespace
