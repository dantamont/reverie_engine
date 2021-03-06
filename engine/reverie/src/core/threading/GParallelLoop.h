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
#include <core/threading/GThreadpool.h>

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
                    results.push_back(m_pool->addTask(functor, start, start + batch_size));
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