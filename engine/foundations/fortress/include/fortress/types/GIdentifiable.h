#pragma once

// Internal
#include "fortress/encoding/uuid/GUuid.h"

namespace rev {

/// @class IdentifiableInterface
/// @brief Interface class representing an object with a unique ID
class IdentifiableInterface {
public:
    /// @name Constructors/Destructor
    /// @{
    IdentifiableInterface(const Uuid& uuid) :
        m_uuid(uuid) {
    }
    IdentifiableInterface() :
        m_uuid(true) {
    }
    /// @}

    /// @name Properties
    /// @{

    /// @property UUID
    /// @brief Uuid of this object
    inline const Uuid& getUuid() const { return m_uuid; }

    inline void setUuid(const Uuid& uuid) { m_uuid = uuid; }


    /// @}

protected:

    /// @brief Destructor made protected to prevent polymorphic deletion of non-virtual destructor
    ~IdentifiableInterface() {}

    /// @name Private Members
    /// @{

    /// @brief Unique identifier for this object
    rev::Uuid m_uuid;

    /// @}

};

} // End namespaces
