#pragma once

#include <mutex>
#include <vector>

#include "fortress/time/GExpireTimer.h"
#include "fortress/layer/framework/GSingleton.h"

namespace rev {

/// @class GarbageCollector
/// @brief A class for managing the lifetime of raw pointers
template<typename CollectedType>
class GarbageCollector: public SingletonInterface<GarbageCollector<CollectedType>> {
public:
    /// @brief Queue an object for a deferred deletion
    void deferredDelete(CollectedType* pointer, Uint64_t timeout) {
        std::unique_lock lock(m_accessMutex);
        m_pointers.emplace_back(TimedPointer{ ExpireTimer(timeout), pointer });
    }

    /// @brief Check for any pointers that need deletion
    void update() {
        std::unique_lock lock(m_accessMutex);
        std::vector<TimedPointer> pointers;
        m_pointers.swap(pointers);
        for (const TimedPointer& pointer : pointers) {
            if (pointer.m_timer.isExpired()) {
                // Delete the pointer
                delete pointer.m_pointer;
            }
            else {
                // Keep in list
                m_pointers.push_back(pointer);
            }
        }
    }

    const Uint32_t pointerCount() const {
        std::unique_lock lock(m_accessMutex);
        return m_pointers.size();
    }

protected:

    GarbageCollector() = default;

    /// @brief Struct controlling expire time for a pointer
    /// @tparam CollectedType 
    struct TimedPointer {
        ExpireTimer m_timer;
        CollectedType* m_pointer{ nullptr };
    };

    std::vector<TimedPointer> m_pointers; ///< The managed pointers
    mutable std::mutex m_accessMutex; ///< Manage access to the pointers to delete
};

} // End rev namespace
