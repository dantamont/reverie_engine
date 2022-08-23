#include <gtest/gtest.h>

#include <chrono>

#include "fortress/types/GSizedTypes.h"
#include "fortress/types/GString.h"
#include "fortress/thread/GThreadpool.h"

namespace rev {

TEST(ThreadPoolTest, TestPool) {

    ThreadPool pool;

    /// @todo Add an assert that all threads finish, but works as of 3/2/2021
    /// @todo Don't wait within the loop (this becomes a serialized operation). 
    /// A better test might be to add all of the futures to a vector, and wait on them
    /// all at the end.
    for (size_t i = 0; i < 1000; i++) {
        std::future<size_t> future = pool.addTask([i, &pool]() {
            auto thread_id = std::this_thread::get_id();
            size_t id = pool.getIndex(thread_id);
            //std::cout << GString::Format("I am job %d on thread %d", (size_t)i, id).c_str();
            return i;
            });

        const auto start = std::chrono::system_clock::now();

        future.wait();

        const auto diff = std::chrono::system_clock::now() - start;
        //std::cout << std::chrono::duration<double>(diff).count() << " seconds\n";

        //std::cout << "Thread " << i << ": " << future.get() << '\n';
    }
}

} /// End rev namespace
