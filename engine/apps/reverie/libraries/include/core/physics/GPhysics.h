#pragma once

// External
#ifdef NDEBUG
#ifdef _DEBUG
#undef _DEBUG
#endif
#endif

#pragma warning(push, 0)        
#include <PxPhysicsAPI.h>
#pragma warning(pop)   

// QT

// Internal
#include "fortress/types/GNameable.h"
#include "fortress/types/GIdentifiable.h"
#include "fortress/types/GLoadable.h"
#include "fortress/containers/math/GVector.h"
#include "fortress/containers/math/GQuaternion.h"

namespace rev {

class CoreEngine;

#define PX_RELEASE(x)	if(x)	{ x->release(); x = nullptr;	}

/// @class PhysicsBase
/// @brief Class representing a base physics object
/// For linking errors, see: https://github.com/NVIDIAGameWorks/PhysX/issues/31
class PhysicsBase : public IdentifiableInterface, public NameableInterface {
public:

    /// @name Constructors/Destructor
    /// @{
    PhysicsBase();
    PhysicsBase(const GString& name);
    virtual ~PhysicsBase();
    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const PhysicsBase& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, PhysicsBase& orObject);


    /// @}

};

} // End namespaces
