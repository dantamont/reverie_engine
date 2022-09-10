#pragma once

#include <functional>
#include <map>
//#include <atomic>
#include <shared_mutex>

#include "fortress/containers/extern/tsl/robin_map.h"
#include "fortress/numeric/GSizedTypes.h"

namespace rev {

/// @class Signal
/// @brief A class representing a signal, to implement the observer pattern.
/// @detail A signal object may call multiple slots with the same signature. 
/// You can connect functions to the signal which will be called when the 
/// emit() method on the signal object is invoked. Any argument passed to emit()
/// will be passed to the given functions.
/// @see https://en.wikipedia.org/wiki/Observer_pattern
/// @see https://schneegans.github.io/tutorials/2015/09/20/signal-slot
template <typename... Args>
class Signal {
public:

    Signal() = default;
    ~Signal() = default;

    // Copy constructor and assignment create a new, empty signal.
    Signal(const Signal& /*unused*/) {}

    Signal& operator=(const Signal& other) {
        if (this != &other) {
            disconnect_all();
        }
        return *this;
    }

    // Move constructor and assignment operator work as expected.
    Signal(Signal&& other) noexcept :
    {
        std::unique_lock lock(m_mutex);
        std::unique_lock otherLock(other.m_mutex);

        m_slots = std::move(other.m_slots);
        m_currentId = other.m_currentId;
    }

    Signal& operator=(Signal&& other) noexcept {
        std::unique_lock lock(m_mutex);
        std::unique_lock otherLock(other.m_mutex);

        if (this != &other) {
            m_slots = std::move(other.m_slots);
            m_currentId = other.m_currentId;
        }

        return *this;
    }

    /// @brief Convenience method for casting a function for a signal
    /// @details For overloaded functions, their arguments may be ambiguous. This sorts things out.
    /// @see https://stackoverflow.com/questions/43998060/auto-return-type-in-template
    /// @example Signal<const GTransformMessage&>::Cast(&TransformWidget::update)
    template <typename T>
    static auto Cast(void (T::* func)(Args...)) {
        return static_cast<void(T::*)(Args...)>(func);
    }

    template <typename T>
    static auto Cast(void (T::* func)(Args...) const) {
        return static_cast<void(T::*)(Args...)>(func);
    }


    /// @brief Connects a std::function to the signal. 
    /// @detail The returned value can be used to disconnect the function again.
    /// @example mySignal.connect(myFunction);
    int connect(std::function<void(Args...)> const& slot) const {
        std::unique_lock lock(m_mutex);
        m_slots.insert(std::make_pair(++m_currentId, slot));
        return m_currentId;
    }

    /// @brief Convenience method to connect a member function of an object to this Signal.
    /// @example mySignal.connect(classInstance, &MyClass::memberFunction);
    /// @note the ampersand isn't strictly necessary, since function pointers can be converted to/from
    ///    functions implicitly, but it more clearly shows the intention
    template <typename T>
    int connect(T* inst, void (T::* func)(Args...)) {
        return connect(
            [=](Args... args) {
                (inst->*func)(args...);
            }
        );
    }

    /// @brief Convenience method to connect a const member function of an object to this Signal.
    /// @example mySignal.connect(classInstance, &MyClass::memberFunction);
    template <typename T>
    int connect(T* inst, void (T::* func)(Args...) const) {
        return connect(
            [=](Args... args) {
                (inst->*func)(args...);
            }
        );
    }

    /// @brief Disconnects a previously connected function.
    void disconnect(int id) const {
        std::unique_lock lock(m_mutex);
        m_slots.erase(id);
    }

    /// @brief Disconnects all previously connected functions.
    void disconnectAll() const {
        std::unique_lock lock(m_mutex);
        m_slots.clear();
        m_currentId = 0;
    }

    /// @brief Calls all connected functions.
    /// @todo Pass by reference
    void emitForAll(Args... p) {
        std::shared_lock lock(m_mutex);
        for (const auto& it : m_slots) {
            it.second(p...);
        }
    }

    /// @brief Calls all connected functions except for one.
    void emitForAllExceptOne(int excludedConnectionID, Args... p) {
        std::shared_lock lock(m_mutex);
        for (const auto& it : m_slots) {
            if (it.first != excludedConnectionID) {
                it.second(p...);
            }
        }
    }

    /// @brief Calls only one connected function.
    void emitFor(int connectionID, Args... p) {
        std::shared_lock lock(m_mutex);
        auto const& it = m_slots.find(connectionID);
        if (it != m_slots.end()) {
            it->second(p...);
        }
    }

private:
    mutable std::shared_mutex m_mutex; ///< Mutex for controlling access to slots map
    mutable tsl::robin_map<Uint32_t, std::function<void(Args...)>> m_slots; ///< The map of slots connected to this signal.
    mutable Uint32_t m_currentId{ 0 }; ///< The most recently set ID used to track signal/slot connections
};


} // End rev namespace
