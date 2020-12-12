#ifndef GB_PHYSICS_H
#define GB_PHYSICS_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// External
#ifdef NDEBUG
#ifdef _DEBUG
#undef _DEBUG
#endif
#endif

#include <PxPhysicsAPI.h>

// QT

// Internal
#include "../GbObject.h"
#include "../mixins/GbLoadable.h"
#include "../geometry/GbVector.h"
#include "../geometry/GbQuaternion.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class CoreEngine;


//////////////////////////////////////////////////////////////////////////////////
// Macro Definitions
//////////////////////////////////////////////////////////////////////////////////
#define PX_RELEASE(x)	if(x)	{ x->release(); x = nullptr;	}

/////////////////////////////////////////////////////////////////////////////////////////////
// Type Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class PhysicsBase
/// @brief Class representing a base physics object
/// For linking errors, see: https://github.com/NVIDIAGameWorks/PhysX/issues/31
class PhysicsBase : public Object, public Serializable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}


    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    PhysicsBase();
    PhysicsBase(const QString& name);
    virtual ~PhysicsBase();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PhysicsBase"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PhysicsBase"; }

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{


    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @}

};

//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif