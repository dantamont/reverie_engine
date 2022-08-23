#pragma once

#include <exception>
#include <vector>
#include <memory>
#include "fortress/containers/GStrictGrowContainer.h"
#include "fortress/system/memory/GPointerTypes.h"
#include "fortress/types/GSizedTypes.h"

namespace rev {

/// @class MultitonInterface
/// @brief Interface class representing a Multiton
/// @tparam CreateOnInstance If true, uses the constructor that creates a Multiton when Instance is requested
template<typename MultitonType>
class MultitonInterface {
public:

    /// @brief Prevent rearranging of multitons in internal vector
    MultitonInterface(const MultitonInterface&) = delete;
    MultitonInterface& operator= (const MultitonInterface&) = delete;

    /// @brief Creates the Multiton
    /// @details Optionally takes in arguments for construction
    template<typename... Args>
    static MultitonType& Create(Int32_t& outIndex, Args&&... args)
    {
        outIndex =  s_instances.push(std::move(prot_make_unique<MultitonType>(std::forward<Args>(args)...)));
        return *s_instances[outIndex];
    }

    /// @brief Return the instance of the Multiton
    /// @details Optionally takes in arguments for the construction of the Multiton instance
    template<typename... Args>
    static MultitonType& Instance(Int32_t index, Args&&... args)
    {
        if (index >= s_instances.size()) {
            throw std::logic_error("Instance not created before access. Must call Create()");
        }
        return *s_instances[index];
    }

    /// @brief Delete the Multiton with the given index
    static void Delete(Int32_t index) {
        assert(nullptr != s_instances[index] && "Cannot delete nullptr");
        s_instances.invalidate(index);
        s_instances[index].reset();
        s_instances[index] = nullptr;
    }

    /// @brief Delete all of the Multitons
    static void DeleteAll() {
        s_instances.clear();
    }

protected:

    /// @brief Private constructor for the Multiton class
    MultitonInterface() {}

    /// @brief Destructor made protected to prevent polymorphic deletion of non-virtual destructor
    ~MultitonInterface() = default;

private:

    /// @name Private Members
    /// @{

    static StrictGrowContainer<std::vector<std::unique_ptr<MultitonType>>> s_instances; ///< The instances of the Multiton

    /// @}

};

template<typename MultitonType>
StrictGrowContainer < std::vector<std::unique_ptr<MultitonType>>> MultitonInterface<MultitonType>::s_instances{ };


} /// End namespaces
