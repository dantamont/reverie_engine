#pragma once

// Standard Includes
#include <set>
#include <vector>

// External

// Project
#include "fortress/containers/GStrictGrowContainer.h"
#include "fortress/types/GLoadable.h"
#include "fortress/types/GNameable.h"
#include "fortress/types/GIdentifiable.h"
#include "core/rendering/shaders/GUniform.h"
#include "core/components/GComponent.h"
#include "fortress/containers/GContainerExtensions.h"
#include "heave/collisions/GCollisions.h"

#include "enums/GSimulationPlayModeEnum.h"

namespace rev {

class SceneObject;
class Scenario;
class ModelComponent;
class CameraComponent;
class CanvasComponent;
class CubeMapComponent;
class ShaderComponent;
class LightComponent;
class Light;
class CoreEngine;
class Renderer;
class ShaderProgram;
class PhysicsScene;
class PhysicsSceneComponent;
class Color;
class DrawCommand;
class OpenGlRenderer;
class WorldRay;
struct WorldRayHit;

/// @brief Flags for the status of the dlag
//enum class GSceneObjectStatusFlag {
//    eStaleUniforms = 1 << 0 ///< True if the uniforms for this scene are stale
//};
//MAKE_FLAGS(GSceneObjectStatusFlag, GSceneObjectStatusFlags)

/// @class Scene
/// @brief A scene consisting of Scene Objects
class Scene: public NameableInterface, public IdentifiableInterface, public LoadableInterface, public std::enable_shared_from_this<Scene>{
public:

    /// @name Constructors
    /// @{

    Scene();
    Scene(CoreEngine* engine);
    Scene(Scenario* scenario);
    ~Scene();

    /// @}

    /// @name Properties
    /// @{    

    /// @brief Retrieve pointer to the engine
    CoreEngine* engine() const;

    /// @brief The components for this scene
    std::vector<Component*>& components() { return m_components; }

    /// @property Scenario
    Scenario* scenario() const;
    void setScenario(Scenario* scenario);

    /// @brief Return objects in the scene
    std::vector<std::shared_ptr<SceneObject>>& topLevelSceneObjects() { return m_topLevelSceneObjects; }

    /// @brief Return cameras in the scene
    std::vector<CameraComponent*>& cameras();
    const std::vector<CameraComponent*>& cameras() const;

    /// @brief Return the models in the scene
    std::vector<ModelComponent*>& models() { return m_models; }

    /// @brief Return all canvases in the scene
    std::vector<CanvasComponent*>& canvases();

    /// @brief Return all cubemap components in the scene
    std::vector<CubeMapComponent*>& cubeMaps();

    /// @brief Default skybox
    CubeMapComponent* defaultCubeMap() { return m_defaultCubeMap; }
    void setDefaultCubeMap(CubeMapComponent* map) { m_defaultCubeMap = map; }


    /// @}

    /// @name Public methods
    /// @{    

    Component* getComponent(const Uuid& uuid);
    Component* getComponent(ComponentType type);

    /// @brief Raycast all 3D objects in the scene
    void raycast(const WorldRay& ray, std::vector<WorldRayHit>& outHits);

    /// @brief Get frustum containing all currently visible scene geometry
    const AABB& getVisibleFrustumBounds() const {
        return m_viewBounds;
    }
    void updateVisibleFrustumBounds();

    /// @brief Generate the draw commands for the scene
    void retrieveDrawCommands(OpenGlRenderer& renderer);

    /// @brief Adds a camera to this scene's cache of cameras on component contruction
    void addCamera(CameraComponent* camera);
    void removeCamera(CameraComponent* camera);

    /// @brief Adds a canvas's object to this scene's cache of canvas objects on component contruction
    void addCanvas(CanvasComponent* canvas);
    void removeCanvas(CanvasComponent* canvas);

    /// @brief Adds a model component to the scene's cache
    void addModel(ModelComponent* model);
    void removeModel(ModelComponent* model);

    /// @brief Adds a cubemap's object to this scene's cache of cubemap objects on component contruction
    void addCubeMap(CubeMapComponent* map);
    void removeCubeMap(CubeMapComponent* map);
    CubeMapComponent* getCubeMap(const Uuid& uuid);

    /// @brief Adds a light component to the scene's cache
    void addLight(LightComponent* light);
    void removeLight(LightComponent* light);

    /// @brief Physics scene
    std::shared_ptr<PhysicsScene> physics();

    /// @brief Physics scene component
    PhysicsSceneComponent* physicsComponent();

    /// @brief Add physics to the scene
    void addPhysics();

    /// @brief Obtain scene object by UUID
    std::shared_ptr<SceneObject> getSceneObject(uint32_t id) const;
    std::shared_ptr<SceneObject> getSceneObjectByName(const GString& name) const;

    /// @brief Whether the scene has the object or not
    bool hasTopLevelObject(const std::shared_ptr<SceneObject>& object);

    /// @brief Add an object to the scene
    void addObject(const std::shared_ptr<SceneObject>& object);

    /// @brief Remove an object from the scene entirely
    /// @note Need to pass pointer as a value, so ref count doesn't hit zero and cause problems
    void removeObject(std::shared_ptr<SceneObject> object, bool eraseFromTopLevel = true);

    /// @brief Remove object from top level list
    void demoteObject(const std::shared_ptr<SceneObject>& object);

    /// @brief Clear the scene
    /// @details Removes all scene objects from static map
    void clear();

    /// @brief Iterator in scene map of given object
    std::vector<std::shared_ptr<SceneObject>>::iterator getIterator(const std::shared_ptr<SceneObject>& object);

    /// @brief Set component for the scene object
    void setComponent(Component* component);

    /// @brief Remove component from the scene object
    /// @details Returns true if component successfully remopved
    void removeComponent(Component* component);
    void removeComponent(const Uuid& componentId);

    /// @brief Clear and recreate all shadow draw commands
    void recreateAllShadowDrawCommands();

    /// @}

    /// @name LoadableInterface Overrides
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Scene& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Scene& orObject);


    /// @}

protected:
    /// @name Friends 
    /// @{
    friend class Scene; ///< @todo Remove, just used to create mesh commands
    friend class Light;
    friend class Scenario;
    friend class SceneObject;
    friend class PhysicsScene;

    /// @}

    /// @name Protected methods
    /// @{  

    /// @brief Initialize a scene
    void initialize();

    void initializeSignals();

    /// @brief Post-construct a scene
    void postConstruction();

    /// @brief What to do on resource load
    void onResourceLoaded(Uuid resourceId);

    /// @brief Disconnect this scene's signals
    void disconnectSignals();

    /// @brief Clear and recreate all draw commands
    void recreateAllDrawCommands(GSimulationPlayMode playMode);
    /// @}

    /// @name Protected members
    /// @{    

    Scenario* m_scenario; ///< Pointer to scenario

    /// @todo This is really for debug scene, so maybe can be smarter about this, redundant with m_scenario
    CoreEngine* m_engine; ///< Pointer to the engine

    //GSceneObjectStatusFlags m_statusFlags{ 0 };
    AABB m_viewBounds; ///< The bounds encompassing all view frustums in the scene
    
    std::vector<Component*> m_components; ///< The components attached to this scene
    std::vector<std::shared_ptr<SceneObject>> m_topLevelSceneObjects; ///< Vector of top-level objects in this scene

    StrictGrowContainer<std::vector<Matrix4x4>> m_worldMatrices; ///< The vector of world matrices for the scene object
    std::vector<ModelComponent*> m_models; ///< Vector of all model components in the scene
    std::vector<CanvasComponent*> m_canvases; ///< Vector of all canvas components, was storing scene objects, which was causing ownership issues
    std::vector<CameraComponent*> m_cameras; ///< Vector of all cameras in the scene
    std::vector<CubeMapComponent*> m_cubeMaps; ///< Vector of all cubemap in the scene
    std::vector<LightComponent*> m_lights; ///< Map of all cubemap in the scene
    CubeMapComponent* m_defaultCubeMap = nullptr;

    Int32_t m_resourceAddedId{ -1 }; ///< Connection index to resource added signal
    Int32_t m_playModeChangedConnectionId{ -1 }; ///< Connection index that play mode changed

    /// @}

};
typedef std::shared_ptr<Scene> ScenePtr;


} // end namespacing

