#include <gtest/gtest.h>

#include "fortress/json/GJson.h"
#include "fortress/numeric/GSizedTypes.h"

namespace rev {

TEST(GJsonTests, Cbor) {
    const std::array<Uint32_t, 5> testArray{ 1, 2, 3, 4, 5 };

    json j;
    j["key"] = 42;
    j["lock"] = "locked";
    j["children"] = testArray;

    std::vector<uint8_t> binary = json::to_cbor(j);
    Uint32_t numBytes = binary.size();

    json j2 = json::from_cbor(binary);
    EXPECT_EQ(j2["key"], 42);
    EXPECT_EQ(j2["lock"], "locked");
    EXPECT_EQ(j2["children"], testArray);

}


} // End rev namespace
