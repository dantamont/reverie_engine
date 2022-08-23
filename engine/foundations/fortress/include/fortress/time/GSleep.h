#pragma once

#include <chrono>
#include <thread>
#include <mutex>
#include "fortress/system/GSystemPlatform.h"
#include "fortress/types/GSizedTypes.h"

#ifdef G_SYSTEM_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#  include <windows.h>
#undef max
#undef min

#else
#  include <time.h>
#  include <errno.h>

#  ifdef __APPLE__
#    include <mach/clock.h>
#    include <mach/mach.h>
#  endif
#endif // _WIN32

namespace rev {

/// @class Sleep
/// @brief Class containing various functions for a thread to sleep
/// @details This is necessitated by the inaccuracy of std::this_thread::sleep_for()
/// @see https://itecnote.com/tecnote/c-precise-thread-sleep-needed-max-1ms-error/
/// @see https://gist.github.com/Youka/4153f12cf2e17a77314c
/// @see https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
class Sleep {
private:
    typedef std::chrono::high_resolution_clock clock;
    template <typename T>
    using duration = std::chrono::duration<T, std::micro>;

private:

    /// @brief Statistics for running precise sleep
    struct TimingStatistics {
        
        /// @see Welford's algorithm https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford%27s_online_algorithm
        void updateStatistics(double observedTimeDelta) {
            ++m_count;
            double delta = observedTimeDelta - m_mean;
            m_mean += delta / m_count;
            m_m2 += delta * (observedTimeDelta - m_mean);
            double stddev = sqrt(m_m2 / (m_count - 1));
            m_estimate = m_mean + stddev;
        }

        std::atomic<double> m_estimate{ 5e3 };

    private:
        double m_mean{ 5e3 };
        double m_m2{ 0 }; ///< Aggregates the square distance from the mean
        Int64_t m_count{ 1 }; ///< Number of entries
    };

public:
    
    /// @brief A simple wrapper around sleep_for for more accuracy
    /// @note Avoid use. Seems to be nearly as busy as BusySleep
    /// @param[in] timeUs How long to wait, in microseconds
    static void SleepFor(double timeUs)
    {
        static constexpr duration<double> s_minSleepDuration(0);
        clock::time_point start = clock::now();
        while (duration<double>(clock::now() - start).count() < timeUs) {
            std::this_thread::sleep_for(s_minSleepDuration);
        }
    }

    /// @brief Sleep for a precise number of microseconds
    /// @details Unlike SleepUs, this is completely cross-platform
    /// @see https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
    static void PreciseSleepUs(double us) {
        static std::mutex s_mutex;
        static TimingStatistics s_statistics;
        static constexpr double divisor = 1 / 1e3;

        while (us > s_statistics.m_estimate) {
            // Sleep
            auto start = clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            // Determine elapsed time in microseconds, and subtract from original sleep request
            auto end = clock::now();
            double observed = (end - start).count() * divisor;
            us -= observed;

            // Lock so statistics are thread-safe
            std::unique_lock lock(s_mutex);
            s_statistics.updateStatistics(observed);
        }

        // spin lock (busy sleep)
        auto start = clock::now();
        while ((clock::now() - start).count() * divisor < us);
    }

    /// @brief Busy sleep for some number of microseconds. 
    /// @note Use with caution, Clogs a CPU
    static void BusySleep(std::chrono::microseconds t) {
        BusySleep(std::chrono::duration_cast<std::chrono::nanoseconds>(t));
    }

#if defined(G_SYSTEM_PLATFORM_LINUX) || defined(G_SYSTEM_PLATFORM_APPLE)
    static void SleepInMs(Uint32_t ms) {
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = ms % 1000 * 1000000;

        while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
    }

    static void SleepInUs(Uint32_t us) {
        struct timespec ts;
        ts.tv_sec = us / 1000000;
        ts.tv_nsec = us % 1000000 * 1000;

        while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
    }

#endif // Unix/Apple

#ifdef G_SYSTEM_PLATFORM_WINDOWS
    static void SleepInMs(Uint32_t ms) {
        SleepInUs(ms * 1000);
    }

    /// @brief Sleep for a precise number of microseconds, with a windows specific implementation
    /// @note This is non-reentrant (not thread safe) because of the static handle timer
    /// @see https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/ 
    static void SleepInUs(Int64_t timeInUs) {
        static HANDLE timer = CreateWaitableTimer(NULL, FALSE, NULL);
        static std::mutex s_mutex;
        static TimingStatistics s_statistics;
        static constexpr double divisor = 1 / 1e3;

        while (timeInUs - s_statistics.m_estimate > 1e-1) {
            double toWait = timeInUs - s_statistics.m_estimate;
            LARGE_INTEGER due;
            due.QuadPart = Int64_t(toWait * -10); // Negative means relative time, units are 100ns chunks, so multiply by ten
            auto start = clock::now();
            SetWaitableTimerEx(timer, &due, 0, NULL, NULL, NULL, 0);
            WaitForSingleObject(timer, INFINITE);
            auto end = clock::now();

            double observed = (end - start).count() * divisor;
            timeInUs -= observed;
            double error = observed - toWait;

            // Lock so statistics are thread-safe
            std::unique_lock lock(s_mutex);
            s_statistics.updateStatistics(observed);
        }

        // spin lock (busy sleep)
        auto start = clock::now();
        while ((clock::now() - start).count() * divisor < timeInUs);
    }
#endif // Windows

private:

    /// @brief Busy sleep for some number of nanoseconds
    /// @note Use with caution, Clogs a CPU
    static void BusySleep(std::chrono::nanoseconds t) {
        auto end = std::chrono::steady_clock::now() + t - s_busySleepOverhead;
        while (std::chrono::steady_clock::now() < end);
    }

    /// @brief Get a rough estimate of how much overhead there is in calling busySleep()
    static std::chrono::nanoseconds GetBusySleepOverhead();

    static const std::chrono::nanoseconds s_busySleepOverhead; ///< The overhead constant that will be used in busySleep()
};

} // End rev namespaces
