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

#ifndef GB_PARALLELIZATION
#define GB_PARALLELIZATION


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


namespace Gb {
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
    ThreadPool(size_t);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        ->std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();

    size_t numThreads() const { return m_workers.size(); }

private:
    // Loop for each thread to handle tasks
    void threadLoop();

    // need to keep track of threads so we can join them
    std::vector<std::thread> m_workers;

    // the task queue
    std::queue<std::function<void()>> m_tasks;

    // synchronization
    std::mutex m_queueMutex;
    std::condition_variable m_condition;
    bool m_stop;
};

// The constructor launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads): 
    m_stop(false)
{
    for (size_t i = 0; i < threads; ++i) {
        m_workers.emplace_back([this] { threadLoop(); });
    }
}

// Loop for receiving tasks from the task queue
inline void ThreadPool::threadLoop() {
    // Infinite loop
    for (;;)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            // Takes predicate, whether condition is really met for the thread to run
            // If returns false, thread goes back to sleep
            // Returns false if the thread pool has been stopped and there
            // while (!pred()) wait(lock);
            this->m_condition.wait(lock,
                [this] { 
                // Essentially while not stopped and there are no tasks, keep waiting
                return m_stop || !m_tasks.empty(); 
            });

            // Return if no tasks left and the thread pool is stopped
            if (m_stop && m_tasks.empty()) return;

            // Populate task with the next one in the queue, and pop from queue
            task = std::move(this->m_tasks.front());
            this->m_tasks.pop();
        }

        // Perform task
        task();

    }
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);

        // don't allow enqueueing after stopping the pool
        if (m_stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        m_tasks.emplace([task]() { (*task)(); });
    }
    m_condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_stop = true;
    }
    m_condition.notify_all();
    for (std::thread &worker : m_workers)
        worker.join();
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// See: https://stackoverflow.com/questions/36246300/parallel-loops-in-c/36246386
/// @param[in] num_elements : size of your for loop
/// @param[in] functor(start, end) :
/// your function processing a sub chunk of the for loop.
/// "start" is the first index to process (included) until the index "end"
/// (excluded)
/// @code
///     for(int i = start; i < end; ++i)
///         computation(i);
/// @endcode
/// @param use_threads : enable / disable threads.
///
///

class ParallelLoopGenerator {
public:
    ParallelLoopGenerator(ThreadPool* pool = nullptr, bool use_threads = true) :
        m_useThreads(use_threads),
        m_pool(pool)
    {
    }

    void parallelFor(unsigned num_elements, std::function<void(int start, int end)> functor)
    {
        // -------
        unsigned num_threads;
        
        if (!m_pool) {
            // Use hardware to determine number of threads
            unsigned num_threads_hint = std::thread::hardware_concurrency();
            num_threads = num_threads_hint == 0 ? 8 : (num_threads_hint);
        }
        else {
            // Use number of threads in pool
            num_threads = m_pool->numThreads();
        }

        unsigned batch_size = num_elements / num_threads;
        unsigned batch_remainder = num_elements % num_threads;

        std::vector<std::thread> my_threads;
        std::vector<std::future<void>> results;
        if (m_useThreads)
        {
            // Multithread execution
            if (m_pool) {
                // Use threads from threadpool (much faster if limited by overhead of thread creation)
                for (unsigned i = 0; i < num_threads; ++i)
                {
                    int start = i * batch_size;
                    //auto result = m_pool->enqueue(functor, start, start + batch_size);
                    results.push_back(m_pool->enqueue(functor, start, start + batch_size));
                    //result.get();
                }
            }
            else {
                // Generate threads on the fly
                my_threads.resize(num_threads);
                for (unsigned i = 0; i < num_threads; ++i)
                {
                    int start = i * batch_size;
                    my_threads[i] = std::thread(functor, start, start + batch_size);
                }
            }
        }
        else
        {
            // Single thread execution (for easy debugging)
            for (unsigned i = 0; i < num_threads; ++i) {
                int start = i * batch_size;
                functor(start, start + batch_size);
            }
        }

        // Deform the elements left
        if (batch_remainder != 0) {
            int start = num_threads * batch_size;
            functor(start, start + batch_remainder);
        }

        // Wait for the other thread to finish their task
        if (m_useThreads) {
            if (!m_pool) {
                std::for_each(my_threads.begin(), my_threads.end(), std::mem_fn(&std::thread::join));
            }
            else {
//                while (m_pool->hasTasks()) {
//                    // Wait until all tasks have completed
//#ifdef DEBUG_MODE
//                    qDebug() << "ThreadPool: Waiting for task completion";
//#endif
//                }
                for (std::future<void>& result : results) {
                    result.get();
                }
            }
        }
    }

private:
    bool m_useThreads;
    ThreadPool* m_pool;
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif