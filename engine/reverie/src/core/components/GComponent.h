#ifndef GB_COMPONENT_H
#define GB_COMPONENT_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External
#include <QMetaType>

// Project
#include "../GObject.h"
#include "../mixins/GLoadable.h"
#include <core/scene/GSceneCommon.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SceneObject;
class Scene;
class CoreEngine;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
class Component: public Object, public Serializable, public Identifiable{
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum ParentType {
        kScene,
        kSceneObject
    };

    /// @brief The required component types to create this component, and those incompatible
    struct Constraints {
        bool isEmpty() const { return m_constraints.size() == 0; }
        tsl::robin_map<ComponentType, bool> m_constraints; // TODO: Make this a vector
    };

    static Constraints GetRequirements(ComponentType type);

    /// @brief Creat a component from its type
    static Component* create(const std::shared_ptr<SceneObject>& object, ComponentType type);
    static Component* create(const std::shared_ptr<SceneObject>& object, const QJsonObject& json);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Component(ComponentType type, bool isScene = false);
    Component(CoreEngine* engine, ComponentType type, bool isScene = false);
    Component(const Component& other);
    Component(const std::shared_ptr<SceneObject>& object, ComponentType type);
    Component(Scene* object, ComponentType type);
    virtual ~Component();
    
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Return as the specified type
    template<typename T>
    T* as() {
        static_assert(std::is_base_of_v<Component, T>, "Error, T must be a component type");
        return static_cast<T*>(this);
    }

    /// @brief Add any required components for this component to be added to scene or object
    virtual void addRequiredComponents() {}

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

    //-----------------------------------------------------------------------------------------------------------------
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

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "Component"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::Component"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    friend class SceneObject;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Map of component requirements, indexed by type
    static tsl::robin_map<ComponentType, Constraints> TypeConstraints;

    /// @brief If component is to be attached to a scene rather than a scene object
    bool m_isSceneComponent;

    /// @brief Whether component is enabled or disabled
    SceneBehaviorFlags m_componentFlags;

    /// @brief Type of component
    ComponentType m_type;

    // TODO: Subclass into scene object and scene components, instead of holding both of these
    /// @brief Weak pointer to scene object associated with this component, if there is one
    std::weak_ptr<SceneObject> m_sceneObject;

    /// @brief Weak pointer to the scene associated with this component
    Scene* m_scene;

    /// @}


};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
