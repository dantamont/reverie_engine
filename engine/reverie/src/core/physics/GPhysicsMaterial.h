#ifndef GB_PHYSICS_MATERIAL_H
#define GB_PHYSICS_MATERIAL_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// External

// QT

// Internal
#include "GPhysics.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Scene;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class PhysicsMaterial
/// @brief Class representing a physics material
class PhysicsMaterial : public PhysicsBase
{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Statics
    /// @{

    static std::shared_ptr<PhysicsMaterial> Create(const QJsonValue& json);
    static std::shared_ptr<PhysicsMaterial> Create(const GString& name, float staticFriction = 0.5f, float dynamicFriction = 0.5f, float restitution = 0.1f);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Destructor
    /// @{

    ~PhysicsMaterial();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property Static Friction
    float getStaticFriction() const { return m_material->getStaticFriction(); }
    void setStaticFriction(float f) const { m_material->setStaticFriction(f); }

    /// @property Dynamic Friction
    float getDynamicFriction() const { return m_material->getDynamicFriction(); }
    void setDynamicFriction(float f) const { m_material->setDynamicFriction(f); }

    /// @property Restitution
    /// @brief Controls elasticity, 1 = elastic, < 1 is inelastic
    float getRestitution() const { return m_material->getRestitution(); }
    void setRestitution(float r) { m_material->setRestitution(r); }

    physx::PxMaterial* getMaterial() const { return m_material; };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PhysicsMaterial"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::PhysicsMaterial"; }

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors
    /// @{

    explicit PhysicsMaterial(const QJsonValue& json);
    PhysicsMaterial(const GString& name, float staticFriction = 0.5f, float dynamicFriction = 0.5f, float restitution = 0.1f);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Private Methods
    /// @{

    /// @brief Add this material to the physics manager static map
    static void AddToManager(std::shared_ptr<PhysicsMaterial> mtl);

    /// @}
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Private Members
    /// @{
    
    /// The physx material for this wrapper
    physx::PxMaterial* m_material;

    /// @}
};


//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif