#ifndef GB_SCENE_OBJECT_H
#define GB_SCENE_OBJECT_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External

// Project
#include <core/scene/GSceneCommon.h>

#include "../containers/GDagNode.h"
#include "../components/GComponent.h"
#include "../containers/GSortingLayer.h"
#include "../rendering/GGLFunctions.h"
#include "../mixins/GLoadable.h"
#include "../containers/GContainerExtensions.h"
#include "../geometry/GCollisions.h"
#include <core/components/GTransformComponent.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CameraComponent;
class SceneCamera;
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
class AudioSourceComponent;
class CubeMapComponent;
class DrawCommand;
class ShadowMap;
class MainRenderer;
class SceneObject;
class ShaderPreset;
class ResourceHandle;
class Blueprint;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class SceneObject
/// @brief  An object to be placed in a GB scene
// TODO: Make scene objects toggleable
// TODO: Make constructors protected
typedef DagNode<SceneObject> SceneObjectDagNode;
class SceneObject: public SceneObjectDagNode, public Nameable, public Serializable{
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
    static std::shared_ptr<SceneObject> Create(Scene* scene, SceneBehaviorFlags flags = SceneBehaviorFlag::kEnabled);
    static std::shared_ptr<SceneObject> Create(CoreEngine* core, const QJsonValue& json);
    static std::shared_ptr<SceneObject> Create(const SceneObject& other);

    /// @brief Get scene object by integer ID
    static std::shared_ptr<SceneObject> Get(size_t id);

    /// @brief Get scene object by name
    static std::shared_ptr<SceneObject> getByName(const GString& name);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Destructor
    /// @{

    //SceneObject(); // Default constructor for Qt metatype declaration
    ~SceneObject();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Properties
    /// @{

    /// @brief Animation bounds in world space
    /// @details Bounds are calculated via updateBounds method by transforming skeleton's bounding box into world space
    const BoundingBoxes& worldBounds() const { return m_worldBounds; }

    /// @brief Set flags directly
    inline void setFlags(SceneBehaviorFlags flags) { m_soFlags = flags; }

    /// @brief Enabled status
    inline bool isEnabled() const { return m_soFlags.testFlag(SceneBehaviorFlag::kEnabled); }
    inline void setEnabled(bool enabled) { m_soFlags.setFlag(SceneBehaviorFlag::kEnabled, enabled); }

    /// @brief Visibility status
    inline bool isVisible() const { return !m_soFlags.testFlag(SceneBehaviorFlag::kInvisible); }
    inline void setVisible(bool visible) { m_soFlags.setFlag(SceneBehaviorFlag::kInvisible, !visible); }

    inline bool isScriptGenerated() const {
        return m_soFlags.testFlag(SceneBehaviorFlag::kScriptGenerated);
    }
    inline void setScriptGenerated(bool val) {
        m_soFlags.setFlag(SceneBehaviorFlag::kScriptGenerated, val);
    }

    /// @brief Return map of components
    std::vector<std::vector<Component*>>& components() { return m_components; }

    /// @brief Obtain pointer to scene containing this object
    Scene* scene() const {
        return m_scene;
    }
    void setScene(Scene* scene) { m_scene = scene; }

    /// @brief The transform component for this scene object
    TransformComponent& transform() { return m_transformComponent; }
    const TransformComponent& transform() const { return m_transformComponent; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Public Methods
    /// @{

    /// @brief Create a blueprint for the scene object and add it to the scenario
    Blueprint& createBlueprint() const;

    /// @brief Determine whether or not any draw commands should be generated for this object
    bool canDraw(SceneCamera& camera,
        MainRenderer& renderer,
        const SortingLayer& currentLayer,
        bool overrideLayer,
        std::shared_ptr<ShaderPreset>& outShaderPreset) const;

    /// @brief Create the commands required to draw this scene object
    void createDrawCommands(SceneCamera& camera,
        MainRenderer& renderer,
        const SortingLayer& currentLayer,
        bool overrideLayer = false); // create draw command regardless of current layer
    
    /// @brief Create commands to draw into a shadow map
    void createDrawCommands(ShadowMap& sm, MainRenderer& renderer); 

    std::vector<SortingLayer*> renderLayers() const;
    bool hasRenderLayer(size_t layerId) const;
    //bool addRenderLayer(const std::shared_ptr<SortingLayer>& layer);

    /// @brief Remove render layer from this scene object and all children
    void removeRenderLayer(size_t layerId);

    std::vector<size_t>& renderLayerIds() { return m_renderLayers; }

    /// @brief Clear models from this object and its children
    void clearModels();

    /// @brief Update the physics for this scene object
    void updatePhysics();

    /// @brief Get component from UUID
    Component* getComponent(const Uuid& uuid, ComponentType type);

    /// @brief Add component to the scene object
    /// @details Returns true if component successfully added
    bool addComponent(Component* component);

    /// @brief Add a component to the scene object, replacing all others of it's type
    inline void setComponent(Component* component) {
        // Delete all components of the given type
        ComponentType type = component->getComponentType();
        if (hasComponent(type)) {
            for (const auto& comp : m_components.at((int)type)) {
                delete comp;
            }
        }
        // Replace components
        m_components[(int)type] = { component };
    }

    /// @brief Remove component from the scene object
    /// @details Returns true if component successfully remopved
    /// \param deletePointer Whether or not to delete the pointer to the removed component
    bool removeComponent(Component* component);

    /// @brief Whether or not component can be added
    bool canAdd(Component* component);

    /// @brief whether or not this scene object satisfies the given component requirements
    bool satisfiesConstraints(const ComponentType& reqs) const;

    /// @brief Search recursively through children for child with the given ID
    std::shared_ptr<SceneObject> getChild(size_t childID) const;
    std::shared_ptr<SceneObject> getChildByName(const GString& name) const;

    /// @brief Switch the scene of the object
    //void switchScene(std::shared_ptr<Scene> newScene);

    /// @brief Return parent object
    std::shared_ptr<SceneObject> parent() const;
    void setParent(const std::shared_ptr<SceneObject>& parent);

    /// @brief Whether the object has the given component type or not
    /// @details Returns nullptr if doesn't have the component, otherwise returns the first component of the given type
    Component* hasComponent(ComponentType type, size_t idx = 0) const {
        if (type == ComponentType::kTransform) {
            // Special case for transform
            return const_cast<TransformComponent*>(&m_transformComponent);
        }

        if (m_components[(int)type].size() <= idx) {
            return nullptr;
        }
        else {
            return m_components[(int)type][idx];
        }
    }

    template<typename T>
    T* hasComponent(ComponentType type, size_t idx = 0) const {
        static_assert(std::is_base_of_v<Component, T>, "Error, converted type must be a component");
        return hasComponent(type, idx)->as<T>();
    }

    /// @brief Remove all shared_ptrs to this scene object so that it can be garbage collected
    void removeFromScene();


    /// @brief Called when a model is loaded, to ensure that bounding boxes are generated if this object has the model as a component
    void onModelLoaded(const std::shared_ptr<ResourceHandle>& modelHandle);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "SceneObject"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::SceneObject"; }
 
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

private:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name DagNode overrides
    /// @{

    virtual void onAddChild(const std::shared_ptr<SceneObject>& child) override;
    virtual void onAddParent(const std::shared_ptr<SceneObject>& parent) override;
    virtual void onRemoveChild(const std::shared_ptr<SceneObject>& child) override;
    virtual void onRemoveParent(const std::shared_ptr<SceneObject>& parent) override;

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Friends
    /// @{
    
    friend class Transform;
    friend class ModelComponent;
    friend class TransformComponent;
    friend class BoneAnimationComponent;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    SceneObject(Scene* scene);
    //SceneObject(CoreEngine* core);

    /// @}


    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Protected Methods
    /// @{

    /// @brief Update the world bounds of the scene object
    void updateBounds(const Transform& transform);

    /// @brief Set uniforms needed to render this scene object using the given command
    void bindUniforms(DrawCommand& rendercommand);

    void setDefaultRenderLayers();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{    

    /// @brief Whether or not the scene object is enabled
    SceneBehaviorFlags m_soFlags;

    /// @brief A pointer to the scene containing this object
    Scene* m_scene;

    /// @brief The world-space bounds of the scene object
    BoundingBoxes m_worldBounds;

    /// @brief Rendering layer IDs
    /// @details Made mutable to access via renderLayers routine
    std::vector<size_t> m_renderLayers;
    
    /// @brief The transform for this scene object
    TransformComponent m_transformComponent;

    /// @brief The components attached to this scene object
    /// @details Main map is indexed by component type, key for sub-map is UUID
    // TODO: Move components to the scene, and store in sorted vectors
    std::vector<std::vector<Component*>> m_components;

    /// @}
};
typedef std::shared_ptr<SceneObject> SceneObjectPtr;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
