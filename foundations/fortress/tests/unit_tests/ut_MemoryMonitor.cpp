#include <gtest/gtest.h>

#include "fortress/numeric/GSizedTypes.h"
#include "fortress/system/memory/GMemoryMonitor.h"

namespace rev {

TEST(MemoryMonitorTest, MaxMemory) {
    Uint64_t maxMemory = MemoryMonitor::GetMaxMemory();
    EXPECT_GE(maxMemory, 1e8); /// Expect greater than 1e8 bytes of memory
    
    Uint64_t maxMemoryMb = MemoryMonitor::GetMaxMemoryMb();
    EXPECT_GE(maxMemoryMb, 1000); /// Expect at least 1GB of memory
}

} // End rev namespace
