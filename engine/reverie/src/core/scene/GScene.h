#ifndef GB_SCENE_H
#define GB_SCENE_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes
#include <set>

// External

// Project
#include "../mixins/GLoadable.h"
#include "../GObject.h"
#include "../rendering/shaders/GUniform.h"
#include "../components/GComponent.h"
#include "../containers/GContainerExtensions.h"
#include "../geometry/GCollisions.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SceneObject;
class Scenario;
class CameraComponent;
class CanvasComponent;
class CubeMapComponent;
class Light;
class CoreEngine;
class Renderer;
class ShaderProgram;
class PhysicsScene;
class PhysicsSceneComponent;
class Color;
class DrawCommand;
class MainRenderer;
class WorldRay;
struct WorldRayHit;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Type Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** @class Scene
    @brief A scene consisting of Scene Objects
*/
class Scene: public Object, public Nameable, public Identifiable, public Loadable, public std::enable_shared_from_this<Scene>{
public:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors
    /// @{

    Scene();
    Scene(CoreEngine* engine);
    Scene(Scenario* scenario);
    ~Scene();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Object Properties
    /// @{
    /// @property className
    /// @brief The name of this class
    const char* className() const override { return "Scene"; }

    /// @property namespaceName
    /// @brief The full namespace for this class
    const char* namespaceName() const override { return "rev::Scene"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{    

    /// @brief Retrieve pointer to the engine
    CoreEngine* engine() const;

    /// @brief The components for this scene
    std::vector<std::vector<Component*>>& components() { return m_components; }

    /// @property Scenario
    Scenario* scenario() const;

    /// @property Skybox
    //std::shared_ptr<CubeMap> skybox() { return m_skybox; }
    //void setSkybox(std::shared_ptr<CubeMap> skybox) { m_skybox = skybox; }

    /// @brief Return objects in the scene
    std::vector<std::shared_ptr<SceneObject>>& topLevelSceneObjects() { return m_topLevelSceneObjects; }

    /// @brief Return cameras in the scene
    std::vector<CameraComponent*>& cameras();
    const std::vector<CameraComponent*>& cameras() const;

    /// @brief Return all canvases in the scene
    std::vector<CanvasComponent*>& canvases();

    /// @brief Return all cubemap components in the scene
    std::vector<CubeMapComponent*>& cubeMaps();

    /// @brief Default skybox
    CubeMapComponent* defaultCubeMap() { return m_defaultCubeMap; }
    void setDefaultCubeMap(CubeMapComponent* map) { m_defaultCubeMap = map; }


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{    

    /// @brief Raycast all 3D objects in the scene
    void raycast(const WorldRay& ray, std::vector<WorldRayHit>& outHits);

    /// @brief Get frustum containing all currently visible scene geometry
    const AABB& getVisibleFrustumBounds() const {
        return m_viewBounds;
    }
    void updateVisibleFrustumBounds();

    /// @brief Generate the draw commands for the scene
    void createDrawCommands(MainRenderer& renderer);

    /// @brief Adds a camera to this scene's cache of cameras on component contruction
    void addCamera(CameraComponent* camera);
    void removeCamera(CameraComponent* camera);

    /// @brief Adds a canvas's object to this scene's cache of canvas objects on component contruction
    void addCanvas(CanvasComponent* canvas);
    void removeCanvas(CanvasComponent* canvas);

    /// @brief Adds a cubemap's object to this scene's cache of cubemap objects on component contruction
    void addCubeMap(CubeMapComponent* map);
    void removeCubeMap(CubeMapComponent* map);
    CubeMapComponent* getCubeMap(const Uuid& uuid);

    /// @brief Physics scene
    std::shared_ptr<PhysicsScene> physics();

    /// @brief Physics scene component
    PhysicsSceneComponent* physicsComponent();

    /// @brief Add physics to the scene
    void addPhysics();

    /// @brief Obtain scene object by UUID
    std::shared_ptr<SceneObject> getSceneObject(size_t id) const;
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

    /// @brief Set uniforms to the given render command
    void bindUniforms(DrawCommand& command);

    /// @brief Iterator in scene map of given object
    std::vector<std::shared_ptr<SceneObject>>::iterator getIterator(const std::shared_ptr<SceneObject>& object);

    /// @brief Add component to the scene object
    /// @details Returns true if component successfully added
    bool addComponent(Component* component);

    /// @brief Remove component from the scene object
    /// @details Returns true if component successfully remopved
    /// \param deletePointer Whether or not to delete the pointer to the removed component
    bool removeComponent(Component* component, bool deletePointer = true);

    /// @brief Whether or not component can be added
    bool canAdd(Component* component);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Loadable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends 
    /// @{

    friend class Light;
    friend class Scenario;
    friend class SceneObject;
    friend class PhysicsScene;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors
    /// @{  

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{  

    /// @brief Add a component to the scene object, replacing all others of it's type
    void setComponent(Component* component);

    /// @brief Initialize a scene
    void initialize();

    /// @brief Post-construct a scene
    void postConstruction();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{    

    /// @brief Pointer to scenario
    /// @note Since scenario is a QObject, this raw pointer is safe
    Scenario* m_scenario;

    /// @brief Pointer to the engine
    // TODO:
    // This is really for debug scene, so maybe can be smarter about this, redundant with m_scenario
    CoreEngine* m_engine;

    /// @brief The bounds encompassing all view frustums in the scene
    AABB m_viewBounds;

    /// @brief The components attached to this scene
    std::vector<std::vector<Component*>> m_components;

    /// @brief Vector of top-level objects in this scene
    std::vector<std::shared_ptr<SceneObject>> m_topLevelSceneObjects;

    /// @brief Vector of uniforms for the scene
    std::vector<Uniform> m_uniforms;

    /// @brief Vector of all canvas components, was storing scene objects, which was causing ownership issues
    std::vector<CanvasComponent*> m_canvases;

    /// @brief Vector of all cameras in the scene
    std::vector<CameraComponent*> m_cameras;

    /// @brief Map of all cubemap in the scene
    std::vector<CubeMapComponent*> m_cubeMaps;
    CubeMapComponent* m_defaultCubeMap = nullptr;

    /// @}

};
typedef std::shared_ptr<Scene> ScenePtr;
Q_DECLARE_METATYPE(ScenePtr)
Q_DECLARE_METATYPE(Scene)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
