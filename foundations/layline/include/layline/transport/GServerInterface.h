#pragma once

#include "layline/transport/GTransportInterface.h"

namespace rev {

/// @class ServerInterface
/// @brief Class representing a server
template<typename AsioProtocolType>
class ServerInterface {
protected:

    static constexpr Uint64_t s_defaultSendTimeMicroseconds{ 0 }; ///< Time for server to send messages. Is immediate by default

public:

    ServerInterface():
        m_thread(std::make_shared<TransportThread>())
    {

    }
    virtual ~ServerInterface() = default;

    /// @brief Actually begin the asynchronous transport calls
    inline void run() {
        if (m_thread) {
            m_thread->run();
        }
    }

    /// @brief Shut down the transport's thread and disconnect
    inline void shutdown() {
        if (m_thread) {
            m_thread->shutdown();
        }
    }

protected:

    std::shared_ptr<TransportThread> m_thread{ nullptr }; ///< The thread for running this server

private:

    /// @brief Create a socket and initiate an asynchronous accept operation to wait for a new connection
    //virtual void acceptConnection() = 0;

};


} // End rev namespace
 