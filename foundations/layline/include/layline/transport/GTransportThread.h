#pragma once

#include <atomic>
#include <memory>

#include <asio.hpp>

namespace rev {

/// @class TransportThread
/// @brief Class representing a thread dedicated to running a transport
class TransportThread {
public:
    TransportThread() = default;

    ~TransportThread() {
        shutdown();
    }

    /// @brief The ASIO I/O context used for transport operations
    asio::io_context& context() { return m_ioContext; }

    /// @brief Whether or not the thread is active
    bool isActive() const { return m_active; }

    /// @brief Actually begin the asynchronous transport calls
    /// @detail Need to join the thread via close() routine
    void run() {
        m_active = true;
        m_thread = std::thread([&]() {m_ioContext.run(); });
    }

    /// @brief Shut down the transport's thread and disconnect
    void shutdown() {
        if (m_active) {
            m_active = false;
            m_ioContext.stop();
            m_thread.join();
            m_ioContext.reset();
        }
    }


private:

    std::atomic<bool> m_active{ false }; ///< Controls whether or not the transport is connected and communicating
    std::thread m_thread{}; ///< The thread that this transport will run on
    asio::io_context m_ioContext{}; ///< The I/O context for this transport thread

};

} // End rev namespace
 