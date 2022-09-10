#pragma once

#include "fortress/math/GRandom.h"
#include "fortress/templates/GTemplates.h"
#include "fortress/numeric/GSizedTypes.h"

namespace rev {

/// @class NetworkEndpoint
class NetworkPort {
public:

    /// @brief Return a random user port
    static NetworkPort UserPort() {
        Uint32_t portNumber = Random::GetRandomNumber(s_minimumValidDynamicPort, s_maximumValidDynamicPort);
        return NetworkPort(portNumber);
    }

    static Uint32_t LowestUserPortNumber() {
        return s_minimumValidUserPort;
    }

    static Uint32_t HighestUserPortNumber() {
        return s_maximumValidUserPort;
    }

    /// @brief Return a wildcard (any free) port
    static NetworkPort Wildcard() {
        return NetworkPort{ 0 };
    }

    /// @brief Wildcard by default
    NetworkPort() = default;

    template<typename T>
    NetworkPort(T portNumber) :
        m_portNumber(portNumber) 
    {
        static_assert(std::is_integral_v<T> && sizeof(T) >= 2,
            "Port number can only be initialized with an integral type that is at least two bytes in size");
    }
    ~NetworkPort() = default;

    Uint16_t portNumber() const {
        return m_portNumber;
    }

    operator Uint16_t() const {
        return m_portNumber;
    }

    operator std::string() const {
        return std::to_string(m_portNumber);
    }

private:

    /// @note A value of 0 means wildcard(find a free port) in UDP, 
    ///   and is reserved for dedicated TCP/IP communication, and thus 
    ///   unused in the TCP/IP protocol, so will be treated like a wildcard
    ///   for that as well.
    Uint16_t m_portNumber{ 0 }; ///< The port number, from 0 - 65535


    static constexpr Uint32_t s_minimumValidUserPort = 1024; ///< Lowest user port
    static constexpr Uint32_t s_maximumValidUserPort = 49151; ///< Highest user port
    static constexpr Uint32_t s_minimumValidDynamicPort = 49152; ///< Lowest dynamic port
    static constexpr Uint32_t s_maximumValidDynamicPort = 65535; ///< Highest dynamic port
};

} // End rev namespace
