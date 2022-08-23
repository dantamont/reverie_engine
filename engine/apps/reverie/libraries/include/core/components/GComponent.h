#pragma once

// Qt
#include <QMetaType>

// External
#include "fortress/types/GLoadable.h"
#include "fortress/types/GIdentifiable.h"
#include "fortress/containers/extern/tsl/robin_map.h"

// Project
#include "core/scene/GSceneBehaviorFlags.h"

namespace rev {

class SceneObject;
class Scene;
class CoreEngine;

enum class ComponentType {
    kNone = -1,
    kTransform,
    kShader,
    kCamera,
    kLight,
    kPythonScript,
    kStateMachine,
    kModel,
    kListener,
    kRigidBody,
    kTEMP_BUFFER,
    kCanvas,
    kCharacterController,
    kBoneAnimation,
    kCubeMap,
    kAudioSource,
    kAudioListener,
    MAX_COMPONENT_TYPE_VALUE,
    kPhysicsScene = 0,
    MAX_SCENE_COMPONENT_TYPE_VALUE
};

// TODO: Implement
enum class ComponentFlag {
    kIsScene = 1 << 0, // If true, this is a scene component, not a scene object component
    kIsEnabled = 1 << 1, // If true, component is enabled
    kLocked = 1 << 2 // If locked, component cannot be modified by the user
};

/** @class Component
    @brief  A component to be attached to a Scene or SceneObject
*/
class Component: public IdentifiableInterface{
public:
    /// @name Static
    /// @{

    enum ParentType {
        kScene,
        kSceneObject
    };

    /// @brief Create a scene component from its type
    /// @todo have a separate class for scene components
    static Component* Create(Scene& scene, const json& json);

    /// @brief Creat a component from its type
    static Component* create(const std::shared_ptr<SceneObject>& object, ComponentType type);
    static Component* create(const std::shared_ptr<SceneObject>& object, const json& json);

    /// @}

    /// @name Constructors/Destructor
    /// @{
    Component(ComponentType type, bool isScene = false);
    Component(CoreEngine* engine, ComponentType type, bool isScene = false);
    Component(const Component& other);
    Component(const std::shared_ptr<SceneObject>& object, ComponentType type);
    Component(Scene* object, ComponentType type);
    virtual ~Component();
    
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return as the specified type
    template<typename T>
    T* as() {
        static_assert(std::is_base_of_v<Component, T>, "Error, T must be a component type");
        return static_cast<T*>(this);
    }

    /// @brief Return the JSON representation of the component, correctly casting for its subclass
    json toJson() const;

    /// @brief Add any required components for this component to be added to scene or object
    virtual void addRequiredComponents(std::vector<Uuid>& outDependencyIds, std::vector<json>& outDependencies) {}

    /// @brief Toggle this component
    void toggle(int enable);

    /// @brief Whether this component is enabled or not
    bool isEnabled() const {
        return m_componentFlags.testFlag(SceneBehaviorFlag::kEnabled);
    }

    /// @brief Enable this component
    virtual void enable(){
        m_componentFlags.setFlag(SceneBehaviorFlag::kEnabled, true);
    }

    /// @brief Disable this component
    virtual void disable(){
        m_componentFlags.setFlag(SceneBehaviorFlag::kEnabled, false);
    }

    /// @brief What to do prior to destruction of the component
    virtual void preDestruction(CoreEngine*) {}

    /// @}

    /// @name Properties
    /// @{

    inline bool isScriptGenerated() const {
        return m_componentFlags.testFlag(SceneBehaviorFlag::kScriptGenerated);
    }
    inline void setScriptGenerated(bool val) {
        m_componentFlags.setFlag(SceneBehaviorFlag::kScriptGenerated, val);
    }

    bool isSceneComponent() const { return m_isSceneComponent; }

    /// @brief Get type of component
    ComponentType getComponentType() const { return m_type; }

    /// @brief Get parent type of the component
    ParentType parentType() const {
        if (sceneObject())
            return kSceneObject;
        else
            return kScene;
    }

    /// @brief Obtain pointer to scene object
    std::shared_ptr<SceneObject> sceneObject() const;

    /// @brief Obtain pointer to scene
    Scene* scene() const {
        return m_scene;
    }

    /// @brief Set the scene object
    void setSceneObject(const std::shared_ptr<SceneObject>& object);

    /// @brief Set the scene object
    void setScene(Scene* object);

    /// @brief Max number of allowed components per scene object
    /// @details If left at -1, there is no limit
    virtual int maxAllowed() const { return -1; }

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Component& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Component& orObject);


    /// @}

protected:
    friend class SceneObject;

    /// @name Protected Members
    /// @{

    bool m_isSceneComponent; ///< If component is to be attached to a scene rather than a scene object
    SceneBehaviorFlags m_componentFlags; ///< Whether component is enabled or disabled
    ComponentType m_type; ///< Type of component

    /// @todo Subclass into scene object and scene components, instead of holding both of these
    std::weak_ptr<SceneObject> m_sceneObject; ///< Weak pointer to scene object associated with this component, if there is one
    Scene* m_scene; ///< Weak pointer to the scene associated with this component

    /// @}


};


} // end namespacing
