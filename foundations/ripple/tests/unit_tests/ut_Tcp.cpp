#include <gtest/gtest.h>

#include <iostream>

#include "fortress/math/GRandom.h"
#include "fortress/thread/GThreadpool.h"
#include "fortress/time/GSystemClock.h"

#include "layline/transport/tcp/GTcpServer.h"
#include "layline/transport/tcp/GTcpClient.h"

#include "ripple/network/messages/GJsonMessage.h"
#include "ripple/network/messages/GRenderSettingsInfoMessage.h"
#include "ripple/network/messages/GRequestRenderSettingsInfoMessage.h"
#include "ripple/network/messages/GRequestTransformMessage.h"
#include "ripple/network/messages/GGetScenarioJsonMessage.h"
#include "ripple/network/messages/GScenarioJsonMessage.h"
#include "ripple/network/messages/GMailbox.h"
#include "ripple/network/gateway/GMessageGateway.h"

namespace rev {

template<typename MessageType>
void testServerClientTransport(Int32_t index) {
    // Create a mailbox for the server
    Mailbox serverMailbox;

    // Start server, listening over any address on the test port
    NetworkPort testPort = NetworkPort(NetworkPort::LowestUserPortNumber() + index);
    NetworkEndpoint endpoint = NetworkEndpoint::Any(testPort);
    TcpServer server;
    server.addListenerTransport(&serverMailbox, endpoint);
    server.run();

    // Create a mailbox for the client
    Mailbox clientMailbox;

    json testJson = { {"one", 1}, {"name", "george"} };

    // Add messages to the client mailbox to send
    constexpr Int32_t numMessages = 10000;
    std::vector<MessageType*> clientMessages;
    for (Uint32_t i = 0; i < numMessages; i++) {
        clientMessages.push_back(new MessageType());
        clientMessages.back()->postConstruction();
        if constexpr (std::is_same_v<GJsonMessage, MessageType>) {
            clientMessages.back()->setJsonBytes(GJson::ToBytes(testJson));
        }
    }
    for (MessageType* msg : clientMessages) {
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
    std::vector<MessageType*> serverMessages;
    while (MessageType* msg =
        static_cast<MessageType*>(serverMailbox.retrieve())) {
        serverMessages.push_back(msg);
    }

    // Test that messages in server match those sent by client
    EXPECT_EQ(serverMessages.size(), numMessages);
    Uint32_t i = 0;
    MessageType* lastMessage = nullptr;
    Uint64_t timeDifferenceSumMicroseconds = 0;
    for (MessageType* msg : serverMessages) {
        GMessageHeader& header = msg->header();
        if (lastMessage) {
            timeDifferenceSumMicroseconds +=
                header.m_sendTimeUtcMicroseconds - lastMessage->header().m_sendTimeUtcMicroseconds;
        }
        EXPECT_NE(header.m_sendTimeUtcMicroseconds, 0);
        if constexpr (std::is_same_v<GJsonMessage, MessageType>) {
            json bodyJson = GJson::FromBytes(msg->getJsonBytes());
            EXPECT_EQ(bodyJson["one"].get<Int32_t>(), Int32_t(1));
            EXPECT_EQ(bodyJson["name"].get_ref<const std::string&>(), "george");
        }
        lastMessage = msg;
        i++;
    }

    // Expect a time-step less than 10 ms
    Uint64_t atLeastOne = std::max(Uint64_t(1), serverMessages.size() - 1);
    Uint64_t timeStep = timeDifferenceSumMicroseconds / atLeastOne;
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

TEST(RippleTcpTests, TestServerClientTransport) {
    std::cout << "Running server-client test ----------------: " << "\n";
    try
    {
        constexpr Uint32_t numTimesToTest = 1;
        for (Uint32_t j = 0; j < numTimesToTest; j++) {
            testServerClientTransport<GJsonMessage>(j);
            testServerClientTransport<GRequestRenderSettingsInfoMessage>(j);
            testServerClientTransport<GRenderSettingsInfoMessage>(j);
            testServerClientTransport<GRequestTransformMessage>(j);
            testServerClientTransport<GGetScenarioJsonMessage>(j);
            testServerClientTransport<GScenarioJsonMessage>(j);
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

template<typename MessageType, typename MessageType2>
void testInterleavedServerClientTransport(Int32_t index) {
    // Create a mailbox for the server
    Mailbox serverMailbox;

    // Start server, listening over any address on the test port
    NetworkPort testPort = NetworkPort(NetworkPort::LowestUserPortNumber() + index);
    NetworkEndpoint endpoint = NetworkEndpoint::Any(testPort);
    TcpServer server;
    server.addListenerTransport(&serverMailbox, endpoint);
    server.run();

    // Create a mailbox for the client
    Mailbox clientMailbox;

    // Add messages to the client mailbox to send
    constexpr Int32_t numMessages = 5000;
    std::vector<GMessage*> clientMessages;
    for (Uint32_t i = 0; i < numMessages; i++) {
        clientMessages.push_back(new MessageType());
        clientMessages.back()->postConstruction();
        clientMessages.push_back(new MessageType2());
        clientMessages.back()->postConstruction();
    }
    for (GMessage* msg : clientMessages) {
        clientMailbox.push(msg);
    }

    // Start client, sending to the loopback address at the rest port
    TcpClient client(&clientMailbox, 0);
    NetworkEndpoint remoteServerEndpoint = NetworkEndpoint::Loopback(testPort);
    client.connectRemotely(remoteServerEndpoint);
    client.startSend();
    client.run();

    // Wait just a bit so stuff has time to run
    while (serverMailbox.receiveQueueCount() < numMessages * 2) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    client.shutdown();
    server.shutdown();

    // Retrieve messages from the server, which should match the client ones
    std::vector<GMessage*> serverMessages;
    while (GMessage* msg = static_cast<GMessage*>(serverMailbox.retrieve())) {
        serverMessages.push_back(msg);
    }

    // Test that messages in server match those sent by client
    EXPECT_EQ(serverMessages.size(), numMessages * 2);
    Uint32_t i = 0;
    GMessage* lastMessage = nullptr;
    Uint64_t timeDifferenceSumMicroseconds = 0;
    for (GMessage* msg : serverMessages) {
        GMessageHeader& header = msg->header();
        if (lastMessage) {
            timeDifferenceSumMicroseconds +=
                header.m_sendTimeUtcMicroseconds - lastMessage->header().m_sendTimeUtcMicroseconds;
        }
        EXPECT_NE(header.m_sendTimeUtcMicroseconds, 0);
        lastMessage = msg;
        i++;
    }

    // Expect a time-step less than 10 ms
    Uint64_t atLeastOne = std::max(Uint64_t(1), serverMessages.size() - 1);
    Uint64_t timeStep = timeDifferenceSumMicroseconds / atLeastOne;
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

TEST(RippleTcpTests, TestInterleavedServerClientTransport) {
    std::cout << "Running server-client test ----------------: " << "\n";
    try
    {
        constexpr Uint32_t numTimesToTest = 1;
        for (Uint32_t j = 0; j < numTimesToTest; j++) {
            testInterleavedServerClientTransport<GGetScenarioJsonMessage, GScenarioJsonMessage>(j);
            testInterleavedServerClientTransport<GScenarioJsonMessage, GRenderSettingsInfoMessage>(j);
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

class TestApplicationGateway: public GMessageGateway {
public:
    virtual void processMessages() override {
        ThreadSafeCout{} << "Processing messages" << "\n";
    }
};

class TestWidgetGateway : public GMessageGateway {
public:
    virtual void processMessages() override {
        ThreadSafeCout{} << "Processing messages" << "\n";
    }
};

TEST(RippleTcpTests, TestGateways) {
    Uint32_t applicationPort = 10000;
    Uint32_t widgetPort = 10001;

    // Initialize communication server settings for application
    GMessageGateway::ServerSettings applicationServerSettings;
    applicationServerSettings.m_listenerEndpoint = NetworkEndpoint::Any(applicationPort);
    applicationServerSettings.m_destinationEndpoint = NetworkEndpoint::Loopback(widgetPort);
    applicationServerSettings.m_sendTimeMicroseconds = 0;
    applicationServerSettings.m_clientConnectionTimeoutMicroSeconds = 5e6;

    auto applicationGateway = std::make_unique<TestApplicationGateway>();

    // Need to initialize server on a separate thread so that initialization operations are non-blocking
    //applicationGateway->initializeServer(applicationServerSettings);
    std::chrono::system_clock::time_point tenSecondsElapsed = std::chrono::system_clock::now() + std::chrono::seconds(10);
    ThreadPool threadPool{ 2 };
    std::future<void> completes = threadPool.addTask(
        &TestApplicationGateway::initializeServer,
        applicationGateway.get(),
        applicationServerSettings);

    // Initialize communication server settings for widgets
    GMessageGateway::ServerSettings widgetServerSettings;
    widgetServerSettings.m_listenerEndpoint = NetworkEndpoint::Any(widgetPort);
    widgetServerSettings.m_destinationEndpoint = NetworkEndpoint::Loopback(applicationPort);
    widgetServerSettings.m_sendTimeMicroseconds = 0;
    widgetServerSettings.m_clientConnectionTimeoutMicroSeconds = 5e6;

    auto widgetGateway = std::make_unique<TestWidgetGateway>();

    
    // Add messages to the widget gateway mailbox to send
    constexpr Int32_t numMessages = 10000;
    std::vector<GJsonMessage*> clientMessages;
    for (Uint32_t i = 0; i < numMessages; i++) {
        clientMessages.push_back(new GJsonMessage());
        clientMessages.back()->postConstruction();
    }
    for (GJsonMessage* msg : clientMessages) {
        widgetGateway->queueMessageForSend(msg);
    }


    // Need to initialize server on a separate thread so that initialization operations are non-blocking
    //widgetGateway->initializeServer(widgetServerSettings);
    std::future<void> completesWidgets = threadPool.addTask(
        &TestWidgetGateway::initializeServer,
        widgetGateway.get(),
        widgetServerSettings);

    // Wait until gateways are initialized, then shut down those threads
    //completes.wait_until(tenSecondsElapsed); // Waits until either done, or 10 seconds pass
    //completesWidgets.wait_until(tenSecondsElapsed); // Waits until either done, or 10 seconds pass
    threadPool.shutdown(); // Similar thread locking to futures

    // Wait just a bit so gateways have time to run
    while (applicationGateway->receiveQueueCount() < numMessages) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    applicationGateway->close();
    widgetGateway->close();

    // Retrieve messages from the application server, which should match the widget ones
    std::vector<GJsonMessage*> serverMessages;
    while (GJsonMessage* msg =
        static_cast<GJsonMessage*>(applicationGateway->popReceivedMessage())) {
        serverMessages.push_back(msg);
    }

    // Test that messages in server match those sent by client
    EXPECT_EQ(serverMessages.size(), numMessages);
    Uint32_t i = 0;
    GJsonMessage* lastMessage = nullptr;
    Uint64_t timeDifferenceSumMicroseconds = 0;
    for (GJsonMessage* msg : serverMessages) {
        GMessageHeader& header = msg->header();
        if (lastMessage) {
            timeDifferenceSumMicroseconds +=
                header.m_sendTimeUtcMicroseconds - lastMessage->header().m_sendTimeUtcMicroseconds;
        }
        EXPECT_NE(header.m_sendTimeUtcMicroseconds, 0);
        lastMessage = msg;
        i++;
    }
}


} /// End rev namespace