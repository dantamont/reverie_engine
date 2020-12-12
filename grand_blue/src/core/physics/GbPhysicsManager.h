#ifndef GB_PHYSICS_MANAGER_H
#define GB_PHYSICS_MANAGER_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// External

// QT

// Internal
#include "../GbManager.h"
#include "../mixins/GbLoadable.h"
#include "GbPhysics.h"
#include "../containers/GbContainerExtensions.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class ProcessManager;
class PhysicsScene;
class PhysicsMaterial;
class PhysicsGeometry;
class PhysicsShapePrefab;

/////////////////////////////////////////////////////////////////////////////////////////////
// Type Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

enum class QueryFlag {
    kStatic = physx::PxQueryFlag::eSTATIC, // 	Traverse static shapes. 
    kDynamic = physx::PxQueryFlag::eDYNAMIC, //	Traverse dynamic shapes
    kPreFilter = physx::PxQueryFlag::ePREFILTER, // Run the pre-intersection-test filter (see PxQueryFilterCallback::preFilter()). 
    kPostFilter = physx::PxQueryFlag::ePOSTFILTER, // Run the post-intersection-test filter (see PxQueryFilterCallback::postFilter()). 
    kAnyHit = physx::PxQueryFlag::eANY_HIT, // Abort traversal as soon as any hit is found and return it via callback.block. Helps query performance. Both eTOUCH and eBLOCK hitTypes are considered hits with this flag. 
    kNoBlock = physx::PxQueryFlag::eNO_BLOCK, // All hits are reported as touching. Overrides eBLOCK returned from user filters with eTOUCH. This is also an optimization hint that may improve query performance. 
    kReserved = physx::PxQueryFlag::eRESERVED // Reserved for physx internal use.
};
typedef QFlags<QueryFlag> QueryFlags;
Q_DECLARE_OPERATORS_FOR_FLAGS(QueryFlags)


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class Error callback
class ErrorCallback : public physx::PxErrorCallback, public Object{
public:
    ErrorCallback();
    ~ErrorCallback();

    virtual void reportError(physx::PxErrorCode::Enum code,
        const char* message, 
        const char* file,
        int line);
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PhysicsManager
/// @brief Class representing the physics manager
/// See: https://gameworksdocs.nvidia.com/PhysX/4.0/documentation/PhysXGuide/Manual/HelloWorld.html
class PhysicsManager : public Manager, public Serializable {
    Q_OBJECT
public:

    /// @brief  Foundation SDK singleton class. 
    /// @details  Need to have an instance of this class to instance the higher level SDKs
    static physx::PxFoundation* foundation() { return m_foundation; }

    /// @brief Abstract singleton factory class used for instancing objects in the Physics SDK.
    /// @details Can use PxPhysics to set global parameters which will effect all scenes, create triangle meshes.
    /// Can get an instance of this class by calling PxCreatePhysics()
    static physx::PxPhysics* physics() { return m_physics; }

    /// @brief he CpuDispatcher is an abstract class the SDK uses for interfacing with the application's thread pool. 
    /// @details Typically, there will be one single CpuDispatcher for the entire application, 
    /// since there is rarely a need for more than one thread pool. 
    /// A CpuDispatcher instance may be shared by more than one TaskManager, 
    /// for example if multiple scenes are being used.
    static physx::PxDefaultCpuDispatcher* dispatcher() { return m_dispatcher; }

    /// @brief Pointers to all the physics scenes
    static std::vector<std::shared_ptr<PhysicsScene>>& scenes() { return m_scenes; }

    /// @brief Pointers to all the physics shapes
    /// @details Shapes are indexed by unique name
    static const std::shared_ptr<PhysicsShapePrefab>& DefaultShape() {
        return s_shapes[DefaultShapeKey()];
    }
    static void RemoveShape(const std::shared_ptr<PhysicsShapePrefab>& prefab);
    static void RenameShape(const std::shared_ptr<PhysicsShapePrefab>& prefab, const GString& name);
    static const tsl::robin_map<GString, std::shared_ptr<PhysicsShapePrefab>>& ShapePrefabs() { return s_shapes; }

    /// @brief Pointers to all baked physics geometry
    static tsl::robin_map<GString, std::shared_ptr<PhysicsGeometry>>& geometry() { return s_geometry; }

    /// @brief Pointers to all physics materials
    static tsl::robin_map<GString, std::shared_ptr<PhysicsMaterial>>& Materials() { return s_materials; }

    /// @brief Clear all geometry and materials
    static void clear();

    static Vector3d toVector3d(const physx::PxVec3& vec3);
    static Vector3 toVector3(const physx::PxVec3& vec3);
    static physx::PxVec3 toPhysX(const Vector3d& vec3);
    static physx::PxVec3 toPhysX(const Vector3f& vec3);
    static Quaternion toQuaternion(const physx::PxQuat& quat);

    static const GString& DefaultShapeKey() { return s_defaultShapeKey; }
    static const GString& DefaultMaterialKey() { return s_defaultMaterialKey; }

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    PhysicsManager(CoreEngine* core);
    ~PhysicsManager();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Get scene object from its rigid body component


    /// @brief Step the physics simulation    void step(float dt);
    void step(float dt);

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
    const char* className() const override { return "PhysicsManager"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PhysicsManager"; }

    /// @}

signals:

public slots:


protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class PhysicsShapePrefab;

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Initialize the manager
    void initialize();

    /// @brief Delete the manager
    void onDelete();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Abstract base class for an application defined memory allocator
    physx::PxDefaultAllocator m_allocator;

    /// @brief User defined interface class. Used by the library to emit debug information
    ErrorCallback m_errorCallback;

    /// @brief  Foundation SDK singleton class. 
    /// @details  Need to have an instance of this class to instance the higher level SDKs
    static physx::PxFoundation* m_foundation;

    /// @brief Abstract singleton factory class used for instancing objects in the Physics SDK.
    /// @details Can use PxPhysics to set global parameters which will effect all scenes, create triangle meshes.
    /// Can get an instance of this class by calling PxCreatePhysics()
    static physx::PxPhysics* m_physics;

    /// @brief A CpuDispatcher is responsible for scheduling the execution of tasks passed to it by the SDK
    static physx::PxDefaultCpuDispatcher* m_dispatcher;

    /// @brief Pointers to all the physics scenes
    static std::vector<std::shared_ptr<PhysicsScene>> m_scenes;

    /// @brief Pointers to all physics shapes
    /// @details Shapes are indexed by name
    static tsl::robin_map<GString, std::shared_ptr<PhysicsShapePrefab>> s_shapes;

    /// @brief Pointers to all baked physics geometry
    static tsl::robin_map<GString, std::shared_ptr<PhysicsGeometry>> s_geometry;

    /// @brief Pointers to all physics materials
    static tsl::robin_map<GString, std::shared_ptr<PhysicsMaterial>> s_materials;

    physx::PxPvd* m_pvd = nullptr;

    static GString s_defaultShapeKey;
    static GString s_defaultMaterialKey;

    /// @}

};

//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif