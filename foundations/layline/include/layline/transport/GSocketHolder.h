#pragma once

#include "layline/network/GNetworkEndpoint.h"
#include "layline/transport/GTransportConnection.h"

namespace rev {

/// @brief Encapsulates an ASIO socket
/// @tparam AsioProtocolType the class used to represent a ASIO networking protocol type
template<typename AsioProtocolType>
class SocketHolder {
private:

    static constexpr Uint32_t s_defaultSocketReceiveBufferSizeBytes = 65535; ///< Default socket receive buffer size

public:

    SocketHolder(asio::io_context& ioContext):
        m_socket(ioContext)
    {
    }

    ~SocketHolder() = default;

    NetworkEndpoint& localEndpoint() { return m_endpoint; }
    const NetworkEndpoint& localEndpoint() const { return m_endpoint; }

    typename AsioProtocolType::socket& socket() {
        return m_socket;
    }

    /// @brief Generally unsafe to access without mutex lock
    const std::vector<std::shared_ptr<TransportConnection>>& connections() const {
        return m_connections;
    }

    /// @brief Obtain the number of bytes available for reading
    Uint64_t availableBytesForRead() const {
        asio::error_code ec;
        Uint64_t avail = m_socket.available(ec);
        if (ec) {
            std::cerr << ec.message();
            assert(false && "Socket failed to get available bytes for read");
        }
        return avail;
    }

    /// @brief Connect the socket to the given remote endpoint
    /// @param[in] timeoutMicroSeconds how long to wait until giving up connection attempt. Negative values mean no reattempts will be made
    /// @note Will open the socket if not yet opened
    /// @see https://stackoverflow.com/questions/27014955/socket-connect-vs-bind
    void connect(const NetworkEndpoint& remoteEndpoint, Int64_t timeoutMicroSeconds = -1) {
        std::shared_ptr<TransportConnection> connection = hasConnection(remoteEndpoint);
        if (!connection) {
            addConnection(remoteEndpoint);
        }

        static asio::error_code s_connectionRefusedError = asio::error::connection_refused;

        bool wasOpen = m_socket.is_open();
        asio::error_code ec;
        m_socket.connect(remoteEndpoint.toAsio<AsioProtocolType>(), ec);
        if (ec == s_connectionRefusedError) {
            if (0 > timeoutMicroSeconds) {
                // Connection failed, and no timeout specified
                std::cerr << ec.message();
                assert(false && "Sadness, socket connection refused. Likely nothing listening.");
            }
            else {
                ExpireTimer timer{ static_cast<Uint64_t>(timeoutMicroSeconds) };
                timer.restart();

                // Reattempt connection if failed, until timeout
                static constexpr Int32_t s_noTimeout = -1;
                while (ec == s_connectionRefusedError && !timer.isExpired()) {
                    m_socket.connect(remoteEndpoint.toAsio<AsioProtocolType>(), ec);
                }

                if (ec) {
                    std::cerr << ec.message();
                    assert(false && "Socket failed to connect after timeout");
                }
            }
        }
        else if (ec) {
            std::cerr << ec.message();
            assert(false && "Socket failed to connect");
        }

        // Set socket options in case not opened explicitly
        if (!wasOpen) {
            setSocketOptions(s_defaultSocketReceiveBufferSizeBytes);
        }
    }

    /// @brief Bind the socket to a local endpoint for receiving messages
    /// @param[in] endpoint the endpoint to connect to
    /// @see https://stackoverflow.com/questions/27014955/socket-connect-vs-bind
    void bindForListen(const NetworkEndpoint& endpoint, bool bind) {
        // Set endpoint, and open the socket with the same protocol
        NetworkAddress::AddressType addressType = endpoint.address().getAddressType();
        if (addressType == NetworkAddress::AddressType::kIpv4) {
            openSocket<NetworkAddress::AddressType::kIpv4>();
        }
        else {
            openSocket<NetworkAddress::AddressType::kIpv6>();
        }

        // Optionally bind the socket to the endpoint
        if (bind) {
            bindSocket(endpoint);
        }
    }

    /// @brief Return whether or not the remote endpoint is already a connection
    std::shared_ptr<TransportConnection> hasConnection(const NetworkEndpoint& remoteEndpoint) const {
        std::shared_lock lock(m_connectionsMutex);
        std::vector<std::shared_ptr<TransportConnection>>::const_iterator iter = std::find_if(
            m_connections.begin(),
            m_connections.end(),
            [&](const std::shared_ptr<TransportConnection>& connection) {
                return connection->remoteEndpoint() == remoteEndpoint;
            }
        );
        return iter != m_connections.end() ? *iter : nullptr;
    }

    /// @brief Add a connection to the socket if it does not have the remote endpoint already
    bool addConnection(const NetworkEndpoint& remoteEndpoint) {
        bool addedConnection = hasConnection(remoteEndpoint) == nullptr;
        if (addedConnection) {
            std::unique_lock lock(m_connectionsMutex);
            m_connections.push_back(
                std::make_shared<TransportConnection>(remoteEndpoint)
            );
        }
        return addedConnection;
    }

    /// @brief Add a connection to the socket
    void addConnection(const std::shared_ptr<TransportConnection>& connection) {
        // Set remote endpoint
        std::unique_lock lock(m_connectionsMutex);
        m_connections.push_back(connection);
    }

    /// @brief Return the number of remote connections
    Uint32_t connectionCount() const {
        std::shared_lock lock(m_connectionsMutex);
        return m_connections.size();
    }

    /// @brief Return strings describing the connections
    std::vector<std::string> connectionInfo() const {
        std::shared_lock lock(m_connectionsMutex);
        std::vector<std::string> outStrings;
        for (const std::shared_ptr<TransportConnection>& connection : m_connections) {
            outStrings.emplace_back(std::string(*connection));
        }
        return outStrings;
    }

    /// @brief Open socket
    /// @detail Opening a socket makes it wait for a connection.
    template<NetworkAddress::AddressType AddressType>
    void openSocket() {
        // Open the socket
        std::error_code error;
        if constexpr (AddressType == NetworkAddress::AddressType::kIpv4) {
            m_socket.open(AsioProtocolType::v4(), error);
        }
        else {
            m_socket.open(AsioProtocolType::v6(), error);
        }
        assert(!error && "Error opening socket");

        /// @brief Set any socket options
        setSocketOptions(s_defaultSocketReceiveBufferSizeBytes);
    }

    /// @brief Close the socket
    /// @detail shutdown() is preferable to close() if socket handle is being stored somewhere else
    /// @see https://stackoverflow.com/questions/4160347/close-vs-shutdown-socket
    /// @see https://stackoverflow.com/questions/69230844/io-context-stop-vs-socket-close
    void closeSocket() {
        if (m_socket.is_open()) {
            std::error_code ec;
            m_socket.close(ec);
            if (ec) {
                ThreadSafeCout{} << "ERROR closing socket: " << ec.message();
            }
        }
    }

    /// @brief Close and shutdown both sides of the socket
    /// @detail shutdown() is preferable to close() if socket handle is being stored somewhere else
    ///   This hopefully makes sure that the socket gets flushed:
    /// "For portable behaviour with respect to graceful closure of a connected socket, call shutdown() before closing the socket."
    /// @see https://www.boost.org/doc/libs/1_43_0/doc/html/boost_asio/reference/basic_stream_socket/close/overload2.html
    /// @see https://stackoverflow.com/questions/4160347/close-vs-shutdown-socket
    /// @see https://stackoverflow.com/questions/69230844/io-context-stop-vs-socket-close
    void closeAndShutdownSocket() {
        if (m_socket.is_open()) {
            std::error_code ec;
            m_socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            m_socket.close(ec);
            if (ec)
            {
                ThreadSafeCout{} << "ERROR shutting down socket: " << ec.message();
            }
        }
    }

    /// @brief Flush a given number of bytes from the socket
    void flushSocket(Uint32_t numBytes) {
        asio::streambuf sb;
        asio::error_code ec;
        if constexpr (std::is_same_v<AsioProtocolType, asio::ip::tcp>) {
            asio::read(m_socket, sb, asio::transfer_exactly(numBytes), ec);
            //ThreadSafeCout{} << "Received: '" << &sb << "'\n";

            if (ec) {
                ThreadSafeCout{} << "ERROR flushing socket: " << ec.message() << "\n";
            }
        }
        else {
            assert(false && "Flush socket is unimplemented for UDP");
        }
    }

    /// @todo Untested
    /// @brief Flush the socket
    /// @see https://stackoverflow.com/questions/3345309/how-to-flush-the-socket-using-boost
    void flushSocket()
    {
        static constexpr Uint32_t s_bufferSize = 1024;
        asio::streambuf b;
        asio::streambuf::mutable_buffers_type bufs = b.prepare(s_bufferSize);
        std::size_t bytes = m_socket.receive(bufs); // !!! This will block until some data becomes available
        b.commit(bytes);
        asio::socket_base::bytes_readable command(true);
        m_socket.io_control(command);

        while (command.get())
        {
            bufs = b.prepare(s_bufferSize);
            bytes = m_socket.receive(bufs);
            b.commit(bytes);
            m_socket.io_control(command); // reset for bytes pending
        }
    }

    /// @brief Bind socket to an endpoint
    /// @detail Binding a socket to an endpoint assigns it an address and port
    void bindSocket(const NetworkEndpoint& endpoint) {
        m_endpoint = endpoint;
        std::error_code error;
        m_socket.bind(endpoint.toAsio<AsioProtocolType>(), error);
        assert(!error && "Error binding socket");
    }

    /// @brief Launch an asynchronous read
    /// @detail Guarantees that all requested bytes will be received
    /// @note Unlike asyncReceive, this will guarantee the requested number of bytes
    template<typename ...Args>
    inline void asyncRead(Args&... args)
    {
        // Is implemented over async_read_some
        asio::async_read(m_socket, args...);
    }

    /// @brief Launch an asynchronous write
    /// @detail Guarantees that all requested bytes will be sent
    template<typename ...Args>
    inline void asyncWrite(Args&... args)
    {
        // Is implemented over async_write_some
        asio::async_write(m_socket, args...);
    }

    /// @brief Launch an asynchronous receive
    /// @detail Doesn't guarantee all bytes will be received, but can be used by any protocol
    /// @note Should be avoided, since the lack of guarantees results in unpredictable failures
    template<typename ...Args>
    inline void asyncReceive(Args&... args)
    {
        if constexpr (std::is_same_v<AsioProtocolType, asio::ip::udp>) {
            /// UDP, datagram-based receive
            m_socket.async_receive_from(args...);
        }
        else {
            /// TCP
            m_socket.async_receive(args...);
        }
    }

    /// @brief Launch an asynchronous send
    /// @detail Doesn't guarantee all bytes will be sent, but can be used by any protocol
    template<typename ...Args>
    inline void asyncSend(Args&... args)
    {
        m_socket.async_send_to(args...);
    }


    /// @brief Get the size of the socket receive buffer
    Uint32_t getSocketReceiveBufferSize() const {
        asio::socket_base::receive_buffer_size option;
        m_socket.get_option(option);
        return option.value();
    }


    /// @brief Set any socket options
    /// @details Must set this each time socket is opened
    /// @see https://stackoverflow.com/questions/35154547/modifying-boostasiosocketset-option
    /// @see https://stackoverflow.com/questions/44930288/receiving-udp-packets-at-high-frequency-packet-loss
    void setSocketOptions(Uint32_t socketReceiveBufferSizeBytes) {
        asio::socket_base::receive_buffer_size optionBigger(socketReceiveBufferSizeBytes);
        m_socket.set_option(optionBigger);

        if constexpr (std::is_same_v<AsioProtocolType, asio::ip::tcp>) {
            asio::socket_base::linger lingerOption(false, 0);
            m_socket.set_option(lingerOption);
        }

        //asio::socket_base::reuse_address option;
        //m_socket.get_option(option);
        //bool is_set = option.value();
        //if (is_set) {
        //    std::cout << "woo no";
        //}
    }

private:

    NetworkEndpoint m_endpoint{}; ///< The local endpoint that this socket is using to receive messages
    typename AsioProtocolType::socket m_socket{}; ///< The ASIO socket used to transmit data

    mutable std::shared_mutex m_connectionsMutex; ///< Mutex for accessing the list of remote endpoints
    std::vector<std::shared_ptr<TransportConnection>> m_connections; ///< The remote endpoints connected to this socket
};

} // End rev namespace
 