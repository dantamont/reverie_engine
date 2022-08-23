#include <gtest/gtest.h>

#include <iostream>
#include "fortress/math/GRandom.h"
#include "fortress/time/GSystemClock.h"
#include "layline/transport/tcp/GTcpServer.h"
#include "layline/transport/tcp/GTcpClient.h"

namespace rev {

/// Tests

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
        try {
            deserializer.read(m_receiveBuffer.data(), bufferIndex);
        }
        catch (std::runtime_error& e) {
#ifdef DEBUG_MODE
            static constexpr Uint64_t previewSize = 100;
            std::vector<Uint64_t> bufferVec = std::vector(
                reinterpret_cast<const Uint64_t*>(&m_receiveBuffer[0]), 
                reinterpret_cast<const Uint64_t*>(&m_receiveBuffer[0]) + previewSize);
#endif
            return false;
        }

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

void testServerClientTransport(Int32_t index) {
    // Create a mailbox for the server
    TestMailbox serverMailbox;

    // Start server, listening over any address on the test port
    NetworkPort testPort = NetworkPort(NetworkPort::LowestUserPortNumber() + index);
    NetworkEndpoint endpoint = NetworkEndpoint::Any(testPort);
    TcpServer server;
    server.addListenerTransport(&serverMailbox, endpoint);
    server.run();

    // Create a mailbox for the client
    TestMailbox clientMailbox;

    // Add messages to the client mailbox to send
    constexpr Int32_t numMessages = 10000;
    std::vector<TestMessage*> clientMessages;
    for (Uint32_t i = 0; i < numMessages; i++) {
        clientMessages.push_back(new TestMessage);
        clientMessages.back()->header() = { i, 0 };
    }
    for (TestMessage* msg : clientMessages) {
        clientMailbox.push(msg);
    }

    // Start client, sending to the loopback address at the rest port
    TcpClient client(&clientMailbox, 0);
    NetworkEndpoint remoteServerEndpoint = NetworkEndpoint::Loopback(testPort);
    client.connectRemotely(remoteServerEndpoint);
    client.startSend();
    client.run();

    // Wait just a bit so stuff has time to run
    while (serverMailbox.receiveQueueCount() < numMessages) {
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
    EXPECT_EQ(serverMessages.size(), numMessages);
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

TEST(TcpTests, TestServerClientTransport) {
    std::cout << "Running server-client test ----------------: " << "\n";
    try
    {
        constexpr Uint32_t numTimesToTest = 1;
        for (Uint32_t j = 0; j < numTimesToTest; j++) {
            testServerClientTransport(j);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
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

TEST(TcpTests, TestServerToServerTransport) {
    std::cout << "Running server-server test ----------------: " << "\n";
    try
    {
        // Create a mailbox for the server
        TestMailbox serverMailbox;

        /// Start listening server, listening over any address on the test port
        NetworkPort testPort = NetworkPort::UserPort();
        NetworkEndpoint endpoint = NetworkEndpoint::Any(testPort);
        TcpServer server;

        // Add a transport for receiving over an endpoint
        server.addListenerTransport(&serverMailbox, endpoint);
        server.run();

        // Create a mailbox for the client (sending) server
        TestMailbox clientServerMailbox;

        // Add messages to the client server mailbox to send
        constexpr Int32_t numMessages = 10000;
        std::vector<TestMessage*> clientMessages;
        for (Uint32_t i = 0; i < numMessages; i++) {
            clientMessages.push_back(new TestMessage);
            clientMessages.back()->header() = { i, 0 };
        }
        for (TestMessage* msg : clientMessages) {
            clientServerMailbox.push(msg);
        }

        // Create and start client server, sending to the loopback address at the rest port
        NetworkEndpoint remoteServerEndpoint = NetworkEndpoint::Loopback(testPort); // Endpoint to send to listening server
        TcpServer clientServer;

        // Add a transport for sending over an endpoint
        clientServer.addSendTransport(&clientServerMailbox, remoteServerEndpoint, 0);
        clientServer.run();

        // Wait just a bit so stuff has time to run
        while (serverMailbox.receiveQueueCount() < numMessages) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        clientServer.shutdown();
        server.shutdown();

        // Retrieve messages from the server, which should match the client ones
        std::vector<TestMessage*> serverMessages;
        while (TestMessage* msg =
            static_cast<TestMessage*>(serverMailbox.retrieve())) {
            serverMessages.push_back(msg);
        }

        // Test that messages in server match those sent by client
        EXPECT_EQ(serverMessages.size(), numMessages);
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
        std::vector<std::string> clientConnectionInfo = clientServer.transports().back()->socketHolder().connectionInfo();

        std::cout << "Listener server Info: " << "\n";
        for (const auto& str : serverConnectionInfo) {
            std::cout << str << "\n";
        }

        std::cout << "Client server Info: " << "\n";
        for (const auto& str : clientConnectionInfo) {
            std::cout << str << "\n";
        }

        EXPECT_EQ(server.transports().back()->socketHolder().connectionCount(), 1);
        EXPECT_EQ(clientServer.transports().back()->socketHolder().connectionCount(), 1);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...) {
        assert(false);
    }
}

TEST(TcpTests, TestIntraServerTransport) {
    try
    {
        // Create a mailbox for the server
        TestMailbox serverMailbox;

        /// Start listening server, listening over any address on the test port
        NetworkPort testPort = NetworkPort::UserPort();
        NetworkEndpoint endpoint = NetworkEndpoint::Any(testPort);
        TcpServer server;

        // Add a transport for receiving over an endpoint
        server.addListenerTransport(&serverMailbox, endpoint);

        // Add messages to the server mailbox to send
        std::vector<TestMessage*> clientMessages;
        constexpr Int32_t numMessages = 10000;
        for (Uint32_t i = 0; i < numMessages; i++) {
            clientMessages.push_back(new TestMessage);
            clientMessages.back()->header() = { i, 0 };
        }
        for (TestMessage* msg : clientMessages) {
            serverMailbox.push(msg);
        }

        // Add a transport to the same server, but this time for sending over an endpoint
        NetworkEndpoint remoteServerEndpoint = NetworkEndpoint::Loopback(testPort); // Endpoint to send to listening server
        server.addSendTransport(&serverMailbox, remoteServerEndpoint, 0);
        server.run();

        // Wait just a bit so stuff has time to run
        while (serverMailbox.receiveQueueCount() < numMessages) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        server.shutdown();

        // Retrieve messages from the server, which should match the client ones
        std::vector<TestMessage*> serverMessages;
        while (TestMessage* msg =
            static_cast<TestMessage*>(serverMailbox.retrieve())) {
            serverMessages.push_back(msg);
        }

        // Test that messages in server match those sent by client
        EXPECT_EQ(serverMessages.size(), numMessages);
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

        std::cout << "Server Info: " << "\n";
        for (const auto& str : serverConnectionInfo) {
            std::cout << str << "\n";
        }

        EXPECT_EQ(server.transports().back()->socketHolder().connectionCount(), 1);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...) {
        assert(false);
    }
}

/// @brief Test timing with low precision step (~10Hz)
TEST(TcpTests, TestSlowTiming) {
    try
    {
        // Create a mailbox for the server
        TestMailbox serverMailbox;

        // Start server
        NetworkPort testPort = NetworkPort::UserPort();
        NetworkEndpoint endpoint = NetworkEndpoint::Any(testPort);
        TcpServer server;
        server.addListenerTransport(&serverMailbox, endpoint);
        server.run();

        // Create a mailbox for the client
        TestMailbox clientMailbox;

        // Add messages to the client mailbox to send
        constexpr Int32_t messageCount = 20;
        std::vector<TestMessage*> clientMessages;
        for (Uint32_t i = 0; i < messageCount; i++) {
            clientMessages.push_back(new TestMessage);
            clientMessages.back()->header() = { i, SystemClock::GetUtcTimeMicroseconds() };
        }
        for (TestMessage* msg : clientMessages) {
            clientMailbox.push(msg);
        }

        // Start client
        Uint64_t sendIntervalMicroseconds = 100e3; ///< 100 ms
        TcpClient client(&clientMailbox, sendIntervalMicroseconds);
        NetworkEndpoint remoteServerEndpoint = NetworkEndpoint::Loopback(testPort);
        //client.bindForListen(NetworkEndpoint::Any(0), false);
        client.connectRemotely(remoteServerEndpoint);
        client.startSend();
        client.run();

        // Wait just a bit so stuff has time to run
        while (serverMailbox.receiveQueueCount() < messageCount) {
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
        EXPECT_EQ(serverMessages.size(), messageCount);
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

        // Expect a time-step of sendIntervalMicroseconds
        Uint64_t timeStep = timeDifferenceSumMicroseconds / (serverMessages.size() - 1);
        Uint64_t timeDifference = abs((Int64_t)timeStep - (Int64_t)sendIntervalMicroseconds);
        EXPECT_LE(timeDifference, 10e3); // Only expecting ~10ms precision here
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}


/// @brief Test timing with high precision busy sleep
TEST(TcpTests, TestBusyTiming) {
    try
    {
        // Create a mailbox for the server
        TestMailbox serverMailbox;

        // Start server
        NetworkPort testPort = NetworkPort::UserPort();
        NetworkEndpoint endpoint = NetworkEndpoint::Any(testPort);
        TcpServer server;
        server.addListenerTransport(&serverMailbox, endpoint);
        server.run();

        // Create a mailbox for the client
        TestMailbox clientMailbox;


        // Add messages to the client mailbox to send
        std::vector<TestMessage*> clientMessages;
        for (Uint32_t i = 0; i < 10; i++) {
            clientMessages.push_back(new TestMessage);
            clientMessages.back()->header() = { i, SystemClock::GetUtcTimeMicroseconds() };
        }
        for (TestMessage* msg : clientMessages) {
            clientMailbox.push(msg);
        }

        // Start client
        Uint64_t sendIntervalMicroseconds = 10 * 1e3; ///< 10 ms
        TcpClient client(&clientMailbox, sendIntervalMicroseconds);
        NetworkEndpoint remoteServerEndpoint = NetworkEndpoint::Loopback(testPort);
        //client.bindForListen(NetworkEndpoint::Any(0), false);
        client.connectRemotely(remoteServerEndpoint);
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
        Uint64_t timeStep = timeDifferenceSumMicroseconds / (serverMessages.size() - 1);
        Uint64_t timeDifference = abs(timeStep - 10 * 1e3);
        EXPECT_LE(timeDifference, 500);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}


} /// End rev namespace