#pragma once

namespace rev {

/// @class NetworkProtocol
/// @brief Class representing a networking protocol
class NetworkProtocol {
    /// @brief The type of the protocol
    enum class Type {
        kInvalid = -1,
        kTcpIp,
        kUdp
    };
};

} // End rev namespace
