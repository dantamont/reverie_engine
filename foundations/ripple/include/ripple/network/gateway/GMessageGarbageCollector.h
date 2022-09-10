#pragma once

#include "fortress/numeric/GSizedTypes.h"

namespace rev {

class GMessage;

/// @brief Class for disposing of message pointers after a set amount of time
class MessageGarbageCollector {
public:

    MessageGarbageCollector() = default;
    ~MessageGarbageCollector() = default;

    /// @brief Create a message to be defer-deleted
    template<typename T>
    T* createCollectedMessage() {
        T* message = new T();
        deferredDelete(message);
        return message;
    }

    /// @brief Create a message, copy-constructed from another message, to be defer-deleted
    template<typename T>
    T* createCollectedMessage(const T& messageIn) {
        T* message = new T(messageIn);
        deferredDelete(message);
        return message;
    }

    /// @brief Update to delete any messages
    void deleteStaleMessages();

    /// @brief Defer delete a message
    void deferredDelete(GMessage* message);

    Uint32_t m_garbageCollectTimeUs{ 10000000 }; ///< How long to wait for garbage collection (10sec default)

};

}