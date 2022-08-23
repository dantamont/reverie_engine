#pragma once

#include <memory>
#include "layline/network/GNetworkAddress.h"
#include "layline/network/GNetworkPort.h"

namespace rev {

/// @class NetworkEndpoint
/// @brief An endpoint consists of a network address and port. Encapsulates a UDP and/or TCP endpoint
/// @note Asio endpoints must NOT be deleted while in use, which is why they are stored as members of this class
class NetworkEndpoint {
public:

    /// @brief Initialize the endpoint from an ASIO endpoint
    template<typename AsioEndpointType>
    static NetworkEndpoint FromAsio(const AsioEndpointType& asioEndpoint) {
        NetworkEndpoint endpoint;
        endpoint.m_port = asioEndpoint.port();
        if (asioEndpoint.address().is_v4()) {
            endpoint.setAddress(NetworkAddressV4::FromAsio(asioEndpoint.address()));
            if (std::is_same_v<AsioEndpointType, typename asio::ip::udp::endpoint>) {
                endpoint.m_udpEndpoint = asio::ip::udp::endpoint(endpoint.m_address->as<NetworkAddressV4>()->toAsio(), endpoint.m_port.portNumber());
            }
            else {
                endpoint.m_tcpEndpoint = asio::ip::tcp::endpoint(endpoint.m_address->as<NetworkAddressV4>()->toAsio(), endpoint.m_port.portNumber());
            }
        }
        else {
            endpoint.setAddress(NetworkAddressV6::FromAsio(asioEndpoint.address()));
            if (std::is_same_v<AsioEndpointType, typename asio::ip::udp::endpoint>) {
                endpoint.m_udpEndpoint = asio::ip::udp::endpoint(endpoint.m_address->as<NetworkAddressV6>()->toAsio(), endpoint.m_port.portNumber());
            }
            else {
                endpoint.m_tcpEndpoint = asio::ip::tcp::endpoint(endpoint.m_address->as<NetworkAddressV6>()->toAsio(), endpoint.m_port.portNumber());
            }
        }
        return endpoint;
    }

    /// @brief Retrieve the any address endpoint on a specific point
    template<NetworkAddress::AddressType AddressType = NetworkAddress::AddressType::kIpv4>
    static NetworkEndpoint Any(const NetworkPort& port) {
        std::unique_ptr<NetworkAddress> networkAddress;
        if constexpr (AddressType == NetworkAddress::AddressType::kIpv4) {
            networkAddress = std::make_unique<NetworkAddressV4>(NetworkAddressV4::Any());
        }
        else {
            networkAddress = std::make_unique<NetworkAddressV6>(NetworkAddressV6::Any());
        }
        return NetworkEndpoint(std::move(networkAddress), port);
    }

    /// @brief Retrieve the loopback endpoint on a specific port
    template<NetworkAddress::AddressType AddressType = NetworkAddress::AddressType::kIpv4>
    static NetworkEndpoint Loopback(const NetworkPort& port) {
        std::unique_ptr<NetworkAddress> networkAddress;
        if constexpr (AddressType == NetworkAddress::AddressType::kIpv4) {
            networkAddress = std::make_unique<NetworkAddressV4>(NetworkAddressV4::Loopback());
        }
        else {
            networkAddress = std::make_unique<NetworkAddressV6>(NetworkAddressV6::Loopback());
        }
        return NetworkEndpoint(std::move(networkAddress), port);
    }

    /// @brief Initialize a network endpoint with an address and port
    NetworkEndpoint(std::unique_ptr<NetworkAddress>&& address, const NetworkPort& port):
        m_port(port)
    {
        setAddress(std::move(address));
    }

    /// @brief Need explicit copy constructor because of unique pointer
    NetworkEndpoint(const NetworkEndpoint& other) {
        *this = other;
    }

    NetworkEndpoint() = default;

    ~NetworkEndpoint() = default;

    /// @brief Return true if valid
    bool isValid() const {
        return nullptr != m_address;
    }

    /// @brief Return the network address
    const NetworkAddress& address() const {
        return *m_address;
    }

    /// @brief Return the ASIO endpoint of the correct protocol type
    template<typename AsioProtocolType>
    typename AsioProtocolType::endpoint& toAsio() {
        static_assert(std::is_same_v<AsioProtocolType, asio::ip::udp> || std::is_same_v<AsioProtocolType, asio::ip::tcp>, "Invalid ASIO protocol type");

        if constexpr (std::is_same_v<AsioProtocolType, asio::ip::tcp>) {
            return m_tcpEndpoint;
        }
        else{
            return m_udpEndpoint;
        }
    }

    /// @brief Return the ASIO endpoint of the correct protocol type
    template<typename AsioProtocolType>
    typename const AsioProtocolType::endpoint& toAsio() const {
        static_assert(std::is_same_v<AsioProtocolType, asio::ip::udp> || std::is_same_v<AsioProtocolType, asio::ip::tcp>, "Invalid ASIO protocol type");

        if constexpr (std::is_same_v<AsioProtocolType, asio::ip::tcp>) {
            return m_tcpEndpoint;
        }
        else{
            return m_udpEndpoint;
        }
    }

    /// @brief Refresh the value of the network endpoint based on the internal ASIO endpoint
    /// @return true if refreshed required, false if not
    template<typename AsioProtocolType>
    bool refresh() {
        bool refreshed = false;
        if constexpr(std::is_same_v<AsioProtocolType, asio::ip::udp>) {
            NetworkAddress::AddressType asioAddressType = NetworkAddress::GetAddressType(m_udpEndpoint.address());

            if (m_port != m_udpEndpoint.port()) {
                refreshed = true;
                m_port = m_udpEndpoint.port();
            }

            if (asioAddressType == NetworkAddress::AddressType::kIpv4) {
                /// Address is Ipv4
                if (!m_address) {
                    // Create address if null
                    m_address = std::make_unique<NetworkAddressV4>(NetworkAddressV4::Any());
                    refreshed = true;
                }

                // Update address
#ifdef DEBUG_MODE
                assert(m_address->getAddressType() == NetworkAddress::AddressType::kIpv4 && "Inconsistent address types");
#endif
                const NetworkAddressV4& address = *m_address->as<NetworkAddressV4>();
                refreshed |= m_udpEndpoint.address() != address;
                if (refreshed) {
                    setAddress(NetworkAddressV4::FromAsio(m_udpEndpoint.address()));
                }
            }
            else {
                /// Address is Ipv6
                if (!m_address) {
                    // Create address if null
                    m_address = std::make_unique<NetworkAddressV6>(NetworkAddressV6::Any());
                    refreshed = true;
                }

                // Update address
#ifdef DEBUG_MODE
                assert(m_address->getAddressType() == NetworkAddress::AddressType::kIpv6 && "Inconsistent address types");
#endif
                const NetworkAddressV6& address = *m_address->as<NetworkAddressV6>();
                refreshed |= m_udpEndpoint.address() != address;
                if (refreshed) {
                    setAddress(NetworkAddressV6::FromAsio(m_udpEndpoint.address()));
                }
            }
        }
        else if constexpr(std::is_same_v<AsioProtocolType, asio::ip::tcp>) {
            NetworkAddress::AddressType asioAddressType = NetworkAddress::GetAddressType(m_udpEndpoint.address());

            if (m_port != m_tcpEndpoint.port()) {
                refreshed = true;
                m_port = m_tcpEndpoint.port();
            }

            if (asioAddressType == NetworkAddress::AddressType::kIpv4) {
                /// Address is Ipv4
                if (!m_address) {
                    // Create address if null
                    m_address = std::make_unique<NetworkAddressV4>(NetworkAddressV4::Any());
                    refreshed = true;
                }

                // Update address
#ifdef DEBUG_MODE
                assert(m_address->getAddressType() == NetworkAddress::AddressType::kIpv4 && "Inconsistent address types");
#endif
                const NetworkAddressV4& address = *m_address->as<NetworkAddressV4>();
                refreshed |= m_tcpEndpoint.address() != address;
                if (refreshed) {
                    setAddress(NetworkAddressV4::FromAsio(m_tcpEndpoint.address()));
                }
            }
            else {
                /// Address is Ipv6
                if (!m_address) {
                    // Create address if null
                    m_address = std::make_unique<NetworkAddressV6>(NetworkAddressV6::Any());
                    refreshed = true;
                }

                // Update address
#ifdef DEBUG_MODE
                assert(m_address->getAddressType() == NetworkAddress::AddressType::kIpv6 && "Inconsistent address types");
#endif
                const NetworkAddressV6& address = *m_address->as<NetworkAddressV6>();
                refreshed |= m_tcpEndpoint.address() != address;
                if (refreshed) {
                    setAddress(NetworkAddressV6::FromAsio(m_tcpEndpoint.address()));
                }
            }
        }
        return refreshed;
    }

    NetworkEndpoint& operator=(const NetworkEndpoint& other) {
        m_port = other.m_port;
        if (!other.m_address) {
            m_address = nullptr;
        }
        else if (other.m_address->getAddressType() == NetworkAddress::AddressType::kIpv4) {
            const NetworkAddressV4& address = *static_cast<const NetworkAddressV4*>(other.m_address.get());
            setAddress(std::make_unique<NetworkAddressV4>(address));

        }
        else if (other.m_address->getAddressType() == NetworkAddress::AddressType::kIpv6) {
            const NetworkAddressV6& address = *static_cast<const NetworkAddressV6*>(other.m_address.get());
            setAddress(std::make_unique<NetworkAddressV6>(address));
        }
        else {
#ifdef DEBUG_MODE
            assert(false && "Invalid address type");
#endif
        }


        return *this;
    }

    bool operator==(const NetworkEndpoint& other) const {
        bool portsEqual = m_port == other.m_port;
        bool addressesEqual = false;
        if (nullptr == m_address || nullptr == other.m_address) {
            addressesEqual = m_address == other.m_address;
        }
        else if (m_address->getAddressType() == NetworkAddress::AddressType::kIpv4) {
            addressesEqual = *m_address->as<NetworkAddressV4>() == *other.m_address->as<NetworkAddressV4>();
        }
        else {
            addressesEqual = *m_address->as<NetworkAddressV6>() == *other.m_address->as<NetworkAddressV6>();
        }

        return portsEqual && addressesEqual;
    }

    bool operator!=(const NetworkEndpoint& other) const {
        return !operator==(other);
    }

    operator std::string() const {
        return std::string(m_port) + ", "
            + (m_address ? 
                (m_address->isIpv4() ? 
                    std::string(*m_address->as<NetworkAddressV4>()):
                    std::string(*m_address->as<NetworkAddressV6>()))
                :
                "null"
                )
            ;
    }

private:

    /// @brief Set the address of the endpoint
    /// @detail Ensures that the address gets set properly in the underlying ASIO endpoints as well
    void setAddress(std::unique_ptr<NetworkAddress>&& address) {
        m_address = std::move(address);
        setEndpoints();
    }

    /// @brief Set the internally stored asio endpoints
    void setEndpoints() {
        if (m_address->isIpv4()) {
            m_udpEndpoint = asio::ip::udp::endpoint(m_address->as<NetworkAddressV4>()->toAsio(), m_port.portNumber());
            m_tcpEndpoint = asio::ip::tcp::endpoint(m_address->as<NetworkAddressV4>()->toAsio(), m_port.portNumber());
        }
        else {
            m_udpEndpoint = asio::ip::udp::endpoint(m_address->as<NetworkAddressV6>()->toAsio(), m_port.portNumber());
            m_tcpEndpoint = asio::ip::tcp::endpoint(m_address->as<NetworkAddressV6>()->toAsio(), m_port.portNumber());
        }
    }

    std::unique_ptr<NetworkAddress> m_address{ nullptr }; ///< The network address associated with the endpoint
    NetworkPort m_port{}; ///< The port associated with the endpoint, wildcard by default

    asio::ip::udp::endpoint m_udpEndpoint{ asio::ip::address_v4::any(), 0 }; ///< The UDP endpoint encapsulated by this class
    asio::ip::tcp::endpoint m_tcpEndpoint{ asio::ip::address_v4::any(), 0 }; ///< The TCP endpoint encapsulated by this class
};

} // End rev namespace
