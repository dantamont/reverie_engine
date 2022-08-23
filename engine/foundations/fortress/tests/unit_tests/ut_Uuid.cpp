#include <gtest/gtest.h>

#include "fortress/encoding/uuid/GUuid.h"

namespace rev {

TEST(UuidTest, Construction) {
    Uuid uuid1(false);
    Uuid uuid2(true);
    Uuid uuid3(true);
    Uuid uuid4((Uint32_t)16);
    Uuid uuid5((Uint64_t)(pow(2, 36) - 1));

    EXPECT_EQ(uuid1.isNull(), true);
    EXPECT_EQ(uuid2.isNull(), false);
    EXPECT_NE(uuid1, uuid3);
    EXPECT_EQ(uuid4.data1(), 16);

    constexpr Uint16_t maxAllowedUint16 = std::numeric_limits<Uint16_t>::max();
    EXPECT_EQ(uuid5.data1(), 15);
    EXPECT_EQ(uuid5.data2(), maxAllowedUint16); ///< Should be saturated
    EXPECT_EQ(uuid5.data3(), maxAllowedUint16); ///< Should be saturated
}

TEST(UuidTest, ToAndFromString) {
    Uuid uuid(true);

    GString uuidStr(uuid);
    EXPECT_EQ(uuidStr[0], '{');
    EXPECT_EQ(uuidStr[37], '}');

    Uuid uuid2(uuidStr);
    EXPECT_EQ(uuid, uuid2);
}

TEST(UuidTest, ToAndFromJson) {
    Uuid uuid(true);

    json uuidJson(uuid);
    Uuid uuid2 = uuidJson;
    EXPECT_EQ(uuid, uuid2);
}

TEST(UuidTest, Comparator) {
    Uuid uuid(true);
    Uuid uuid2(uuid);

    EXPECT_EQ(uuid, uuid2);
    EXPECT_EQ(uuid2 != uuid, false);
}

} // End rev namespace
