// TODO: Move this into widget file, unnecessary clutter
#ifndef GB_SCENE_MODELS_H 
#define GB_SCENE_MODELS_H
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

class CoreEngine;
class Scenario;
class Scene;
class SceneObject;
class UndoCommand;

namespace View {

class SceneTreeWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SceneRelatedItem : public QTreeWidgetItem, public Gb::Object {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{

    enum SceneType {
        kScenario = 1000,
        kScene,
        kSceneObject,
        kComponent
    };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    SceneRelatedItem(std::shared_ptr<Object> object, SceneRelatedItem* parent, SceneType type = kScenario);
    ~SceneRelatedItem();
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Handle reparenting under a new item
    void reparent(SceneRelatedItem* newParent);

    /// @brief Perform an action from this item
    void performAction(UndoCommand* command);

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    void setWidget();
    void removeWidget();

    /// @brief Return object represented by this tree item
    inline std::shared_ptr<Object> object() {
        if (std::shared_ptr<Object> objectPtr = m_object.lock()) {
            return objectPtr;
        }
        else {
            throw("Error, no object associated with this scene item");
            return nullptr;
        }
    }

    /// @brief Get the scene-related type of this tree item
    SceneType sceneType() const { return SceneType(type()); }

    /// @brief Convenience method for retrieving casted tree widget
    View::SceneTreeWidget* sceneTreeWidget() const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{
    virtual const char* className() const override { return "SceneRelatedItem"; }
    virtual const char* namespaceName() const override { return "Gb::View:SceneRelatedItem"; }

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class SceneTreeWidget;
    friend class ChangeNameCommand;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Initialize the scene tree item
    void initializeItem();

    /// @brief Set the text on this item from the data
    void refreshText();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Shared pointer to the model corresponding to this tree item
    std::weak_ptr<Object> m_object;

    /// @brief Pointer to widget if this item has one
    QWidget* m_widget;

    /// @}
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif // GB_SCENE_TREE_H 





