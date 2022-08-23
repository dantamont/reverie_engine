#pragma once

#include <iostream>
#include <array>
#include <shared_mutex>

#include "fortress/logging/GThreadSafeCout.h"
#include "layline/network/GNetworkEndpoint.h"
#include "layline/transport/GSocketHolder.h"
#include "layline/transport/GTransportConnection.h"
#include "layline/transport/messages/GTransportMailboxInterface.h"
#include "layline/transport/GTransportThread.h"
#include "logging/GLogger.h"

namespace rev {

/// @class TransportInterface
/// @brief Class representing an abstraction of a networking server or client, e.g. A Tcp/IP server
/// @tparam AsioProtocolType the class used to represent a ASIO networking protocol type
/// @tparam TransportT the type of transport to begin communication to a network as (either a server or client)
/// @detail This class is not specific to any network protocol
/// @todo Add check that only a single TCP/IP and/or UDP socket is open per port
/// @see https://stackoverflow.com/questions/3329641/how-do-multiple-clients-connect-simultaneously-to-one-port-say-80-on-a-server
/// @see https://stackoverflow.com/questions/16508685/understanding-inaddr-any-for-socket-programming
template<typename AsioProtocolType>
class TransportInterface {
protected:

    /// @note Use of a function pointer would not allow lambda captures
    using HandlerFunctionType = std::function<void(asio::error_code const&, Uint64_t)>;

public:

    /// @brief Convenience routine for catching exceptions
    static void CatchException(const std::exception& e) {
        std::string errorMessage = e.what();
        std::cerr << errorMessage << std::endl;
        assert(false);
    }


    /// @brief Convenience routine for catching error codes
    /// @todo Maybe ignore asio::error::connection_aborted?
    static void CatchErrorCode(const std::error_code& e) {
        std::string errorMessage = e.message();
        std::cerr << errorMessage << std::endl;
        if (e) {
            //assert(false);
            Logger::LogWarning(errorMessage);
        }
    }

    TransportInterface(TransportMailboxInterfaceBase* mailbox, const std::shared_ptr<TransportThread>& thread) :
        m_mailbox(mailbox),
        m_socketHolder(thread->context()),
        m_thread(thread)
    {
    }

    virtual ~TransportInterface() = default;

    TransportThread& thread() { return *m_thread; }

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

    const SocketHolder<AsioProtocolType>& socketHolder() const {
        return m_socketHolder;
    }

    SocketHolder<AsioProtocolType>& socketHolder() {
        return m_socketHolder;
    }

protected:

    virtual void updateConnections() {}

    SocketHolder<AsioProtocolType> m_socketHolder; ///< Encapsulates a socket
    TransportMailboxInterfaceBase* m_mailbox; ///< The mailbox for sending and receiving message
    std::shared_ptr<TransportThread> m_thread{ nullptr }; ///< The thread for running this transport;
};

typedef TransportInterface<asio::ip::tcp> TcpTransportInterface;
typedef TransportInterface<asio::ip::udp> UdpTransportInterface;



/// @class ReceiveTransportInterface
/// @brief Class representing an abstraction of a networking server, e.g. A Tcp/IP server
/// @tparam AsioProtocolType the class used to represent a ASIO networking protocol type
/// @detail This class is not specific to any network protocol
template<typename AsioProtocolType>
class ReceiveTransportInterface: public TransportInterface<AsioProtocolType> {
public:

    using TransportInterface::TransportInterface;

    /// @brief Bind the transport to a local endpoint for receiving messages
    /// @param[in] endpoint the endpoint to connect to
    /// @note Unsafe to call in constructor of base classes, since it is virtual
    virtual void bindForListen(const NetworkEndpoint& localEndpoint, bool bind) = 0;

    /// @brief Start receiving messages
    void startReceive()
    {
        try {
            onStartReceive();
            receive();
        }
        catch (std::exception& e)
        {
            CatchException(e);
        }
    }

protected:

    /// @brief Overrideable logic to be called to initialize an asynchronous receive
    virtual void onStartReceive() {}

    /// @brief Overrideable logic to receive a message
    virtual void receive() = 0;

    /// @brief Overrideable logic for the completion handler that is called on asynchronous receive
    virtual void onHandleReceive(Uint64_t bytesTransferred) {
        if (bytesTransferred) {
            // Unpack received message
            m_mailbox->unpackReceive(bytesTransferred);
        }

        startReceive();
    }

    /// @brief Completion handler that is called on asynchronous receive
    void handleReceive(const asio::error_code& error, Uint64_t bytesTransferred)
    {
        if (!error && m_thread->isActive() || error == asio::error::eof)
        {
            if (error == asio::error::eof) {
                ThreadSafeCout{} << "EOF reached for transport";
            }

            // Refresh connections list
            updateConnections();

            // Custom receive handler
            onHandleReceive(bytesTransferred);
        }
        else if (error == asio::error::message_size) {
            assert(false && "Message size error on receive");
        }
        else {
            CatchErrorCode(error);
        }
    }

    /// @brief Perform an asynchronous read
    virtual void asyncRead(HandlerFunctionType functionHandle) = 0;

};

/// @class SendTransportInterface
/// @brief Class representing an abstraction of a networking server or client, e.g. A Tcp/IP server
/// @tparam AsioProtocolType the class used to represent a ASIO networking protocol type
template<typename AsioProtocolType>
class SendTransportInterface : public TransportInterface<AsioProtocolType> {
public:

    using TransportInterface::TransportInterface;

    /// @brief Start sending messages
    void startSend()
    {
        try {
            onStartSend();
            send();
        }
        catch (std::exception& e)
        {
            CatchException(e);
        }
    }

    /// @brief Open the socket for communication
    template<NetworkAddress::AddressType AddressType>
    void openSocket() {
        m_socketHolder.openSocket<AddressType>();
    }

protected:

    /// @brief Overrideable logic to be called to initialize an asynchronous send
    virtual void onStartSend() {}

    /// @brief Overrideable logic to send a message
    virtual void send() = 0;

    /// @brief Overrideable logic for the completion handler that is called on asynchronous send
    virtual void onHandleSend(Uint64_t bytesTransferred) = 0;

    /// @brief Completion handler that is called after sending messages
    void handleSend(const asio::error_code& error, Uint64_t bytesTransferred)
    {
        /// @todo Handle timing here
        if (!error && m_thread->isActive())
        {
            onHandleSend(bytesTransferred);
        }
        else if (error == asio::error::message_size) {
            assert(false && "Message size error on send");
        }
        else {
            CatchErrorCode(error);
        }
    }

    /// @brief Perform an asynchronous write
    virtual void asyncWrite(HandlerFunctionType functionHandle) = 0;
};

} // End rev namespace
 