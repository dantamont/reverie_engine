#pragma once

#include <exception>
#include "fortress/system/memory/GPointerTypes.h"

namespace rev {

/// @class SingletonInterface
/// @brief Interface class representing a singleton
/// @tparam CreateOnInstance If true, uses the constructor that creates a singleton when Instance is requested
template<typename SingletonType, bool CreateOnInstance = true>
class SingletonInterface {
public:

    /// @brief Creates the singleton
    /// @details Optionally takes in arguments for construction
    template<typename... Args>
    static SingletonType& Create(Args&&... args)
    {
        assert(nullptr == s_instance && "Singleton already created");
        s_instance = prot_make_unique<SingletonType>(std::forward<Args>(args)...);
        return *s_instance;
    }

    /// @brief Return the instance of the singleton
    /// @details Optionally takes in arguments for the construction of the singleton instance
    template<typename... Args>
    static SingletonType& Instance(Args&&... args)
    {
        if constexpr (CreateOnInstance) {
            if (nullptr == s_instance) {
                s_instance = prot_make_unique<SingletonType>(std::forward<Args>(args)...);
            }
        }
        else {
            if (nullptr == s_instance) {
                throw std::logic_error("Instance not created before access. Must call Create()");
            }
        }
        return *s_instance;
    }

    /// @brief Delete the singleton
    static void Delete() {
        if (s_instance) {
            s_instance.reset();
        }
        s_instance = nullptr;
    }

protected:

    /// @brief Private constructor for the singleton class
    SingletonInterface() {}

    /// @brief Destructor made protected to prevent polymorphic deletion of non-virtual destructor
    ~SingletonInterface() = default;

private:

    /// @name Private Members
    /// @{

    static std::unique_ptr<SingletonType> s_instance; ///< The instance of the singleton

    /// @}

};

template<typename SingletonType, bool CreateOnInstance>
std::unique_ptr<SingletonType> SingletonInterface<SingletonType, CreateOnInstance>::s_instance{ nullptr };


} /// End namespaces
