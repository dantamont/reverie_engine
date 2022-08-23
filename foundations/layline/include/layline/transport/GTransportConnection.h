#pragma once

#include <atomic>
#include <iostream>
#include <array>
#include <memory>

#include <asio.hpp>

#include "fortress/logging/GThreadSafeCout.h"
#include "layline/network/GNetworkEndpoint.h"

namespace rev {

/// @brief A class representing a connection between two transports
class TransportConnection {
public:

    TransportConnection() = default;
    TransportConnection(const NetworkEndpoint& remoteEndpoint):
        m_remoteEndpoint(remoteEndpoint)
    {

    }

    /// @brief The remote endpoint for the connection
    const NetworkEndpoint& remoteEndpoint() const {
        return m_remoteEndpoint;
    }

    /// @brief The remote endpoint for the connection
    NetworkEndpoint& remoteEndpoint() {
        return m_remoteEndpoint;
    }

    /// @brief Return true if attached to two endpoints, false otherwise
    bool attached() const {
        return m_remoteEndpoint.isValid();
    }

    operator std::string() const {
        return "\nRemote Endpoint: " + std::string(m_remoteEndpoint);
    }

private:

    NetworkEndpoint m_remoteEndpoint{}; ///< The remote endpoint for the connection. Unique to this connection
};

} // End rev namespace
 