#ifndef GB_PHYSICS_SCENE_H
#define GB_PHYSICS_SCENE_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// External
#include <PxPhysicsAPI.h>

// QT

// Internal
#include "../GObject.h"
#include "../mixins/GLoadable.h"
#include "../geometry/GVector.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class PhysicsActor;
class Scene;
class PhysicsRaycast;
class SceneObject;
class CCTManager;

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

/// @class PhysicsScene
/// @brief Class representing a physics scene
class PhysicsScene : public Object, public Identifiable, public Serializable {
public:

    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    static std::shared_ptr<PhysicsScene> PhysicsScene::create(Scene* scene);
    //static std::shared_ptr<PhysicsScene> PhysicsScene::create(const Vector3f& gravity);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Destructor
    /// @{
    ~PhysicsScene();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    Scene* scene() const {
        if (m_pxScene) {
            return (Scene*)m_pxScene->userData;
        }
        else {
            return nullptr;
        }
    }

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    physx::PxScene* pxScene() { return m_pxScene; }
    const physx::PxScene* pxScene() const { return m_pxScene; }

    /// @brief Return character controller manager
    const std::shared_ptr<CCTManager>& cctManager() const { return m_cctManager; }
    void createCctManager();

    /// @brief Perform a raycast for this scene, returning true if at least one hit was obtained
    bool raycast(PhysicsRaycast& cast) const;

    /// @brief Get actors
    std::vector<physx::PxActor*> actors();
    std::vector<physx::PxActor*> dynamicActors();

    /// @brief Get active actors
    std::vector<physx::PxActor*> activeActors();

    /// @brief Step forward in the simulation
    void simulate(float dt) {
        m_pxScene->simulate(dt);
    }

    /// @brief Fetch results from the most recent simulation step
    void fetchResults(bool block);

    /// @brief Set gravity
    Vector3f getGravity() const;
    void setGravity(const Vector3f& gravity);

    /// @brief Add actor to the scene
    void addActor(PhysicsActor* actor);

    /// @brief Remove an actor from the scene
    /// @details If the actor is not part of this scene (see PxActor::getScene), the call is ignored and an error is issued.
    /// You can not remove individual articulation links(see PxArticulationLink) from the scene.Use removeArticulation() instead.
    /// If the actor is a PxRigidActor then all assigned PxConstraint objects will get removed from the scene automatically.
    /// If the actor is in an aggregate it will be removed from the aggregate.
    void removeActor(PhysicsActor* actor);

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
    const char* className() const override { return "PhysicsScene"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::PhysicsScene"; }

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors
    /// @{
    PhysicsScene(Scene* scene);
    //PhysicsScene(const Vector3f& gravity);
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Initialize the scene
    void initialize(Scene* scene);

    /// @brief Initialize the scene description
    void initializeDescription();

    /// @brief Delete the scene
    void onDelete();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The character controller manager for this scene
    std::shared_ptr<CCTManager> m_cctManager = nullptr;

    /// @brief PhysX scene wrapped by this physics scene
    physx::PxScene* m_pxScene;

    /// @brief Description of the physx scene, used for initialization
    physx::PxSceneDesc m_description;

    /// @}

};

//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif