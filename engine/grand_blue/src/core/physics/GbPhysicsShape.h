#ifndef GB_PHYSICS_SHAPE_H
#define GB_PHYSICS_SHAPE_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// External

// QT

// Internal
#include "GbPhysics.h"

namespace Gb {

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
    static std::shared_ptr<PhysicsShapePrefab> get(const QJsonValue& json);
    static std::shared_ptr<PhysicsShapePrefab> get(const QString& name);
    static std::shared_ptr<PhysicsShapePrefab> get(const QString& name,
        const std::shared_ptr<PhysicsGeometry>& geometry,
        const std::shared_ptr<PhysicsMaterial>& material);
    static std::shared_ptr<PhysicsShapePrefab> create(const QJsonValue& json);
    static std::shared_ptr<PhysicsShapePrefab> create(const QString& name,
        const std::shared_ptr<PhysicsGeometry>& geometry,
        const std::shared_ptr<PhysicsMaterial>& material);

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
    std::shared_ptr<PhysicsGeometry> geometry() { return m_geometry; }
    const std::shared_ptr<PhysicsGeometry>& geometry() const { return m_geometry; }
    void setGeometry(const std::shared_ptr<PhysicsGeometry>& geometry);

    /// @brief Get the material for the shape
    std::shared_ptr<PhysicsMaterial> material() {
        if (m_materials.size() == 1) {
            return m_materials.begin()->second;
        }
        return nullptr;
    };
    std::shared_ptr<PhysicsMaterial> getMaterial() const {
        if (m_materials.size() == 1) {
            return m_materials.begin()->second;
        }
        return nullptr;
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Make an exclusive copy of this shape for the given rigid body
    physx::PxShape* createExclusive(RigidBody& rigidBody) const;

    /// @brief Set the material for the shape
    void addMaterial(const std::shared_ptr<PhysicsMaterial>& mtl);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PhysicsShapePrefab"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PhysicsShapePrefab"; }

    /// @}

protected:
    friend class PhysicsShape;
    friend class PhysicsManager;

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    PhysicsShapePrefab(const QJsonValue& json);
    PhysicsShapePrefab(const QString& name,
        const std::shared_ptr<PhysicsGeometry>& geometry,
        const std::shared_ptr<PhysicsMaterial>& material);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Private Methods
    /// @{

    /// @brief Add this material to the physics manager static map
    static void addToManager(std::shared_ptr<PhysicsShapePrefab> shape);

    void addInstance(PhysicsShape* instance);
    void removeInstance(PhysicsShape* instance);

    /// @}
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Private Members
    /// @{

    /// @brief The geometry for this shape
    std::shared_ptr<PhysicsGeometry> m_geometry;

    /// @brief The materials for this shape
    std::unordered_map<QString, std::shared_ptr<PhysicsMaterial>> m_materials;

    std::unordered_map<Uuid, PhysicsShape*> m_instances;

    /// @}
};


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class PhysicsShape
/// @brief An instantiation of a physics shame
/// @details Simulation Shapes are enabled for contact generation, and scene query shapes are enabled for raycasts
/// by default, shapes are enabled for both.  Contact generation is not necessarily always a part of physics
class PhysicsShape: public Object {
public:

    PhysicsShape() : // Default constructor for container storage
        m_pxShape(nullptr),
        m_prefab(nullptr){
    }
    PhysicsShape(PhysicsShapePrefab& prefab, RigidBody* body);
    ~PhysicsShape();

    /// @brief Detach from the associated body
    void detach() const;

    /// @brief Toggle the physics shape on or off for contact tests
    void toggleContact(bool toggle) {
        m_pxShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, toggle);
    }

    /// @brief Toggle the physics shape on or off for scene queries, e.g., raycasts
    void toggleSceneQueries(bool toggle) {
        m_pxShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, toggle);
    }

    /// @brief Called when prefab is modified
    void reinitialize();

    bool isEmpty() const { return !m_pxShape && !m_prefab; }
    PhysicsShapePrefab& prefab() const { return *m_prefab; }
    
    void setPrefab(PhysicsShapePrefab& prefab, bool removeFromOldPrefab = true);

    physx::PxShape* pxShape() { return m_pxShape; }
private:
    friend class Rigidbody;
    friend class PhysicsShapePrefab;

    /// @brief Called from prefab destructor to ensure clean deletion
    void prepareForDelete();

    physx::PxShape* m_pxShape = nullptr;
    RigidBody* m_body = nullptr;

    PhysicsShapePrefab* m_prefab;
};



//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif