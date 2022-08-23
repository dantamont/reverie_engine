#include <gtest/gtest.h>
#include "fortress/types/GSizedTypes.h"
#include "fortress/thread/GThreadpool.h"
#include "fortress/thread/GParallelLoop.h"

namespace rev{


TEST(ParallelForLoop, Locking)
{
    ThreadPool myThreadPool(4);

    // Test parallel loop
    static constexpr Uint32_t vecSize = 100;
    std::vector<float> tempVec(vecSize);

    ParallelLoopGenerator loop(&myThreadPool, true);
    loop.parallelFor(vecSize,
        [&](int start, int end) {

        for (int i = start; i < end; ++i) {

            tempVec[i] = float(i);
        }

    });
}


} /// End rev namespace
