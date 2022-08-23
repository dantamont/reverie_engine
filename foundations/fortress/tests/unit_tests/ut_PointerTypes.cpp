#include <gtest/gtest.h>

#include "fortress/types/GSizedTypes.h"
#include "fortress/system/memory/GPointerTypes.h"

namespace rev {

struct MyStruct {
protected:
    MyStruct() {

    }
    Uint8_t m_data1{ 1 };
    Uint16_t m_data2{ 7 };
};

TEST(PointerTypesTest, Construction) {
    std::shared_ptr<MyStruct> myStruct = prot_make_shared<MyStruct>();
    std::unique_ptr<MyStruct> myStructUnique = prot_make_unique<MyStruct>();
}

} // End rev namespace
