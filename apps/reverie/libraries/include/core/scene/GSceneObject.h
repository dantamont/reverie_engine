#pragma once

// Project
#include "fortress/types/GNameable.h"
#include "fortress/containers/graph/GDagNode.h"
#include "fortress/containers/GSortingLayer.h"
#include "fortress/types/GLoadable.h"
#include "fortress/containers/GContainerExtensions.h"

#include "core/scene/GSceneBehaviorFlags.h"
#include "core/components/GComponent.h"
#include "core/rendering/GGLFunctions.h"
#include "heave/collisions/GCollisions.h"
#include "core/components/GTransformComponent.h"

namespace rev {

class AbstractCamera;
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
class OpenGlRenderer;
class SceneObject;
class ShaderPreset;
class ResourceHandle;
class Blueprint;

/// @class SceneObject
/// @brief  An object to be placed in a GB scene
/// @todo Make scene objects toggleable
typedef DagNode<SceneObject> SceneObjectDagNode;
class SceneObject: public SceneObjectDagNode, public NameableInterface{
public:
    typedef std::shared_ptr<Scene> ScenePtr;
    /// @name Static
    /// @{

    enum RenderableType {
        kModel,
        kCanvas
    };

    /// @brief Create a shared pointer to a DAG node (since the constructor is private)
    static std::shared_ptr<SceneObject> Create(Scene* scene, SceneBehaviorFlags flags = SceneBehaviorFlag::kEnabled);
    static std::shared_ptr<SceneObject> Create(CoreEngine* core, const nlohmann::json& json);
    static std::shared_ptr<SceneObject> Create(const SceneObject& other);

    /// @brief Get scene object by integer ID
    static std::shared_ptr<SceneObject> Get(uint32_t id);

    /// @brief Get scene object by name
    static std::shared_ptr<SceneObject> getByName(const GString& name);

    /// @}

    /// @name Destructor
    /// @{

    ~SceneObject();

    /// @}

    /// @name Properties
    /// @{

    /// @brief Animation bounds in world space
    /// @details Bounds are calculated via updateBounds method by transforming skeleton's bounding box into world space
    const BoundingBoxes& worldBounds() const { return m_worldBounds; }

    /// @brief Set flags directly
    inline void setFlags(SceneBehaviorFlags flags) { m_soFlags = flags; }
    inline void setMoved(bool hasMoved) {
        m_soFlags.setFlag(SceneBehaviorFlag::kMoved, hasMoved);
    }

    /// @brief Whether or not the scene object moved in the last frame
    inline bool objectMoved() const {
        return m_soFlags.testFlag(SceneBehaviorFlag::kMoved);
    }

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

    /// @brief Return vector of components
    std::vector<Component*>& components() { return m_components; }

    /// @brief Obtain pointer to scene containing this object
    Scene* scene() const {
        return m_scene;
    }
    void setScene(Scene* scene) { m_scene = scene; }

    /// @brief The transform component for this scene object
    const TransformComponent& transform() const { return m_transformComponent; }
    TransformComponent& transform() { return m_transformComponent; }

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Create a blueprint for the scene object and add it to the scenario
    Blueprint& createBlueprint() const;

    /// @brief Determine whether or not any draw commands should be generated for this object
    bool canDraw(SceneCamera& camera,
        const SortingLayer& currentLayer,
        bool overrideLayer,
        std::shared_ptr<const ShaderPreset>& outShaderPreset) const;

    /// @brief Create the commands required to draw this scene object
    void retrieveDrawCommands(SceneCamera& camera,
        OpenGlRenderer& renderer,
        const SortingLayer& currentLayer,
        bool overrideLayer = false); // create draw command regardless of current layer

    /// @brief Create draw commands for the model component
    /// @note Assumes model is loaded
    void createModelDrawCommands();
    
    /// @brief Create shadow draw commands for the model component
    /// @note Assumes model is loaded
    void createShadowDrawCommands();

    /// @brief Create commands to draw into a shadow map
    void retrieveShadowDrawCommands(OpenGlRenderer& renderer, ShadowMap* shadowMap);

    std::vector<SortingLayer> renderLayers() const;

    bool hasRenderLayer(uint32_t layerId) const;

    void addRenderLayer(Uint32_t layerId);

    /// @brief Remove render layer from this scene object and all children
    void removeRenderLayer(uint32_t layerId);

    const std::vector<uint32_t>& renderLayerIds() const { return m_renderLayers; }

    /// @brief Update the physics for this scene object
    void updatePhysics();

    /// @brief Add a component to the scene object, replacing all others of its type
    void setComponent(Component* component);

    /// @brief Remove component from the scene object
    /// @details Returns true if component successfully remopved
    bool removeComponent(Component* component);
    bool removeComponent(const Uuid& componentId);

    /// @brief Whether or not component can be added
    bool canAdd(Component* component);

    /// @brief Search recursively through children for child with the given ID
    std::shared_ptr<SceneObject> getChild(uint32_t childID) const;
    std::shared_ptr<SceneObject> getChildByName(const GString& name) const;

    /// @brief Return parent object
    std::shared_ptr<SceneObject> parent() const;
    void setParent(const std::shared_ptr<SceneObject>& parent);

    /// @brief Whether the object has the given component type or not
    /// @details Returns nullptr if doesn't have the component, otherwise returns the component
    Component* getComponent(ComponentType type) const;

    template<typename T>
    T* getComponent(ComponentType type) const {
        static_assert(std::is_base_of_v<Component, T>, "Error, converted type must be a component");
        return getComponent(type)->as<T>();
    }

    /// @brief Remove all shared_ptrs to this scene object so that it can be garbage collected
    void removeFromScene();

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const SceneObject& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, SceneObject& orObject);


    /// @}

private:
    /// @name DagNode overrides
    /// @{

    virtual void onAddChild(const std::shared_ptr<SceneObject>& child) override;
    virtual void onAddParent(const std::shared_ptr<SceneObject>& parent) override;
    virtual void onRemoveChild(const std::shared_ptr<SceneObject>& child) override;
    virtual void onRemoveParent(const std::shared_ptr<SceneObject>& parent) override;

    /// @}

protected:
    /// @name Friends
    /// @{
    friend class SceneCamera; /// @todo Remove, for persistent draw commands
    friend class Scene; /// @todo Remove
    friend class rev::TransformTemplate<Matrix4x4>;
    friend class ModelComponent;
    friend class TransformComponent;
    friend class BoneAnimationComponent;

    /// @}

    /// @name Constructors/Destructor
    /// @{

    SceneObject(Scene* scene);

    /// @}

    /// @name Protected Methods
    /// @{

    /// @brief Update the world bounds of the scene object
    void updateBounds(const IndexedTransform& transform);

    /// @brief Set uniforms needed to render this scene object using the given command
    void applyWorldUniform(DrawCommand& rendercommand);

    void setDefaultRenderLayers();

    /// @brief Update draw commands on render layer added or removed
    void onRenderLayerAdded(Uint32_t layerId);
    void onRenderLayerRemoved(Uint32_t layerId);

    /// @}

    /// @name Protected members
    /// @{    

    SceneBehaviorFlags m_soFlags{ 0 }; ///< Whether or not the scene object is enabled
    Scene* m_scene{ nullptr }; ///< A pointer to the scene containing this object
    BoundingBoxes m_worldBounds; ///< The world-space bounds of the scene object

    /// @details Made mutable to access via renderLayers routine
    std::vector<uint32_t> m_renderLayers; ///< Rendering layer IDs
    
    TransformComponent m_transformComponent; ///< The transform for this scene object

    /// @details A scene object may store one component of each type
    /// @todo Move the components into the scene, and store in sorted vectors
    std::vector<Component*> m_components; ///< The components attached to this scene object. 

    std::vector<std::shared_ptr<DrawCommand>> m_drawCommands; ///< The draw commands for the scene object

    /// @}
};
typedef std::shared_ptr<SceneObject> SceneObjectPtr;


} // end namespacing
