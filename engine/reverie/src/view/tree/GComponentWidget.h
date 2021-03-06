#ifndef GB_COMPONENT_WIDGET_H 
#define GB_COMPONENT_WIDGET_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External

// Project
#include "../../core/service/GService.h"
#include <core/scene/GBlueprint.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

class CoreEngine;
class Component;
class SceneObject;
class Scene;
//class Blueprint;

namespace View {

class ComponentItem;
class ComponentBlueprintItem;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ComponentTreeWidget : public QTreeWidget, public AbstractService {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ComponentTreeWidget(CoreEngine* engine, const QString& name, QWidget* parent = nullptr);
    ~ComponentTreeWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Add component item to the widget
    void addItem(Component* component);
    void addItem(View::ComponentItem* item);
    void addItem(View::ComponentBlueprintItem* item);

    /// @brief Remove component item from the widget
    void removeItem(Component* component);

    /// @brief Get tree item corresponding to the given component
    View::ComponentItem* getItem(Component* component);

    /// @brief Resize columns to fit content
    void resizeColumns();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{
    virtual const char* className() const override { return "ComponentTreeWidget"; }
    virtual const char* namespaceName() const override { return "rev::View::ComponentTreeWidget"; }

    /// @brief Returns True if this AbstractService represents a service
    /// @details This is not a service
    virtual bool isService() const override { return false; };

    /// @brief Returns True if this AbstractService represents a tool.
    /// @details This is not a tool
    virtual bool isTool() const override { return false; };

    /// @}

protected slots:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Slots
    /// @{

    /// @brief What to do on new scene selection
    void selectScene(const Uuid& sceneID);
    
    /// @brief Clear scene and components
    void clearScene();

    /// @brief What to do on new scene object selection
    void selectSceneObject(size_t sceneObjectID);

    /// @brief What to do on new scene object selection
    void onSelectedBlueprint(QModelIndex blueprintIndex);

    /// @brief Clear scene object and components
    void clearSceneObject();

    /// @brief What to do on item double click
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

    /// @brief What to do on item expanded
    void onItemExpanded(QTreeWidgetItem* item);

    /// @brief What to do on current item change
    //void onCurrentItemChanged(QTreeWidgetItem *item, QTreeWidgetItem *previous);

    /// @}
protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class ComponentItem;
    friend class ComponentBlueprintItem;

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    //void clearBlueprint();

    std::shared_ptr<SceneObject> currentSceneObject() const {
        if (const std::shared_ptr<SceneObject>& so = m_currentSceneObject.lock()) {
            return so;
        }
        else {
            return nullptr;
        }
    }

    /// @brief Override default mouse release event
    void mouseReleaseEvent(QMouseEvent *event) override;

    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

    /// @brief Remove an item
    void removeItem(View::ComponentItem* componentItem);

    /// @brief Initialize the widget
    void initializeWidget();

#ifndef QT_NO_CONTEXTMENU
    /// @brief Generates a context menu, overriding default implementation
    /// @note Context menus can be executed either asynchronously using the popup() function or 
    ///       synchronously using the exec() function
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Actions performable in this widget
    QAction* m_addShaderComponent;
    QAction* m_addCameraComponent;
    QAction* m_addLightComponent;
    QAction* m_addScriptComponent;
    QAction* m_addModelComponent;
    QAction* m_addAnimationComponent;
    QAction* m_addListenerComponent;
    QAction* m_addRigidBodyComponent;
    QAction* m_addCanvasComponent;
    QAction* m_addCharControllerComponent;
    QAction* m_addCubeMapComponent;
    QAction* m_addAudioSourceComponent;
    QAction* m_addAudioListenerComponent;

    /// @brief Actions to add scene components
    QAction* m_addPhysicsSceneComponent;

    QAction* m_deleteComponent;

    /// @brief The scene object for which components are currently being displayed
    std::weak_ptr<SceneObject> m_currentSceneObject;

    /// @brief The scene for which components are currently being displayed
    Scene* m_currentScene;

    /// @brief The component clicked by a right-mouse operation
    ComponentItem* m_currentComponentItem;

    /// @brief Scene object just for rendering components for blueprints
    //std::shared_ptr<SceneObject> m_blueprintSceneObject;
    Blueprint m_selectedBlueprint;

    /// @brief Core engine for the application
    CoreEngine* m_engine;

    /// @}
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // rev

#endif





