#ifndef GB_SCENE_OBJECT_H
#define GB_SCENE_OBJECT_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External

// Project
#include "../containers/GbDagNode.h"
#include "../components/GbComponent.h"
#include "../containers/GbSortingLayer.h"
#include "../rendering/GbGLFunctions.h"
#include "../mixins/GbLoadable.h"
#include "../containers/GbContainerExtensions.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TransformComponent;
class CameraComponent;
class Camera;
class LightComponent;
class Scene;
class ModelComponent;
class ShaderComponent;
class CanvasComponent;
class Renderer;
class ShaderProgram;
struct CompareByRenderLayer;
class Renderable;
class CharControlComponent;
class BoneAnimationComponent;
class CubeMapComponent;
class DrawCommand;
class MainRenderer;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** @class SceneObject
    @brief  An object to be placed in a GB scene
*/
// TODO: Make scene objects toggleable
// TODO: Make constructors protected
class SceneObject: public DagNode, public Serializable, public Persistable {
public:
    typedef std::shared_ptr<Scene> ScenePtr;
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum RenderableType {
        kModel,
        kCanvas
    };

    /// @brief Create a shared pointer to a DAG node (since the constructor is private)
    static std::shared_ptr<SceneObject> create(std::shared_ptr<Scene> scene);
    static std::shared_ptr<SceneObject> create(CoreEngine* core, const QJsonValue& json);

    /// @brief Get scene object by UUID
    static std::shared_ptr<SceneObject> get(const Uuid& uuid);
    static std::shared_ptr<SceneObject> get(const QString& uuidStr);

    /// @brief Get scene object by name
    static std::shared_ptr<SceneObject> getByName(const QString& name);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name DAG node overrides
    /// @{

    /// @property Node Type
    /// @brief DAG node type
    static inline NodeType type() { return NodeType::kSceneObject; }

    /// @} 

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Destructor
    /// @{

    SceneObject(); // Default constructor for Qt metatype declaration
    ~SceneObject();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Properties
    /// @{

    /// @brief Enabled status
    bool isEnabled() const { return m_isEnabled; }
    void setEnabled(bool enabled) { m_isEnabled = enabled; }

    /// @brief Return map of components
    std::vector<std::vector<Component*>>& components() { return m_components; }

    /// @brief Return the engine used by this scene object
    CoreEngine* engine() const { return m_engine; }

    /// @brief Obtain pointer to scene containing this object
    std::shared_ptr<Scene> scene() const;
    void setScene(std::shared_ptr<Scene> scene) { m_scene = scene; }

    /// @brief The transform component for this scene object
    const std::shared_ptr<TransformComponent>& transform() { return m_transformComponent; }

    /// @brief Access and set a shader for this scene object
    ShaderComponent* shaderComponent();

    /// @property Camera
    /// @brief Access and set a camera to this scene object
    CameraComponent* camera();
    void setCamera(CameraComponent* camera);

    /// @property Character Controller
    CharControlComponent* characterController();

    /// @property Model Component
    ModelComponent* modelComponent();

    /// @property Canvas Component
    CanvasComponent* canvasComponent();

    /// @property Bone animation componnet
    BoneAnimationComponent* boneAnimation();

    /// @property Light
    /// @brief Access a light for this scene object
    LightComponent* light();

    /// @property CubeMap
    CubeMapComponent* cubeMap();


    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Public Methods
    /// @{

    /// @brief Create the commands required to draw this scene object
    void createDrawCommands(Camera& camera,
        MainRenderer& renderer,
        const SortingLayer& currentLayer,
        bool overrideLayer = false); // create draw command regardless of current layer

    std::vector<std::shared_ptr<SortingLayer>> renderLayers();
    std::vector<std::shared_ptr<SortingLayer>> getRenderLayers() const;
    bool hasRenderLayer(const QString& label);
    bool addRenderLayer(const std::shared_ptr<SortingLayer>& layer);
    bool removeRenderLayer(const QString& label);

    std::vector<std::weak_ptr<SortingLayer>>& _renderLayers() {
        return m_renderLayers;
    }

    /// @brief Clear models from this object and its children
    void clearModels();

    /// @brief Update the physics for this scene object
    void updatePhysics();

    /// @brief Abort all scripted processes associated with this object
    void abortScriptedProcesses();

    /// @brief Get component from UUID
    Component* getComponent(const Uuid& uuid, Component::ComponentType type);

    /// @brief Add component to the scene object
    /// @details Returns true if component successfully added
    bool addComponent(Component* component);

    /// @brief Remove component from the scene object
    /// @details Returns true if component successfully remopved
    /// \param deletePointer Whether or not to delete the pointer to the removed component
    bool removeComponent(Component* component);

    /// @brief Whether or not component can be added
    bool canAdd(Component* component);

    /// @brief whether or not this scene object satisfies the given component requirements
    bool satisfiesConstraints(const Component::ComponentType& reqs) const;

    /// @brief Obtain direct child scene objects
    std::vector<std::shared_ptr<SceneObject>> children() const;

    /// @brief Search recursively through children for child with the given UUID
    std::shared_ptr<SceneObject> getChild(const Uuid& uuid) const;
    std::shared_ptr<SceneObject> getChildByName(const QString& name) const;

    /// @brief Switch the scene of the object
    void switchScene(std::shared_ptr<Scene> newScene);

    /// @brief Return parent object
    std::shared_ptr<SceneObject> parent() const;
    void setParent(const std::shared_ptr<SceneObject>& parent);

    /// @brief Whether the object has the given component type or not
    bool hasComponent(Component::ComponentType type) const;

    /// @brief Whether the object has a camera or not
    bool hasCamera() const;

    /// @brief Whether or not the object has a shader
    bool hasShaderComponent() const;

    ///// @brief Method to render the scene object, given a renderable type
    //void draw(RenderableType type);

    /// @brief Remove all shared_ptrs to this scene object so that it can be garbage collected
    void remove();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "SceneObject"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::SceneObject"; }
 
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
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
    
    friend class Transform;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    SceneObject(const std::shared_ptr<Scene>& scene);
    //SceneObject(CoreEngine* core);

    /// @}


    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Protected Methods
    /// @{

    /// @brief Add a component to the scene object, replacing all others of it's type
    void setComponent(Component* component);

    /// @brief Set uniforms needed to render this scene object using the given command
    void bindUniforms(DrawCommand& rendercommand);

    void setDefaultRenderLayers();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{    

    /// @brief Whether or not the scene object is enabled
    bool m_isEnabled;

    /// @brief A pointer to the scene containing this object
    std::shared_ptr<Scene> m_scene;

    /// @brief Rendering layers
    std::vector<std::weak_ptr<SortingLayer>> m_renderLayers;

    //TODO: User-defined metadata
    
    /// @brief The transform for this scene object
    std::shared_ptr<TransformComponent> m_transformComponent;

    /// @brief The components attached to this scene object
    /// @details Main map is indexed by component type, key for sub-map is UUID
    // TODO: Move components to the scene, and store in sorted vectors
    std::vector<std::vector<Component*>> m_components;

    /// @brief Pointer to the core engine
    CoreEngine* m_engine;

    /// @}
};
typedef std::shared_ptr<SceneObject> SceneObjectPtr;
Q_DECLARE_METATYPE(SceneObjectPtr)
Q_DECLARE_METATYPE(SceneObject)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
