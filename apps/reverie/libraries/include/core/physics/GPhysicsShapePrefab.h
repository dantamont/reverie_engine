#pragma once

// Internal
#include "GPhysics.h"
#include "logging/GLogger.h"

namespace rev {

class Scene;
class PhysicsMaterial;
class PhysicsGeometry;
class RigidBody;
class PhysicsShape;

/// @class PhysicsShapePrefab
/// @brief Class representing physics geometry
class PhysicsShapePrefab : public PhysicsBase {
public:
    /// @name Statics
    /// @{
    //static PhysicsShapePrefab* Get(const GString& name, std::unique_ptr<PhysicsGeometry> geometry, const std::shared_ptr<PhysicsMaterial>& material);
    static PhysicsShapePrefab* Create(const nlohmann::json& json);
    static PhysicsShapePrefab* Create(const GString& name, std::unique_ptr<PhysicsGeometry> geometry, const std::shared_ptr<PhysicsMaterial>& material);

    /// @}

    /// @name Destructor
    /// @{
    ~PhysicsShapePrefab();
    /// @}

    /// @name Properties
    /// @{

    /// @brief Get the geometry of the shape
    PhysicsGeometry* geometry() const { return m_geometry.get(); }
    void setGeometry(std::unique_ptr<PhysicsGeometry> geometry);

    /// @brief Get the material for the shape
    inline PhysicsMaterial* material() {
        if (m_materials.size() == 1) {
            return m_materials[0].get();
        }
        else {
            Logger::Throw("Error, material is ambiguous");
        }
        return nullptr;
    };
    inline PhysicsMaterial* getMaterial() const {
        if (m_materials.size() == 1) {
            return m_materials[0].get();
        }
        else {
            Logger::Throw("Error, material is ambiguous");
        }
        return nullptr;
    };

    /// @}

    /// @name Public methods
    /// @{

    /// @brief Update all instances with new prefab parameters
    void updateInstances();

    /// @brief Make an exclusive copy of this shape for the given rigid body
    physx::PxShape* createExclusive(RigidBody& rigidBody) const;

    /// @brief Set the material for the shape
    void addMaterial(const std::shared_ptr<PhysicsMaterial>& mtl);

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const PhysicsShapePrefab& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, PhysicsShapePrefab& orObject);


    /// @}


protected:
    friend class PhysicsShape;
    friend class RigidBody;
    friend class PhysicsManager;

    /// @name Constructors/Destructor
    /// @{

    PhysicsShapePrefab();
    PhysicsShapePrefab(const GString& name);

    /// @name Private Methods
    /// @{

    /// @brief Add this material to the physics manager static map
    static PhysicsShapePrefab* AddToManager(std::unique_ptr<PhysicsShapePrefab> shape);

    void addInstance(PhysicsShape* instance);
    void removeInstance(PhysicsShape* instance);

    /// @}

    /// @name Private Members
    /// @{

    /// @brief The geometry for this shape
    std::unique_ptr<PhysicsGeometry> m_geometry;

    /// @brief The materials for this shape
    std::vector<std::shared_ptr<PhysicsMaterial>> m_materials;

    std::vector<PhysicsShape*> m_instances;

    /// @}
};



} // End namespaces
