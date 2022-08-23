#include <gtest/gtest.h>

#include "fortress/containers/GByteArray.h"

namespace rev {

struct MyStruct {
    Uint8_t m_data1{ 1 };
    Uint16_t m_data2{ 7 };
};

TEST(ByteArrayTest, Construction) {
    static constexpr size_t ArrSize = 10;
    MyStruct* myArray = new MyStruct[ArrSize];
    for (Uint32_t i = 0; i < ArrSize; i++) {
        MyStruct& entry = myArray[i];
        entry.m_data1 = i;
        entry.m_data2 = i * 7 + i * 2;
    }

    ByteArray myByteArray(myArray);

    for (Uint32_t i = 0; i < ArrSize; i++) {
        MyStruct& entry = myByteArray.as<MyStruct>()[i];
        EXPECT_EQ(entry.m_data1, myArray[i].m_data1);
        EXPECT_EQ(entry.m_data2, myArray[i].m_data2);
    }

}

} // End rev namespace
