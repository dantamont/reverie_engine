#include <gtest/gtest.h>

#include "fortress/containers/GContainerExtensions.h"

namespace rev {

//class NoCopyClass {
//public:
//    NoCopyClass() = default;
//    NoCopyClass(int val, bool set) :
//        m_value(val),
//        m_set(set)
//    {
//    }
//    NoCopyClass(NoCopyClass&& other) {}
//    NoCopyClass(const NoCopyClass& myClass) = delete;
//
//    NoCopyClass& operator=(const NoCopyClass&) = delete;
//    NoCopyClass& operator=(NoCopyClass&& other) {}
//
//private:
//
//    int m_value{ 7 };
//    bool m_set{ false };
//};


TEST(ContainerExtensionTests, ArrayConstruction) {
    // By value
    std::array<int, 9> myArray;
    for (int i = 0; i < 9; i++) {
        myArray[i] = 10;
    }

    std::array<int, 9> betterArray = CreateArray<9>(10);

    EXPECT_EQ(myArray, betterArray);
}

//TEST(ContainerExtensionTests, AggregateConstruction) {
//    // By value
//    std::array<NoCopyClass, 3> myArray = { NoCopyClass(5, true), NoCopyClass(5, true), NoCopyClass(5, true)};
//
//    std::array<NoCopyClass, 3> betterArray = CreateArray<3>(NoCopyClass(5, true));
//
//    EXPECT_EQ(myArray, betterArray);
//}

} // End rev namespace
