#ifndef GB_PHYSICS_SHAPE_PREFAB_H
#define GB_PHYSICS_SHAPE_PREFAB_H

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
class PhysicsMaterial;
class PhysicsGeometry;
class RigidBody;
class PhysicsShape;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class PhysicsShapePrefab
/// @brief Class representing physics geometry
class PhysicsShapePrefab : public PhysicsBase {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Statics
    /// @{
    //static PhysicsShapePrefab* Get(const GString& name, std::unique_ptr<PhysicsGeometry> geometry, const std::shared_ptr<PhysicsMaterial>& material);
    static PhysicsShapePrefab* Create(const QJsonValue& json);
    static PhysicsShapePrefab* Create(const GString& name, std::unique_ptr<PhysicsGeometry> geometry, const std::shared_ptr<PhysicsMaterial>& material);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Destructor
    /// @{
    ~PhysicsShapePrefab();
    /// @}

    //--------------------------------------------------------------------------------------------
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
            throw("Error, material is ambiguous");
        }
        return nullptr;
    };
    inline PhysicsMaterial* getMaterial() const {
        if (m_materials.size() == 1) {
            return m_materials[0].get();
        }
        else {
            throw("Error, material is ambiguous");
        }
        return nullptr;
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Update all instances with new prefab parameters
    void updateInstances();

    /// @brief Make an exclusive copy of this shape for the given rigid body
    physx::PxShape* createExclusive(RigidBody& rigidBody) const;

    /// @brief Set the material for the shape
    void addMaterial(const std::shared_ptr<PhysicsMaterial>& mtl);

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
    const char* className() const override { return "PhysicsShapePrefab"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::PhysicsShapePrefab"; }

    /// @}

protected:
    friend class PhysicsShape;
    friend class RigidBody;
    friend class PhysicsManager;

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    PhysicsShapePrefab();
    PhysicsShapePrefab(const GString& name);

    //PhysicsShapePrefab(const QJsonValue& json);
    //PhysicsShapePrefab(const GString& name,
    //    const std::shared_ptr<PhysicsGeometry>& geometry,
    //    const std::shared_ptr<PhysicsMaterial>& material);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Private Methods
    /// @{

    /// @brief Add this material to the physics manager static map
    static PhysicsShapePrefab* AddToManager(std::unique_ptr<PhysicsShapePrefab> shape);

    void addInstance(PhysicsShape* instance);
    void removeInstance(PhysicsShape* instance);

    /// @}
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Private Members
    /// @{

    /// @brief The geometry for this shape
    std::unique_ptr<PhysicsGeometry> m_geometry;

    /// @brief The materials for this shape
    std::vector<std::shared_ptr<PhysicsMaterial>> m_materials;

    std::vector<PhysicsShape*> m_instances;

    /// @}
};


//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif