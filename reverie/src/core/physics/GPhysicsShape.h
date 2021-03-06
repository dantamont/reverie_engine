#ifndef GB_PHYSICS_SHAPE_H
#define GB_PHYSICS_SHAPE_H

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
class PhysicsShapePrefab;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PhysicsShape
/// @brief An instantiation of a physics shape
/// @details Simulation Shapes are enabled for contact generation, and scene query shapes are enabled for raycasts
/// by default, shapes are enabled for both.  Contact generation is not necessarily always a part of physics
class PhysicsShape: public Identifiable {
public:

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructors
    /// @{

    PhysicsShape();
    PhysicsShape(PhysicsShapePrefab& prefab, RigidBody* body);
    ~PhysicsShape();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Properties
    /// @{

    PhysicsShapePrefab& prefab() const { return *m_prefab; }
    physx::PxShape* pxShape() { return m_pxShape; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Public Methods
    /// @{

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

    void setPrefab(PhysicsShapePrefab& prefab, bool removeFromOldPrefab = true);

    /// @}

private:

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Friends
    /// @{

    friend class Rigidbody;
    friend class PhysicsShapePrefab;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Private Methods
    /// @{

    /// @brief Called from prefab destructor to ensure clean deletion
    void prepareForDelete();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Private Members
    /// @{

    physx::PxShape* m_pxShape = nullptr;
    RigidBody* m_body = nullptr;

    PhysicsShapePrefab* m_prefab;

    /// @}
};



//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif