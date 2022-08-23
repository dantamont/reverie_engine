#pragma once

// Standard Includes

// Qt
#include <QAction>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// Project
#include "geppetto/qt/widgets/tree/GTreeWidget.h"

namespace rev {

class UndoCommand;
class ComponentTreeWidget;

/// @todo Maybe need to call deleteLater on item widgets
/// @see https://stackoverflow.com/questions/58269530/do-you-need-to-delete-widget-after-removeitemwidget-from-qtreewidget
class ComponentItem : public TreeItem {
public:
    /// @name Enums and Static
    /// @{

    /// @todo Eliminate this and have a single ComponentType enum
    enum ComponentItemType {
        kInvalid = -1,
        kTransform = 2000, // Tree widget item takes a type
        kShader,
        kCamera,
        kLight,
        kScript, 
        kStateMachine,
        kModel,
        kListener,
        kRigidBody,
        kUnused0,
        kCanvas,
        kCharacterController,
        kBoneAnimation,
        kCubeMap,
        kAudioSource,
        kAudioListener,
        kNUM_COMPONENT_TYPES,
        kPhysicsScene,
        kSceneObjectSettings,
        kBlueprint,
        kCOUNT
    };

    /// @}

    /// @name Constructors and Destructors
    /// @{

    /// @brief Constructor
    /// @param[in] type denote whether is a scenario, scene, or scene object
    /// @param[in] j JSON representing the object represented by this item
    /// @param[in] id the ID of the object represented by this item. Unused for now
    /// @todo Use ID once components no longer use UUID
    ComponentItem(ComponentItemType type, const json& j, Uint32_t id = 0);
    ~ComponentItem();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    void setWidget() override;
    void removeWidget(int column = 0) override;

    /// @brief Get the component type of this tree item
    ComponentItemType componentType() const { return ComponentItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    ComponentTreeWidget* componentWidget() const;

    /// @}

protected:
 
    friend class ComponentTreeWidget;

    /// @brief Initialize the component tree item
    void initializeItem();

    QWidget* m_componentWidget{ nullptr }; ///< The component widget that is wrapped in the treeWidgetKeeper wrapper
    Int32_t m_sceneObjectId{ -1 };
};



/// @class ComponentBlueprintItem
class ComponentBlueprintItem : public TreeItem {
public:

    /// @name Constructors and Destructors
    /// @{
    ComponentBlueprintItem(const json& blueprintJson);
    ~ComponentBlueprintItem();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    void setWidget() override;
    void removeWidget(int column = 0) override;

    /// @brief Convenience method for retrieving casted inspector widget
    ComponentTreeWidget* componentWidget() const;

    /// @}

protected:

    friend class ComponentTreeWidget;

    /// @brief Initialize the component tree item
    void initializeItem();

};


} // rev





