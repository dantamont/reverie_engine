/// @file GThreadpool.h
/// @author Dante Tufano
// ThreadPool Licensing
//Copyright (c) 2012 Jakob Progsch, Václav Zeman
//
//This software is provided 'as-is', without any express or implied
//warranty. In no event will the authors be held liable for any damages
//arising from the use of this software.
//
//Permission is granted to anyone to use this software for any purpose,
//including commercial applications, and to alter it and redistribute it
//freely, subject to the following restrictions:
//
//   1. The origin of this software must not be misrepresented; you must not
//   claim that you wrote the original software. If you use this software
//   in a product, an acknowledgment in the product documentation would be
//   appreciated but is not required.
//
//   2. Altered source versions must be plainly marked as such, and must not be
//   misrepresented as being the original software.
//
//   3. This notice may not be removed or altered from any source
//   distribution.
/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef G_THREADPOOL_H
#define G_THREADPOOL_H


// QT

// Internal
#include <algorithm>
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>


namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class ThreadPool class
// See: https://github.com/progschj/ThreadPool
class ThreadPool {
public:

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    ThreadPool(size_t numThreads = std::thread::hardware_concurrency())
    {
        if (numThreads) {
            initialize(numThreads);
        }
        else {
            m_numThreads = 0;
        }
    }

    ~ThreadPool() {
        shutdown();
        throwOnFailure();
    }

    /// @}


    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    size_t numThreads() const {
        return m_numThreads;
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Obtain the index of the thread with the specified ID
    size_t getIndex(std::thread::id threadId) {
        std::unique_lock lock(m_threadIdMutex);
        return m_threadIds[threadId];
    }

    /// @brief Check if a thread has failed, rethrowing an error if it has
    void throwOnFailure() {
        std::unique_lock lock(m_exceptionMutex); // Need thread-safety
        if (m_exceptionPtr) {
            // Rethrow exception so threads don't silently fail
            std::string exceptionStr = m_exceptionStr;
            std::rethrow_exception(m_exceptionPtr);
        }
    }

    /// @brief Add a job to the pool's task queue
    template<class F, class... Args>
    auto addTask(F&& f, Args&&... args)
        ->std::future<typename std::invoke_result_t<F, Args...>>
    {
        using return_type = typename std::invoke_result_t<F, Args...>;

        auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);

            // Don't allow enqueueing after stopping the pool
            if (m_shutdown.load()) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            m_tasks.emplace([task]() { (*task)(); });
        }
        m_condition.notify_one();
        return res;
    }

    /// @brief Specialization to take a lambda function
    //template<>
    //std::future<void> addTask(const std::function<void()>& f) {
    //}

    /// @brief Shutdown all of the threads in the pool
    inline void shutdown() {
        // Break each thread's while loop
        m_shutdown.store(true);
        m_condition.notify_all();

        // Join each thread
        for (std::thread& t : m_threads) {
            // Check if joinable, in which case threads are not already joined
            if (t.joinable()) {
                t.join();
            }
        }

        m_shutdown.store(false);
    }

    /// @}

private:
    //--------------------------------------------------------------------------------------------
    /// @name Methods
    /// @{

    inline void initialize(size_t numThreads) {
        if (m_threads.size()) {
            // If already initialized, shutdown the running threads
            shutdown();
        }

        std::unique_lock lock(m_threadIdMutex);
        m_threadIds.clear();
        for (size_t i = 0; i < numThreads; i++) {
            m_threads.push_back(std::thread(&ThreadPool::safeTaskLoop, this));
            m_threadIds[m_threads.back().get_id()] = i;
        }
        m_numThreads = numThreads;
    }

    /// @brief Wrapper for the task loop, which catches exceptions
    void safeTaskLoop() {
        try {
            taskLoop();
        }
        catch (...) {
            // Catch any errors so that they can be thrown again on the main thread
            std::unique_lock lock(m_exceptionMutex);
            m_exceptionPtr = std::current_exception();
        }
    }

    /// @brief Function for each thread's task handling
    /// @details This loop runs indefinitely until the threadpool is shut down
    inline void taskLoop() {
        while (true) {
            std::function<void()> task;
            {
                // Wait causes the current thread to block until the condition variable is notified
                // The predicate passed into the wait routine gives behavior equivalent to:
                // while(!pred()){wait(lock);}

                // Once the wait condition is reached, this thread will own the lock
                std::unique_lock lock(m_queueMutex);
                m_condition.wait(lock, [this] {return !m_tasks.empty() || m_shutdown.load(); });

                if (m_shutdown && m_tasks.empty()) {
                    // No tasks and is shutting down, so break the loop
                    return;
                }

                // Remove the first-added task from the queue to perform it
                task = std::move(m_tasks.front());
                m_tasks.pop();
            }

            // Perform the task
            task();
        }
    }

    /// @}


    //--------------------------------------------------------------------------------------------
    /// @name Members
    /// @{

    /// @brief The number of threads in the pool
    size_t m_numThreads;

    /// @brief The condition variable for assigning threads to tasks
    std::condition_variable m_condition;

    /// @brief The threads running in the threadpool
    std::vector<std::thread> m_threads;

    /// @brief Map of identifiers for eaceh thread and their corresponding IDs in the threads vector
    std::unordered_map<std::thread::id, size_t> m_threadIds;

    /// @brief The tasks to be handled by the thread pool
    std::queue<std::function<void()>> m_tasks;

    /// @brief Mutex for sccessing task queue
    std::mutex m_queueMutex;

    /// @brief Mutex for accessing thread IDs
    std::mutex m_threadIdMutex;

    /// @brief Mutex for throwing exceptions
    std::mutex m_exceptionMutex;

    /// @brief Pointer for tracking exceptions on worker threads
    std::exception_ptr m_exceptionPtr;
    std::string m_exceptionStr;

    /// @brief Whether or not to shutdown the threadpool
    std::atomic<bool> m_shutdown;

    /// @}
};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif