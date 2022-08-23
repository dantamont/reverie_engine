#include <gtest/gtest.h>

#include <iostream>

#include "layline/network/GNetworkEndpoint.h"

namespace rev {

/// Tests
TEST(NetworkTests, ByteOrder) {
    // Expect no effect on an array with single-byte stride
    std::array<Uint8_t, 4> testArray = { 127, 0, 0, 1 };
    std::array<Uint8_t, 4> outArray = EndianConverter::NetworkToHost(testArray);
    EXPECT_EQ(outArray, testArray);

    // Test on a 32-bit int representation of the address
    Uint32_t littleEndianInt = (1 << 24) | (127 << 0);
    Uint32_t bigEndianInt = (127 << 24) | (1 << 0);
    Uint32_t swappedInt = EndianConverter::SwapEndianness(littleEndianInt);

    EXPECT_EQ(swappedInt, bigEndianInt);

    Uint32_t networkInt = EndianConverter::NetworkToHost(littleEndianInt);
    if (SystemMonitor::SystemEndianness::kNetwork == SystemMonitor::GetEndianness()) {
        EXPECT_EQ(networkInt, littleEndianInt);
    }
    else {
        EXPECT_EQ(networkInt, bigEndianInt);
    }
}

TEST(NetworkTests, Address) {
    /// Test IPv4
    asio::ip::address_v4 anyAddress = asio::ip::address_v4::any();
    NetworkAddressV4 myAnyAddress = NetworkAddressV4::Any();
    std::array<Uint8_t, 4> anyBytes = { 0, 0, 0, 0 };
    EXPECT_EQ(myAnyAddress.address(), anyBytes);
    EXPECT_EQ(myAnyAddress, anyAddress);
    EXPECT_NE(myAnyAddress, NetworkAddressV4::Loopback());
    EXPECT_NE(myAnyAddress, asio::ip::address_v4::loopback());

    /// Test Ipv6
    asio::ip::address_v6 anyAddress6 = asio::ip::address_v6::any();
    NetworkAddressV6 myAnyAddress6 = NetworkAddressV6::Any();
    std::array<Uint8_t, 16> anyBytes6 = { 0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0 };
    EXPECT_EQ(myAnyAddress6.address(), anyBytes6);
    EXPECT_EQ(myAnyAddress6, anyAddress6);
    EXPECT_NE(myAnyAddress6, NetworkAddressV6::Loopback());
    EXPECT_NE(myAnyAddress6, asio::ip::address_v6::loopback());

    // Test FromAsio
    std::array<Uint8_t, 4> loopback = { 127, 0, 0, 1 };
    std::unique_ptr<NetworkAddressV4> fromAsioV4 = NetworkAddressV4::FromAsio(asio::ip::address_v4::loopback());
    EXPECT_EQ(*fromAsioV4, asio::ip::address_v4::loopback());

    std::array<Uint32_t, 4> loopbackv6 = std::array<Uint32_t, 4>{ 0, 0, 0, 1 };
    std::unique_ptr<NetworkAddressV6> fromAsioV6 = NetworkAddressV6::FromAsio(asio::ip::address_v6::loopback());
    EXPECT_EQ(*fromAsioV6, asio::ip::address_v6::loopback());
}


TEST(NetworkTests, Endpoint) {
    // Test expected equivalences
    NetworkEndpoint loopback = NetworkEndpoint::Loopback(0);
    NetworkEndpoint loopback2 = NetworkEndpoint::Loopback(0);
    NetworkEndpoint loopbackDifferent = NetworkEndpoint::Loopback(1);
    EXPECT_EQ(loopback, loopback2);
    EXPECT_NE(loopback2, loopbackDifferent);

    // Test copy constructor
    NetworkEndpoint loopbackCopy = NetworkEndpoint(loopback);
    EXPECT_EQ(loopback, loopbackCopy);

    loopback.refresh<asio::ip::udp>();
    EXPECT_EQ(loopback, loopbackCopy);

    NetworkEndpoint any = NetworkEndpoint::Any(0);
    EXPECT_NE(any, loopback);

    // Test FromAsio
    asio::ip::udp::endpoint testUdpV4(asio::ip::address_v4::loopback(), 13);
    NetworkEndpoint fromAsioUdpV4 = NetworkEndpoint::FromAsio(testUdpV4);
    EXPECT_EQ(fromAsioUdpV4, NetworkEndpoint::Loopback(13));

    asio::ip::udp::endpoint testUdpV6(asio::ip::address_v6::loopback(), 13);
    NetworkEndpoint fromAsioUdpV6 = NetworkEndpoint::FromAsio(testUdpV6);
    EXPECT_EQ(fromAsioUdpV6, NetworkEndpoint::Loopback<NetworkAddress::AddressType::kIpv6>(13));

    asio::ip::tcp::endpoint testTcpV4(asio::ip::address_v4::loopback(), 13);
    NetworkEndpoint fromAsioTcpV4 = NetworkEndpoint::FromAsio(testTcpV4);
    EXPECT_EQ(fromAsioTcpV4, NetworkEndpoint::Loopback(13));

    asio::ip::tcp::endpoint testTcpV6(asio::ip::address_v6::loopback(), 13);
    NetworkEndpoint fromAsioTcpV6 = NetworkEndpoint::FromAsio(testTcpV6);
    EXPECT_EQ(fromAsioTcpV6, NetworkEndpoint::Loopback<NetworkAddress::AddressType::kIpv6>(13));
}

} /// End rev namespace