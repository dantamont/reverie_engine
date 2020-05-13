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
class ComponentItem : public QTreeWidgetItem, public Gb::Object {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Enums and Static
    /// @{

    enum ComponentType {
        kTransform = 2000, // Tree widget item takes a type
        kRenderer,
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
        kBoneAnimation
    };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ComponentItem(Component* component);
    ~ComponentItem();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Perform an action from this item
    void performAction(UndoCommand* command);

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    void setWidget();
    void removeWidget();

    /// @brief Return component represented by this tree item
    inline Component* component() { return m_component; }

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

    /// @brief Pointer to the model corresponding to this tree item
    Component* m_component;

    /// @brief Pointer to widget if this item has one
    QWidget* m_widget;

    /// @}
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif // GB_SCENE_TREE_H 





