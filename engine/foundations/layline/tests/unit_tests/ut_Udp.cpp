#include <gtest/gtest.h>

#include <iostream>
#include "fortress/time/GSystemClock.h"
#include "layline/transport/GTransportInterface.h"
#include "layline/transport/udp/GUdpListenerTransport.h"
#include "layline/transport/udp/GUdpServer.h"
#include "layline/transport/udp/GUdpClient.h"

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
/// @see https://think-async.com/Asio/asio-1.20.0/doc/asio/overview/networking/protocols.html
/// @see https://think-async.com/Asio/asio-1.20.0/doc/asio/overview/networking/bsd_sockets.html
/// @see https://stackoverflow.com/questions/4298167/boost-asio-how-to-keep-a-client-connection-alive
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static constexpr Uint32_t s_numClientsForSimultaneousTest = 5;


struct TestHeader {
    Uint32_t m_number{ 0 };
    mutable Uint64_t m_sendTimeUtcMicroseconds{ 0 }; ///< The time since Utc when the message was sent, in microseconds
};

/// @brief Class for testing transport message
class TestMessage : public TransportMessage<TestHeader> {
public:
    using TransportMessage<TestHeader>::TransportMessage;

    virtual Uint32_t pack(Uint8_t* buffer, Uint32_t index = 0) const override {
        m_header.m_sendTimeUtcMicroseconds = SystemClock::GetUtcTimeMicroseconds();
        return TransportMessage<TestHeader>::pack(buffer, index);
    }

    operator std::string() const {
        return GString::Format("Test message:\n   Size: %d bytes\n   Index: %d", 
            getSerializedSizeInBytes(),
            m_header.m_number
            ).c_str();
    }
};

/// @brief Class for testing mailbox interface
class TestMailbox : public TransportMailboxInterface<1, s_numClientsForSimultaneousTest> {
public:

    using TransportMailboxInterface::TransportMailboxInterface;

    /// @brief The size of the message headers used by this mailbox
    virtual Uint32_t messageHeaderSizeBytes() const override {
        return sizeof(TestHeader) + sizeof(Uint64_t)/*for size of count*/;
    }

    /// @brief Return the number of bytes expected from the next message body
    virtual Uint32_t calculateMessageBodySizeBytes() override {
        return 0;
    }

protected:

    /// @brief Check the validity of the contents of the receive buffer
    /// @detail Called before unpacking a message from the receive buffer. Invalid contents are ignored
    virtual bool isReceiveBufferValid(Uint32_t bufferIndex) override {
        // Return if exceeded maximum buffer size or reached null terminator
        static constexpr Size_t maxSize = ReceiveMessageBuffer::GetMaxSizeBytes();
        if (bufferIndex >= maxSize) {
            return false;
        }
        else if (0 == m_receiveBuffer[bufferIndex]) {
            return false;
        }

        TestHeader header;
        SerializationProtocol<TestHeader> deserializer(header);
        deserializer.read(m_receiveBuffer.data(), bufferIndex);
        Int32_t diff = header.m_number - m_lastMessageIndex;
        bool valid = header.m_number > m_lastMessageIndex;
        if (valid) {
            if (diff > 1) {
                ThreadSafeCout{} << "Skipped a message, test will fail: "
                    << "Last: " << m_lastMessageIndex << ", Now:"
                    << header.m_number;
            }
            m_lastMessageIndex = header.m_number;
        }
        return valid;
    }

    /// @brief Called when unpacking from the receive buffer into a message
    /// @detail Override this to create a message of the appropriate TransportMessageInterface subclass
    virtual TransportMessageInterface* unpackMessage(Uint32_t bufferIndex, Uint32_t& outBytesRead) const {
        TestMessage* message = new TestMessage();
        outBytesRead = message->unpack(m_receiveBuffer.data(), bufferIndex);
        return message;
    }

    Int64_t m_lastMessageIndex = -1; // The index of the most recently received message
};

TEST(UdpTests, TestTransport) {
    try
    {
        // Create a mailbox for the server
        TestMailbox serverMailbox;

        // Start server
        NetworkPort testPort = NetworkPort::LowestUserPortNumber();
        NetworkEndpoint endpoint = NetworkEndpoint::Any(testPort);
        UdpServer server;
        server.addTransport(&serverMailbox, endpoint, 0);
        server.run();

        // Create a mailbox for the client
        TestMailbox clientMailbox;

        // Add messages to the client mailbox to send
        std::vector<TestMessage*> clientMessages;
        for (Uint32_t i = 0; i < 10; i++) {
            clientMessages.push_back(new TestMessage);
            clientMessages.back()->header() = { i, 0 };
        }
        for (TestMessage* msg : clientMessages) {
            clientMailbox.push(msg);
        }

        // Start client
        UdpClient client(&clientMailbox, 0);
        NetworkEndpoint remoteServerEndpoint = NetworkEndpoint::Loopback(testPort);
        client.openSocket<NetworkAddress::AddressType::kIpv4>();
        client.addRemoteConnection(remoteServerEndpoint); /// Set up to send to server
        client.setSendConnection(0);
        client.startSend();
        client.run();

        // Wait just a bit so stuff has time to run
        while (serverMailbox.receiveQueueCount() < 10) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        client.shutdown();
        server.shutdown();

        // Retrieve messages from the server, which should match the client ones
        std::vector<TestMessage*> serverMessages;
        while (TestMessage* msg = 
            static_cast<TestMessage*>(serverMailbox.retrieve())) {
            serverMessages.push_back(msg);
        }

        // Test that messages in server match those sent by client
        EXPECT_EQ(serverMessages.size(), 10);
        Uint32_t i = 0;
        TestMessage* lastMessage = nullptr;
        Uint64_t timeDifferenceSumMicroseconds = 0;
        for (TestMessage* msg : serverMessages) {
            if (lastMessage) {
                timeDifferenceSumMicroseconds += 
                    msg->header().m_sendTimeUtcMicroseconds - lastMessage->header().m_sendTimeUtcMicroseconds;
            }
            EXPECT_EQ(msg->header().m_number, i);
            EXPECT_NE(msg->header().m_sendTimeUtcMicroseconds, 0);
            lastMessage = msg;
            i++;
        }

        // Expect a time-step less than 10 ms
        Uint64_t timeStep = timeDifferenceSumMicroseconds / (serverMessages.size() - 1);
        EXPECT_LE(timeStep, 10 * 1e3); 

        // Test number of connections
        std::vector<std::string> serverConnectionInfo = server.transports().back()->socketHolder().connectionInfo();
        std::vector<std::string> clientConnectionInfo = client.socketHolder().connectionInfo();

        std::cout << "Server Info: " << "\n";
        for (const auto& str : serverConnectionInfo) {
            std::cout << str << "\n";
        }

        std::cout << "Client Info: " << "\n";
        for (const auto& str : clientConnectionInfo) {
            std::cout << str << "\n";
        }

        EXPECT_EQ(server.transports().back()->socketHolder().connectionCount(), 1);
        EXPECT_EQ(client.socketHolder().connectionCount(), 1);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...) {
        assert(false);
    }
}

TEST(UdpTests, TestTiming) {
    try
    {
        // Create a mailbox for the server
        TestMailbox serverMailbox;

        // Start server
        NetworkPort testPort = NetworkPort::LowestUserPortNumber();
        NetworkEndpoint endpoint = NetworkEndpoint::Any(testPort);
        UdpServer server;
        server.addTransport(&serverMailbox, endpoint, 0);
        server.run();

        // Create a mailbox for the client
        TestMailbox clientMailbox;


        // Add messages to the client mailbox to send
        std::vector<TestMessage*> clientMessages;
        for (Uint32_t i = 0; i < 10; i++) {
            clientMessages.push_back(new TestMessage);
            clientMessages.back()->header() = { i, SystemClock::GetUtcTimeMicroseconds()};
        }
        for (TestMessage* msg : clientMessages) {
            clientMailbox.push(msg);
        }

        // Start client
        Uint64_t sendIntervalMicroseconds = 10 * 1e3; ///< 10 ms
        UdpClient client(&clientMailbox, sendIntervalMicroseconds);
        NetworkEndpoint remoteServerEndpoint = NetworkEndpoint::Loopback(testPort);
        client.openSocket<NetworkAddress::AddressType::kIpv4>();
        client.addRemoteConnection(remoteServerEndpoint);
        client.setSendConnection(0);
        client.startSend();
        client.run();

        // Wait just a bit so stuff has time to run
        while (serverMailbox.receiveQueueCount() < 10) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        server.shutdown();
        client.shutdown();

        // Retrieve messages from the server, which should match the client ones
        std::vector<TestMessage*> serverMessages;
        while (TestMessage* msg =
            static_cast<TestMessage*>(serverMailbox.retrieve())) {
            serverMessages.push_back(msg);
        }

        // Test that messages in server match those sent by client
        EXPECT_EQ(serverMessages.size(), 10);
        Uint32_t i = 0;
        TestMessage* lastMessage = nullptr;
        Uint64_t timeDifferenceSumMicroseconds = 0;
        for (TestMessage* msg : serverMessages) {
            if (lastMessage) {
                timeDifferenceSumMicroseconds +=
                    msg->header().m_sendTimeUtcMicroseconds - lastMessage->header().m_sendTimeUtcMicroseconds;
            }
            EXPECT_EQ(msg->header().m_number, i);
            EXPECT_NE(msg->header().m_sendTimeUtcMicroseconds, 0);
            lastMessage = msg;
            i++;
        }

        // Expect a time-step of 10 ms
        static constexpr Int64_t s_expectedTimeStepMicroseconds = (10 * 1e3);
        Int64_t timeStepMicroSeconds = timeDifferenceSumMicroseconds / (serverMessages.size() - 1);
        Int64_t timeDifference = abs(timeStepMicroSeconds - s_expectedTimeStepMicroseconds);
        EXPECT_LE(timeDifference, 500);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

TEST(UdpTests, TestMultipleClients) {
    try
    {
        // Create a mailbox for the server
        TestMailbox serverMailbox;

        // Start server
        NetworkPort testPort = NetworkPort::LowestUserPortNumber();
        NetworkEndpoint endpoint = NetworkEndpoint::Any(testPort);
        UdpServer server;
        server.addTransport(&serverMailbox, endpoint, 0);
        server.run();

        // Create multiple clients
        std::vector<std::shared_ptr<UdpClient>> clients;
        std::vector<std::shared_ptr<TestMailbox>> mailboxes;
        std::vector<TestMessage*> clientMessages;
        Uint32_t waitCount = 0;
        for (Uint32_t i = 0; i < s_numClientsForSimultaneousTest; i++) {

            // Create messages for the client mailbox to send
            clientMessages.clear();
            for (Uint32_t j = 10*i; j < (10*i + 10); j++) {
                clientMessages.push_back(new TestMessage);
                clientMessages.back()->header() = { j, 0 };
            }

            // Create a mailbox for the client
            std::shared_ptr<TestMailbox> clientMailbox = std::make_shared<TestMailbox>();

            for (TestMessage* msg : clientMessages) {
                clientMailbox->push(msg);
            }

            // Start client
            /// @note packets simply get dropped (at least I think that's why it doesn't work)
            /// when the send interval is too small. The solution to this is to actually implement
            /// a mechanism for handling dropped packets
            Uint64_t sendIntervalMicroseconds = 10 * 1e3; ///< 10 ms
            std::shared_ptr<UdpClient> client = std::make_shared<UdpClient>(
                clientMailbox.get(), sendIntervalMicroseconds);
            NetworkEndpoint remoteServerEndpoint = NetworkEndpoint::Loopback(testPort);
            client->openSocket<NetworkAddress::AddressType::kIpv4>();
            client->addRemoteConnection(remoteServerEndpoint);
            client->setSendConnection(0);
            client->startSend();
            client->run();

            // Wait just a bit so stuff has time to run
            while (serverMailbox.receiveQueueCount() < (10 * i + 10)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                waitCount++;
                std::cout << "Waiting to receive messages\n";
            }

            mailboxes.push_back(clientMailbox);
            clients.push_back(client);
            waitCount = 0;
        }

        // Shut down clients and server
        for (const auto& client : clients) {
            client->shutdown();
        }
        server.shutdown();

        // Retrieve messages from the server, which should match the client ones
        std::vector<TestMessage*> serverMessages;
        while (TestMessage* msg =
            static_cast<TestMessage*>(serverMailbox.retrieve())) {
            serverMessages.push_back(msg);
        }

        // Test that messages in server match those sent by clients
        EXPECT_EQ(serverMessages.size(), 10 * s_numClientsForSimultaneousTest);
        Uint32_t i = 0;
        TestMessage* lastMessage = nullptr;
        Uint64_t timeDifferenceSumMicroseconds = 0;
        for (TestMessage* msg : serverMessages) {
            if (lastMessage) {
                timeDifferenceSumMicroseconds +=
                    msg->header().m_sendTimeUtcMicroseconds - lastMessage->header().m_sendTimeUtcMicroseconds;
            }
            EXPECT_EQ(msg->header().m_number, i);
            EXPECT_NE(msg->header().m_sendTimeUtcMicroseconds, 0);
            lastMessage = msg;
            i++;
        }

        // Test number of connections
        EXPECT_EQ(server.transports().back()->socketHolder().connectionCount(), s_numClientsForSimultaneousTest);
        for (const auto& client : clients) {
            EXPECT_EQ(client->socketHolder().connectionCount(), 1);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...) {
        assert(false);
    }
}

} /// End rev namespace