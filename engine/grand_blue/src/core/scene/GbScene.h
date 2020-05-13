#ifndef GB_SCENE_H
#define GB_SCENE_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes
#include <set>

// External

// Project
#include "../mixins/GbLoadable.h"
#include "../GbObject.h"
#include "../rendering/shaders/GbUniform.h"
#include "../components/GbComponent.h"
#include "../containers/GbContainerExtensions.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SceneObject;
class Scenario;
class CameraComponent;
class CanvasComponent;
class Light;
class CoreEngine;
class Renderer;
class ShaderProgram;
class CubeMap;
class PhysicsScene;
class PhysicsSceneComponent;
class Color;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Type Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @struct CompareByRenderLayer
/// @brief Struct containing a comparator for sorting scene objects list by render sorting layer
struct CompareByRenderLayer {
    bool operator()(const std::shared_ptr<SceneObject>& a, const std::shared_ptr<SceneObject>& b) const;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @class Scene
    @brief A scene consisting of Scene Objects
*/
class Scene: public Object, public Loadable, std::enable_shared_from_this<Scene>{
public:
    typedef std::multiset<std::shared_ptr<SceneObject>, CompareByRenderLayer> SceneObjectSet;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    static std::shared_ptr<Scene> create(Scenario* scenario);
    static std::shared_ptr<Scene> create(CoreEngine* engine); // For debug manager

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors
    /// @{
    Scene();
    ~Scene();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb Object Properties
    /// @{
    /// @property className
    /// @brief The name of this class
    const char* className() const override { return "Scene"; }

    /// @property namespaceName
    /// @brief The full namespace for this class
    const char* namespaceName() const override { return "Gb::Scene"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{    

    /// @brief The components for this scene
    std::unordered_map<Component::ComponentType, std::vector<Component*>>& components() { return m_components; }

    /// @property Scenario
    Scenario* scenario() const;

    /// @property Skybox
    std::shared_ptr<CubeMap> skybox() { return m_skybox; }
    void setSkybox(std::shared_ptr<CubeMap> skybox) { m_skybox = skybox; }

    /// @brief Return objects in the scene
    SceneObjectSet& topLevelSceneObjects() { return m_topLevelSceneObjects; }

    /// @brief Return cameras in the scene
    std::vector<CameraComponent*>& cameras();

    /// @brief Return all canvases in the scene
    std::vector<CanvasComponent*>& canvases();

    /// @brief Return renderers in the scene
    const std::vector<std::shared_ptr<Renderer>> renderers() const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{    

    /// @brief Adds a camera to this scene's cache of cameras on component contruction
    void addCamera(CameraComponent* camera);
    void removeCamera(CameraComponent* camera);

    /// @brief Adds a canvas's object to this scene's cache of canvas objects on component contruction
    void addCanvas(CanvasComponent* canvas);
    void removeCanvas(CanvasComponent* canvas);

    /// @brief Physics scene
    std::shared_ptr<PhysicsScene> physics();

    /// @brief Physics scene component
    PhysicsSceneComponent* physicsComponent();

    /// @brief Add physics to the scene
    void addPhysics();

    /// @brief Obtain scene object by UUID
    std::shared_ptr<SceneObject> getSceneObject(const Uuid& uuid) const;
    std::shared_ptr<SceneObject> getSceneObjectByName(const QString& name) const;

    /// @brief Whether the scene has the object or not
    bool hasTopLevelObject(const std::shared_ptr<SceneObject>& object);

    /// @brief Add an object to the scene
    void addObject(const std::shared_ptr<SceneObject>& object);

    /// @brief Remove an object from the scene
    void removeObject(const std::shared_ptr<SceneObject>& object, bool eraseFromNodeMap = false);

    /// @brief Clear the scene
    /// @details Removes all scene objects from static map
    void clear();

    /// @brief Set uniforms in the given shader program
    void bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram);

    /// @brief Iterator in scene map of given object
    SceneObjectSet::iterator getIterator(const std::shared_ptr<SceneObject>& object);

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
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

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

    Scene(CoreEngine* engine);
    Scene(Scenario* scenario);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{  

    /// @brief Add a component to the scene object, replacing all others of it's type
    void setComponent(Component* component);

    /// @brief Retrieve pointer to the engine
    CoreEngine* engine() const;

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
    CoreEngine* m_engine;

    /// @brief The skybox for this scene
    std::shared_ptr<CubeMap> m_skybox;

    /// @brief The components attached to this scene
    std::unordered_map<Component::ComponentType, std::vector<Component*>> m_components;

    /// @brief Set of top-level objects in this scene
    /// @details Set is sorted by the render layer of each scene object
    /// @note Must not modify in a way that would affect sorting order once added to set
    SceneObjectSet m_topLevelSceneObjects;

    /// @brief Map of uniforms for the scene
    std::unordered_map<QString, Uniform> m_uniforms;

    /// @brief Map of all canvas components, was storing scene objects, which was causing ownership issues
    std::vector<CanvasComponent*> m_canvases;

    /// @brief Map of all cameras in the scene
    std::vector<CameraComponent*> m_cameras;

    /// @}

};
typedef std::shared_ptr<Scene> ScenePtr;
Q_DECLARE_METATYPE(ScenePtr)
Q_DECLARE_METATYPE(Scene)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
