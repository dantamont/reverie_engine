#pragma once

#include <array>
#include <memory>

#include <asio.hpp>

#include "fortress/encoding/binary/GEndianConverter.h"
#include "fortress/string/GString.h"
#include "layline/network/GNetworkProtocol.h"

namespace rev {

/// @class NetworkAddress
/// @brief Abstraction of an IPv4 or IPv6 address
class NetworkAddress {
public:
    enum class AddressType {
        kIpv4, ///< A 32-bit IPv4 address
        kIpv6 ///< A 64-bit IPv6 address
    };

    /// @brief Return the address type of the given ASIO address
    static NetworkAddress::AddressType GetAddressType(const asio::ip::address& address) {
        // Asio address will never have an invalid type
        if (address.is_v4()) {
            return NetworkAddress::AddressType::kIpv4;
        }
        else {
            return NetworkAddress::AddressType::kIpv6;
        }
    }

    NetworkAddress() = default;
    NetworkAddress(const char* name) :
        m_name(name) {
    }
    virtual ~NetworkAddress() = default;

    /// @brief Return whether or not the address is ipv4
    bool isIpv4() const {
        return getAddressType() == AddressType::kIpv4;
    }

    /// @brief Return whether or not the address is ipv6
    bool isIpv6() const {
        return getAddressType() == AddressType::kIpv6;
    }

    /// @brief Return whether or not the address is the any address
    bool isAny() const;

    /// @brief Get the address type
    virtual const AddressType getAddressType() const = 0;

    /// @brief Return the network address as the given type
    template<typename T>
    const T* as() const {
        static_assert(std::is_base_of_v<NetworkAddress, T>, "Can only cast to a network address type");
        return static_cast<const T*>(this);
    }

private:

    GString m_name{}; ///< Name if a named address
};

/// @class NetworkAddressInterface
/// @brief Template interface for providing common functions between network addresses
/// @detail Addresses are stored in network-byte order (big-endian) representation via std::arrays
template<typename StorageType>
class NetworkAddressInterface : public NetworkAddress {
public:

    /// @brief Return the address
    const StorageType& address() const {
        return m_address;
    }

    bool operator==(const NetworkAddressInterface& other) const {
        return m_address == other.m_address;
    }

    bool operator!=(const NetworkAddressInterface& other) const {
        return m_address != other.m_address;
    }

    operator std::string() const {
        std::string outStr = "";
        Uint32_t count = 0;
        for (const auto& entry : m_address) {
            if (count != 0) {
                outStr += ".";
            }
            outStr += std::to_string(entry);
            count++;
        }
        return outStr;
    }

protected:
    /// @brief Populate the ASIO address with this address
    template<typename AsioAddressType, AddressType Type>
    AsioAddressType toAsioBase() const {
        if constexpr (std::is_same_v<AsioAddressType, asio::ip::address_v4>) {
            static_assert(Type != AddressType::kIpv6, "Address IP type mismatch");
        }
        else if constexpr (std::is_same_v<AsioAddressType, asio::ip::address_v6>) {
            static_assert(Type != AddressType::kIpv4, "Address IP type mismatch");
        }
        else {
            static_assert(false, "Invalid IP address type");
        }
        AsioAddressType address(m_address);
        return address;
    }

    StorageType m_address; ///< The address
};

/// @class NetworkAddressV4
/// @brief Abstraction of an IPv4 address
/// @detail Stores a 32-bit network address
class NetworkAddressV4 : public NetworkAddressInterface<std::array<Uint8_t, 4>>
{
public:
    using NetworkAddressInterface::operator==; ///< So this class can use template definitions
    using NetworkAddressInterface::operator!=; ///< So this class can use template definitions
    using NetworkAddressInterface::operator std::string;

    /// @brief Return a V4 network address from an ASIO address
    static std::unique_ptr<NetworkAddressV4> FromAsio(const asio::ip::address& asioAddress) {
        /// \note to_uint() returns address in host byte order, so don't use this, as we want network order
        assert(asioAddress.is_v4());
        asio::ip::address_v4::bytes_type addressBytes = asioAddress.to_v4().to_bytes();

        return std::make_unique<NetworkAddressV4>(addressBytes);
    }

    /// @brief Return "any" address 0.0.0.0
    /// @detail The any address, INADDR_ANY, is a way of configuring servers to listen on any IPv4 address at all
    static const NetworkAddressV4& Any() {
        static const NetworkAddressV4 any{ 0 };
        return any;
    }

    /// @brief Return loopback address 127.0.0.1
    /// @detail The loopback address refers to the address of the current device, or host, attempting to access it
    static const NetworkAddressV4& Loopback() {
        static const NetworkAddressV4 loopback{ 127, 0, 0, 1 }; ///< Hex representation is 0x7F000001
        return loopback;
    }

    /// @brief Return the broadcast address 255.255.255.255
    /// @detail The broadcast address is an IP address used to target all systems on a specific subnet, instead of single hosts
    static const NetworkAddressV4& Broadcast() {
        static const NetworkAddressV4 broadcast{ 255, 255, 255, 255 }; ///< Hex representation is 0xFFFFFFFF
        return broadcast;
    }

    NetworkAddressV4(const std::array<Uint8_t, 4>& address)
    {
        setAddress(address);
    }

    NetworkAddressV4(Uint8_t add1, Uint8_t add2, Uint8_t add3, Uint8_t add4)
    {
        setAddress(add1, add2, add3, add4);
    }

    NetworkAddressV4(Uint32_t address)
    {
        setAddress(address);
    }

    /// @brief Get the address type
    virtual const AddressType getAddressType() const {
        return AddressType::kIpv4;
    }
    static constexpr AddressType GetAddressTypeStaticV4() {
        return AddressType::kIpv4;
    }

    /// @brief Populate the ASIO address with this address
    asio::ip::address_v4 toAsio() const {
        return toAsioBase<asio::ip::address_v4, AddressType::kIpv4>();
    }

    /// @brief Compare to an ASIO address
    bool operator==(const asio::ip::address& asioAddress) const {
        bool equivalent = false;
        if (asioAddress.is_v4()) {
            equivalent = (m_address == asioAddress.to_v4().to_bytes());
        }
        return equivalent;
    }

    friend bool operator==(const asio::ip::address& asioAddress, const NetworkAddressV4& address) {
        return address == asioAddress;
    }

    /// @brief Compare to an ASIO address
    bool operator!=(const asio::ip::address& asioAddress) const {
        return !operator==(asioAddress);
    }

    friend bool operator!=(const asio::ip::address& asioAddress, const NetworkAddressV4& address) {
        return address != asioAddress;
    }

private:
    /// @brief Set address from a std::array
    void setAddress(const std::array<Uint8_t, 4>& address) {
        m_address = address;
    }

    /// @brief Set address from four 8-bit integers
    void setAddress(Uint8_t add1, Uint8_t add2, Uint8_t add3, Uint8_t add4) {
        m_address[0] = add1;
        m_address[1] = add2;
        m_address[2] = add3;
        m_address[3] = add4;
    }

    /// @brief Set address from a 32-bit integer
    void setAddress(Uint32_t address) {
        memcpy(&m_address[0], &address, sizeof(Uint32_t));
    }
};

/// @class NetworkAddressV6
/// @brief Abstraction of an IPv6 address
/// @detail Stores a 128-bit network address, using the ASIO bytes type
class NetworkAddressV6 : public NetworkAddressInterface<std::array<Uint8_t, 16>> {
public:
    using NetworkAddressInterface::operator==; ///< So this class can use template definitions
    using NetworkAddressInterface::operator!=; ///< So this class can use template definitions
    using NetworkAddressInterface::operator std::string;

    /// @brief Return a V6 network address from an ASIO address
    static std::unique_ptr<NetworkAddressV6> FromAsio(const asio::ip::address& asioAddress) {
        /// \note address is stored in network byte order
        assert(asioAddress.is_v6());
        asio::ip::address_v6::bytes_type addressBytes = asioAddress.to_v6().to_bytes();
        return std::make_unique<NetworkAddressV6>(
            EndianConverter::NetworkToHost(addressBytes)
            );
    }

    /// @brief Return "any" address 0:0:0:0:0:0:0:0, corresponding to 0.0.0.0 in IPv4
    /// @detail The any address, INADDR_ANY, is a way of configuring servers to listen on any IPv4 address at all
    static const NetworkAddressV6& Any() {
        static const NetworkAddressV6 any{ std::array<Uint32_t, 4>{ 0, 0, 0, 0 } };
        return any;
    }

    /// @brief Return loopback address 0:0:0:0:0:0:0:1, corresponding to 127.0.0.1 in IPv4
    /// @detail The loopback address refers to the address of the current device, or host, attempting to access it
    static const NetworkAddressV6& Loopback() {
        static const NetworkAddressV6 loopback{ std::array<Uint8_t, 16>{ 0, 0, 0, 0,
            0, 0, 0, 0, 
            0, 0, 0, 0, 
            0, 0, 0, 1 } };
        return loopback;
    }

    NetworkAddressV6(const std::array<Uint8_t, 16> & address) {
        setAddress(address);
    }

    NetworkAddressV6(const std::array<Uint32_t, 4> & address) {
        setAddress(address);
    }

    /// @brief Get the address type
    virtual const AddressType getAddressType() const {
        return AddressType::kIpv6;
    }
    static constexpr AddressType GetAddressTypeStaticV6() {
        return AddressType::kIpv6;
    }

    /// @brief Populate the ASIO address with this address
    asio::ip::address_v6 toAsio() const {
        return toAsioBase<asio::ip::address_v6, AddressType::kIpv6>();
    }

    /// @brief Compare to an ASIO address
    bool operator==(const asio::ip::address& asioAddress) const {
        bool equivalent = false;
        if (asioAddress.is_v6()) {
            equivalent = m_address == asioAddress.to_v6().to_bytes();
        }
        return equivalent;
    }

    friend bool operator==(const asio::ip::address& asioAddress, const NetworkAddressV6& address) {
        return address == asioAddress;
    }

    /// @brief Compare to an ASIO address
    bool operator!=(const asio::ip::address& asioAddress) const {
        return !operator==(asioAddress);
    }

    friend bool operator!=(const asio::ip::address& asioAddress, const NetworkAddressV6& address) {
        return address != asioAddress;
    }

private:

    /// @brief Set address from an array
    template<typename T, Uint64_t N>
    void setAddress(const std::array<T, N>& address) {
        static constexpr Uint64_t numBytesToCopy = sizeof(T) * N;
        static_assert(numBytesToCopy == 16, "Address must be 16 bytes");
        memcpy(&m_address[0], &address[0], numBytesToCopy);
    }
};

} // End rev namespace
