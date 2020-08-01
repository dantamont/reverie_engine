// TODO: Move this into widgets file, is unnecessary clutter
#ifndef GB_COMPONENT_MODELS_H 
#define GB_COMPONENT_MODELS_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt
#include <QAction>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// Project
#include "../../core/GbObject.h"
#include "../../view/tree/GbTreeWidget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class Component;
class CoreEngine;
class Scenario;
class Scene;
class SceneObject;
class UndoCommand;

namespace View {

class ComponentTreeWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ComponentItem : public TreeItem<Component> {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Enums and Static
    /// @{

    enum ComponentType {
        kTransform = 2000, // Tree widget item takes a type
        kShader,
        kCamera,
        kLight,
        kScript, 
        kStateMachine,
        kModel,
        kListener,
        kRigidBody,
        kPhysicsScene,
        kCanvas,
        kCharacterController,
        kBoneAnimation,
        kCubeMap,
        kSceneObjectSettings
    };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ComponentItem(Component* component);
    ComponentItem(SceneObject* sceneObject);
    ~ComponentItem();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Perform an action from this item
    void performAction(UndoCommand* command);

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    void setWidget() override;
    void removeWidget(int column = 0) override;

    /// @brief Return component represented by this tree item
    inline Component* component() { return m_object; }

    /// @brief Get the component type of this tree item
    ComponentType componentType() const { return ComponentType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    View::ComponentTreeWidget* componentWidget() const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{
    virtual const char* className() const override { return "ComponentItem"; }
    virtual const char* namespaceName() const override { return "Gb::View:ComponentItem"; }

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class ComponentTreeWidget;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Set the component type
    ComponentType getComponentType(Component* component);

    /// @brief Initialize the component tree item
    void initializeItem();


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    SceneObject* m_sceneObject;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif // GB_SCENE_TREE_H 





