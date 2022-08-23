#include <gtest/gtest.h>

#include <vector>
#include "fortress/containers/GStrictGrowContainer.h"


// Tests
using namespace rev;


TEST(TestStrictGrowContainer, Vector)
{
    StrictGrowContainer<std::vector<Uint32_t>> container;
    Uint32_t indexAdded = container.push(0);

    EXPECT_EQ(indexAdded, 0);

    container.emplace_back();
    container.emplace_back();

    EXPECT_EQ(container.size(), 3);
    
    constexpr Uint32_t testVal = 7;
    container.invalidate(1);
    indexAdded = container.push(testVal);

    EXPECT_EQ(container[1], testVal);
    EXPECT_EQ(indexAdded, 1);
}
