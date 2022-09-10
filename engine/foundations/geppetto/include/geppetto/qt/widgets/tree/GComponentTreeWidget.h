#pragma once

// External
#include <QtWidgets>

// Project
#include "fortress/json/GJson.h"
#include "fortress/types/GNameable.h"
#include "fortress/numeric/GSizedTypes.h"
#include "fortress/encoding/uuid/GUuid.h"

#include "geppetto/qt/widgets/tree/GTreeWidget.h"

#include "enums/GSceneObjectComponentTypeEnum.h"
#include "enums/GSceneTypeEnum.h"
#include "ripple/network/gateway/GMessageGateway.h"
#include "ripple/network/messages/GGetScenarioJsonMessage.h"

namespace rev {

class WidgetManager;
class ComponentItem;
class ComponentBlueprintItem;

/// @class ComponentTreeWidget
/// @brief A tree widget representing components
/// @todo Convert to view
class ComponentTreeWidget : public QTreeWidget, public rev::NameableInterface {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{
    ComponentTreeWidget(WidgetManager* engine, const QString& name, QWidget* parent = nullptr);
    ~ComponentTreeWidget();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Add component item to the widget
    void addItem(GSceneType sceneType, const json& componentJson);
    void addItem(ComponentItem* item);
    void addItem(ComponentBlueprintItem* item);

    /// @brief Remove component item from the widget
    void removeItem(const Uuid& componentId);

    /// @brief Get tree item corresponding to the given component
    ComponentItem* getItem(const Uuid& componentId);

    /// @brief Resize columns to fit content
    void resizeColumns();

    /// @brief Send signal to trigger onSelectSceneObject
    void selectSceneObject(uint32_t sceneObjectID);

    /// @brief What to do on scene object selection
    void onSelectSceneObject(Uint32_t sceneObjectId);

    /// @brief Send signal to trigger onSelectScene
    void selectScene(const GString& name);

    /// @brief What to do on new scene selection
    void onSelectScene(const GString& name);

    /// @}

protected slots:
    
    /// @name Protected Slots
    /// @{

    /// @brief Clear scene and components
    void clearScene();

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
    /// @name Friends
    /// @{

    friend class ComponentItem;
    friend class ComponentBlueprintItem;

    /// @}
    
    /// @name Protected Methods
    /// @{

    /// @brief Clear widgets, caching them until the next clear so that messaging doesn't break
    void cachedClear();

    /// @brief Override default mouse release event
    void mouseReleaseEvent(QMouseEvent *event) override;

    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

    /// @brief Remove an item
    void removeItem(ComponentItem* componentItem);

    /// @brief Initialize the widget
    void initializeWidget();

#ifndef QT_NO_CONTEXTMENU
    /// @brief Generates a context menu, overriding default implementation
    /// @note Context menus can be executed either asynchronously using the popup() function or 
    ///       synchronously using the exec() function
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU
    /// @}

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

    Int32_t m_currentSceneObjectId{ -1 }; ///< The scene object for which components are currently being displayed
    
    GString m_currentSceneName; ///< The scene for which components are currently being displayed

    ComponentItem* m_currentComponentItem; ///< The component clicked by a right-mouse operation
    json m_selectedBlueprintJson; ///< JSON representing a selected blueprint
    WidgetManager* m_widgetManager; ///< Widget manager handling scenario

    GGetScenarioJsonMessage m_getScenarioJsonMessage; ///< Message to retrieve scenario JSON from the main application

    //std::array<ComponentItem*, ComponentItem::ComponentItemType::kCOUNT> m_componentWidgets; ///< Persistent component widgets, indexed by type

    std::vector<ComponentItem*> m_cachedItems; ///< Items cached until next clear so their deletion doesn't deallocate messages being sent. Will be unnecessary once all widgets are persistent.

    /// @}
};


} // rev

