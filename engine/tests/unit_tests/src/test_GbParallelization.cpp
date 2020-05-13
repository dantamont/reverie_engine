///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "test_GbParallelization.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace Gb;

QMutex TestParallelization::m_mutex;
//std::vector<float> TestParallelization::m_vector;

TestParallelization::TestParallelization() : 
    TestBase(),
    m_pool(2)
{
}

void TestParallelization::parallelLoop() {
    std::vector<float> tempVec(100);

    ParallelLoopGenerator loop(&m_pool, true);
    //ParallelLoopGenerator loop(nullptr, true);
    loop.parallelFor(90, 
        [&](int start, int end) {

        //QMutexLocker locker(&m_mutex);
        for (int i = start; i < end; ++i) {
            
            tempVec[i] = float(i);
        }


    });
}