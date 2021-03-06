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
#include <core/GObject.h>
#include <core/service/GService.h>
#include <view/tree/GTreeWidget.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

class CoreEngine;
class Scenario;
class Scene;
class SceneObject;

namespace View {

class SceneRelatedItem;
class SceneTreeWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class SceneRelatedItem
class SceneRelatedItem : public TreeItem<rev::Object> {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{

    enum SceneType {
        kScenario = 1000,
        kScene,
        kSceneObject
    };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{

    SceneRelatedItem(SceneObject* so);
    SceneRelatedItem(Scenario* scenario);
    SceneRelatedItem(Scene* scene);

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief My own mime data method for convenience
    QVariant mimeData(int role) const;

    /// @brief Handle reparenting under a new item
    void reparent(SceneRelatedItem* newParent, int index = -1);

    /// @brief Perform an action from this item
    void performAction(UndoCommand* command);

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    virtual void setWidget() override;

    /// @brief Get the scene-related type of this tree item
    SceneType sceneType() const { return SceneType(type()); }

    /// @brief Convenience method for retrieving casted tree widget
    View::SceneTreeWidget* sceneTreeWidget() const;

    /// @brief Set the text on this item from the data
    void refreshText();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{
    virtual const char* className() const override { return "SceneRelatedItem"; }
    virtual const char* namespaceName() const override { return "rev::View:SceneRelatedItem"; }

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
    virtual void initializeItem() override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @}
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class SceenTreeWidget
// TODO: Implement drag and drop: https://www.qtcentre.org/threads/39783-QTreeWidget-Drag-and-Drop
class SceneTreeWidget : public TreeWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
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
    Scene* getCurrentScene();

    /// @brief Get current scene object
    SceneObject* getCurrentSceneObject();

    /// @brief Set the scenario tree item
    void setScenarioTreeItem();
    void setScenarioTreeItem(View::SceneRelatedItem* scenario);

    /// @brief Clear the header item
    void removeScenarioTreeItem();

    /// @brief Add/remove a scene tree item
    void addSceneTreeItem(Scene* scene);
    void addSceneTreeItem(View::SceneRelatedItem* scene);
    void removeTreeItem(Scene* scene);

    /// @brief Add/remove a scene object tree item
    void addSceneObjectTreeItem(SceneObject* sceneObject, int index = -1);
    void removeTreeItem(SceneObject* sceneObject);

    /// @brief Get tree item corresponding to the given object
    View::SceneRelatedItem* getItem(rev::SceneObject* object);
    View::SceneRelatedItem* getItem(rev::Scene* object);
    View::SceneRelatedItem* getItem(rev::Scenario* object);

    /// @brief Clear selected objects
    void clearSelectedItems();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{
    virtual const char* className() const override { return "SceneTreeWidget"; }
    virtual const char* namespaceName() const override { return "rev::View::SceneTreeWidget"; }

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
    void selectedSceneObject(size_t sceneObjectID);

    /// @brief Emit on deselection of any scene objects
    void deselectedSceneObject();

    /// @brief Emit on deselection of any scenes 
    void deselectedScene();

protected slots:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Slots
    /// @{

    /// @brief What to do on item double click
    virtual void onItemDoubleClicked(QTreeWidgetItem *item, int column) override;

    /// @brief What to do on item clicked
    virtual void onItemClicked(QTreeWidgetItem *item, int column) override;

    /// @brief What to do on item expanded
    virtual void onItemExpanded(QTreeWidgetItem* item) override;

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

    virtual QMimeData* mimeData(const QList<QTreeWidgetItem *> items) const override;
    virtual QStringList mimeTypes() const override;

    /// @brief initialize an item added to the widget
    virtual void initializeItem(QTreeWidgetItem* item);

    virtual void onDropOn(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination) override;
    virtual void onDropBelow(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination) override;
    virtual void onDropAbove(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination) override;
    virtual void onDropViewport(QDropEvent* event, QTreeWidgetItem* source) override;
    virtual void onDropFromOtherWidget(QDropEvent* event, QTreeWidgetItem* source, QWidget* otherWidget);

    virtual bool dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action);

    /// @brief Remove an item
    void removeItem(View::SceneRelatedItem* sceneItem);

    /// @brief Initialize the tree widget
    virtual void initializeWidget();

#ifndef QT_NO_CONTEXTMENU
    /// @brief Generates a context menu, overriding default implementation
    /// @note Context menus can be executed either asynchronously using the popup() function or 
    ///       synchronously using the exec() function
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Actions performable in this widget
    QAction* m_addScenario;
    QAction* m_addSceneObject;
    QAction* m_copySceneObject;
    QAction* m_removeSceneObject;

    /// @brief The scene item clicked by a right-mouse operation
    SceneRelatedItem* m_currentSceneItem;
    SceneRelatedItem* m_currentSceneObjectItem;

    SceneRelatedItem* m_lastLeftClickedItem;

    /// @}
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // rev

#endif // GB_SCENE_TREE_H 





