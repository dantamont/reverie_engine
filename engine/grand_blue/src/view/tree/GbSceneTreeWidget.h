#ifndef GB_SCENE_TREE_H 
#define GB_SCENE_TREE_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt
#include <QAction>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// Project
#include "../../core/service/GbService.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class CoreEngine;
class Scenario;
class Scene;
class SceneObject;

namespace View {

class SceneRelatedItem;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class SceenTreeWidget
// TODO: Implement drag and drop: https://www.qtcentre.org/threads/39783-QTreeWidget-Drag-and-Drop
class SceneTreeWidget : public QTreeWidget, public AbstractService {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{

    static unsigned int s_numColumns;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    SceneTreeWidget(CoreEngine* core, const QString& name, QWidget* parent = nullptr);
    ~SceneTreeWidget();
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Return core engine
    CoreEngine* engine() const { return m_engine; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Get current scene 
    std::shared_ptr<Scene> getCurrentScene();

    /// @brief Get current scene object
    std::shared_ptr<SceneObject> getCurrentSceneObject();

    /// @brief Set the scenario tree item
    void setScenarioTreeItem();
    void setScenarioTreeItem(View::SceneRelatedItem* scenario);

    /// @brief Clear the header item
    void removeScenarioTreeItem();

    /// @brief Add/remove a scene tree item
    void addSceneTreeItem(std::shared_ptr<Scene> scene);
    void addSceneTreeItem(View::SceneRelatedItem* scene);
    void removeTreeItem(std::shared_ptr<Scene> scene);

    /// @brief Add/remove a scene object tree item
    void addSceneObjectTreeItem(const std::shared_ptr<SceneObject>& sceneObject);
    void removeTreeItem(const std::shared_ptr<SceneObject>& sceneObject);

    /// @brief Get tree item corresponding to the given object
    View::SceneRelatedItem* getItem(std::shared_ptr<Gb::Object> object);

    /// @brief Resize columns to fit content
    void resizeColumns();

    /// @brief Clear selected objects
    void clearSelectedItems();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{
    virtual const char* className() const override { return "SceneTreeWidget"; }
    virtual const char* namespaceName() const override { return "Gb::View::SceneTreeWidget"; }

    /// @brief Returns True if this AbstractService represents a service
    /// @details This is not a service
    virtual bool isService() const override { return false; };

    /// @brief Returns True if this AbstractService represents a tool.
    /// @details This is not a tool
    virtual bool isTool() const override{ return false; };

    /// @}

signals:
    /// @brief Emit on selection of a scene 
    void selectedScene(const Uuid& sceneID);

    /// @brief Emit on selection of a scene object
    void selectedSceneObject(const Uuid& sceneObjectID);

    /// @brief Emit on deselection of any scene objects
    void deselectedSceneObject();

    /// @brief Emit on deselection of any scenes 
    void deselectedScene();

protected slots:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Slots
    /// @{

    /// @brief What to do on item double click
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

    /// @brief What to do on item clicked
    void onItemClicked(QTreeWidgetItem *item, int column);

    /// @brief What to do on item expanded
    void onItemExpanded(QTreeWidgetItem* item);

    /// @brief What to do on current item change
    //void onCurrentItemChanged(QTreeWidgetItem *item, QTreeWidgetItem *previous);

    /// @}
protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class SceneRelatedItem;

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Override default mouse release event
    void mouseReleaseEvent(QMouseEvent *event) override;

    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

    /// @brief Remove an item
    void removeItem(View::SceneRelatedItem* sceneItem);

    /// @brief Initialize the tree widget
    void initializeTreeWidget();

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
    QAction* m_addScenario;
    QAction* m_addScene;
    QAction* m_removeScene;
    QAction* m_addSceneObject;
    QAction* m_removeSceneObject;

    /// @brief The scene item clicked by a right-mouse operation
    SceneRelatedItem* m_currentSceneItem;
    SceneRelatedItem* m_currentSceneObjectItem;

    SceneRelatedItem* m_lastLeftClickedItem;

    /// @brief Core engine for the application
    CoreEngine* m_engine;

    /// @}
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif // GB_SCENE_TREE_H 





