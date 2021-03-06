#ifndef TEST_THREADPOOL_H
#define TEST_THREADPOOL_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../GTest.h"
#include <core/threading/GThreadPool.h>
#include <core/containers/GString.h>
#include <core/GLogger.h>

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ThreadpoolTest : public Test
{
public:

    ThreadpoolTest(): Test(){}
    ~ThreadpoolTest() {}

    /// @brief Perform unit tests for Timer class
    virtual void perform() {

        ThreadPool pool;
        Logger::getStdOutHandler("StandardOut");
        Logger::LogInfo(GString::Format("Running thread pool with %d threads", pool.numThreads()).c_str());

        // TODO: Add an assert that all threads finish, but workks as of 3/2/2021
        for (size_t i = 0; i < 100; i++) {
            std::future<void> future = pool.addTask([i, &pool]() {
                auto thread_id = std::this_thread::get_id();
                size_t id = pool.getIndex(thread_id);
                Logger::LogInfo(GString::Format("I am job %d on thread %d", (size_t)i, id).c_str());
            });
            future.wait();
        }
    }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}


#endif