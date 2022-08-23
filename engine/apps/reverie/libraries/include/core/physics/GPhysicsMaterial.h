#pragma once

// Internal
#include "GPhysics.h"

namespace rev {

class Scene;

/// @class PhysicsMaterial
/// @brief Class representing a physics material
class PhysicsMaterial : public PhysicsBase
{
public:
    /// @name Statics
    /// @{

    static std::shared_ptr<PhysicsMaterial> Create(const nlohmann::json& json);
    static std::shared_ptr<PhysicsMaterial> Create(const GString& name, float staticFriction = 0.5f, float dynamicFriction = 0.5f, float restitution = 0.1f);

    /// @}

    /// @name Destructor
    /// @{

    ~PhysicsMaterial();

    /// @}

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

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const PhysicsMaterial& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, PhysicsMaterial& orObject);


    /// @}


protected:

    /// @name Constructors
    /// @{

    explicit PhysicsMaterial(const nlohmann::json& json);
    PhysicsMaterial(const GString& name, float staticFriction = 0.5f, float dynamicFriction = 0.5f, float restitution = 0.1f);

    /// @}

    /// @name Private Methods
    /// @{

    /// @brief Add this material to the physics manager static map
    static void AddToManager(std::shared_ptr<PhysicsMaterial> mtl);

    /// @}
    /// @name Private Members
    /// @{
    
    /// The physx material for this wrapper
    physx::PxMaterial* m_material;

    /// @}
};

} // End namespaces
