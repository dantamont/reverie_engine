#pragma once

// std
#include <chrono>
#include <iomanip>

// QT

// Internal
#include "fortress/string/GString.h"

namespace rev {

/// @brief Clock class
/// @details For managing simulation time
// See: https://codereview.stackexchange.com/questions/225923/a-simple-stop-watch-which-i-want-to-extend
class StopwatchTimer {
private:
    using clock = std::chrono::steady_clock;

    /// @name Private Members
    /// @{

    static constexpr clock::duration s_zeroDuration = std::chrono::nanoseconds::zero(); ///< Zero length duration
    clock::time_point m_startTime = {};
    clock::duration m_elapsedTime = s_zeroDuration; ///< std::chrono::steady_clock::duration is std::duration::nanoseconds

    /// @}
public:
    /// @name Static
    /// @{

    /// @brief Convience methods to convert clock durations to specified units
    /// @tparam Rep the return type, e.g. Float64_t, int
    template<typename Rep>
    static Rep ToSeconds(const clock::duration& duration) {
        return std::chrono::duration<Rep>(duration).count();
    }

    /// @tparam Rep the return type, e.g. Float64_t, int
    template<typename Rep>
    static Rep ToMicroSeconds(const clock::duration& duration) {
        return std::chrono::duration_cast<std::chrono::duration<Rep, std::micro>>(duration).count();
    }

    /// @brief Create a timer that initialized the specified number of seconds ago
    static StopwatchTimer FromElapsedSeconds(float seconds);

    /// @brief Convert steady clock time to time_t
    static std::time_t SteadyToTime_t(const clock::time_point& time);

    /// @brief Return a string representation
    static GString ToString(const clock::time_point& time) {
        return ToString(time, GString());
    }
    static GString ToString(const clock::time_point& time, const GString& format);

    /// @}

    /// @name Constructors/Destructor
    /// @{

    StopwatchTimer();
    StopwatchTimer(const StopwatchTimer& other);
    virtual ~StopwatchTimer();

    /// @}

    /// @name Properties
    /// @{

    const clock::time_point& startTime() const {
        return m_startTime;
    }

    /// @}

    /// @name Operators
    /// @{

    StopwatchTimer& operator=(const StopwatchTimer& other);

    /// @}

    /// @name Public methods
    /// @{

    /// @brief Whether or not the clock is currently counting time
    bool isRunning() const {
        // Is running if start time is not a null time_point
        return m_startTime != clock::time_point{};
    }

    /// @brief Restart the timer, returning the elapsed time as a duration
    clock::duration restart() {
        // Stop to cache elapsed time
        stop(); 
        clock::duration elapsedTime(m_elapsedTime);

        // Reset to clear elapsed time
        reset();

        // Start clock again
        start();

        return elapsedTime;
    }

    /// @brief Start the timer, doing nothing if already running
    void start() {
        // Only start of not running
        if (!isRunning()) {
            m_startTime = clock::now();
        }
    }

    /// @brief Stop the timer, caching the elapsed time
    void stop() {
        if (isRunning()) {
            m_elapsedTime += clock::now() - m_startTime;
            m_startTime = {};
        }
    }

    /// @brief Clear the timer
    void reset() {
        m_startTime = {};
        m_elapsedTime = s_zeroDuration;
    }

    /// @brief Get elapsed time as a clock duration
    clock::duration getElapsedDuration() const {
        auto result = m_elapsedTime;
        if (isRunning()) {
            result += clock::now() - m_startTime;
        }
        return result;
    }

    /// @brief Return elapsed time in seconds, with given type, e.g. float, double
    /// @tparam Rep the return type, e.g., Float64_t
    /// @todo Make this templated using GUnits for time conversions
    /// @example float elapsedSec = myTimer.getElapsed<float>();
    template<typename Rep>
    Rep getElapsed() const {
        return ToSeconds<Rep>(getElapsedDuration());
    }

    /// @brief Return elapsed time in microseconds
    Uint64_t getElapsedMicroseconds() const {
        return ToMicroSeconds<Uint64_t>(getElapsedDuration());
    }

    /// @}
};


} // End rev namespaces
