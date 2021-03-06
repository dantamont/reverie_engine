#ifndef GB_TREE_WIDGET_H 
#define GB_TREE_WIDGET_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes
#include <vector>

// External

// Project
#include "../../core/service/GService.h"
#include "../../core/mixins/GLoadable.h"
#include "../../model_control/commands/GActionManager.h"
#include "../../core/GCoreEngine.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class UndoCommand;

namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
class TreeItem : public QTreeWidgetItem, public rev::Object {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Enums and Static
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    TreeItem(T* object, int type = 2000) :
        QTreeWidgetItem(type),
        m_object(object),
        m_widget(nullptr)
    {
        initializeItem();
    }
    virtual ~TreeItem() {
    }

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    virtual void setWidget() {}
    virtual void removeWidget(int column = 0) {
        treeWidget()->removeItemWidget(this, column);
        m_widget = nullptr;
    }

    /// @brief Return component represented by this tree item
    inline T* object() { return m_object; }
    inline Nameable* nameable() { return dynamic_cast<Nameable*>(m_object); }
    inline Identifiable* identifier() { return dynamic_cast<Identifiable*>(m_object); }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{
    virtual const char* className() const override { return "TreeItem"; }
    virtual const char* namespaceName() const override { return "rev::View:TreeItem"; }

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Initialize the component tree item
    virtual void initializeItem() {
        // Set default size of item
        //setSizeHint(0, QSize(200, 400));

        // Set flags to be drag and drop enabled
        setFlags(flags() | (Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled));
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Pointer to the model corresponding to this tree item
    T* m_object;

    /// @brief Pointer to widget if this item has one
    QWidget* m_widget;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Ensures that an item's embedded widget is not deleted when its item is deleted,
/// which is what happens when Qt reeparents widgets
class Tree_widget_keeper_wrapper : public QWidget {
    Q_OBJECT
public:
    Tree_widget_keeper_wrapper(QWidget* child) {
        _child = child;
        QVBoxLayout* layout1 = new QVBoxLayout(this);
        layout1->setContentsMargins(0, 0, 0, 0);
        layout1->addWidget(_child);
    }

    ~Tree_widget_keeper_wrapper() {
        if (_child->parent() == this) {
            _child->hide();
            _child->setParent(0);
        }
    }

private:
    QWidget* _child;

};

/// @class TreeWidget
/// @details Contains helper methods to make setting some callbacks easier, but also fixes how reparenting items
/// breaks widgets in default QTreeWidget implementation:
/// See: https://stackoverflow.com/questions/25559221/qtreewidgetitem-issue-items-set-using-setwidgetitem-are-dispearring-after-movin
class TreeWidget : public QTreeWidget, public AbstractService {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    
    enum ActionCategory {
        kItemSelected,
        kNoItemSelected,
        kMAX_NUM_CATEGORIES
    };

    enum InteractionType {
        kDoubleClick,
        kLeftClick,
        kContextClick, // Used to generate context menu
        kExpanded,
        kDrag,
        kDrop,
        kDeselected,
        kMAX_INTERACTION_TYPE
    };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    TreeWidget(CoreEngine* engine, const QString& name, QWidget* parent = nullptr, size_t numColumns = 1);
    ~TreeWidget();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Add component item to the widget
    virtual void addItem(QTreeWidgetItem* item);

    /// @brief Remove component item from the widget
    virtual void removeItem(const Object& object);
    virtual void removeItem(QTreeWidgetItem* item);

    /// @brief Get tree item corresponding to the given component
    template<typename T>
    QTreeWidgetItem* getItem(const T& object) {
        QTreeWidgetItemIterator it(this);
        while (*it) {
            TreeItem<T>* item = static_cast<TreeItem<T>*>(*it);
            if (dynamic_cast<Identifiable*>(item->object())->getUuid() == dynamic_cast<const Identifiable*>(&object)->getUuid()) {
                return item;
            }
            ++it;
        }

        throw("Error, no item found that corresponds to the given object");

        return nullptr;
    }

    /// @brief Resize columns to fit content
    virtual void resizeColumns();

    /// @brief Create an action from the given command
    template<typename T, typename ...args>
    void addAction(const args& ... values, ActionCategory category) {
        T* tempCommand = new T(args...);
        QAction* action = new QAction(tempCommand->text(), this);
        action->setStatusTip(tempCommand->description());
        connect(action,
            &QAction::triggered,
            m_engine->actionManager(),
            [this] {m_engine->actionManager()->performAction(
                new T(args...)
            );
        });
        m_actions[category].push_back(action);
        delete tempCommand;
    }

    /// @brief Create an action given a lambda functon
    template<typename T, typename ...args>
    void addAction(ActionCategory category, 
        QString&& name,
        QString&& description,
        T&& onAction) {
        QAction* action = new QAction(name, this);
        action->setStatusTip(description);
        connect(action,
            &QAction::triggered,
            this,
            onAction);
        m_actions[category].push_back(action);
    }

    /// @brief Reimplementing this to attach information to tree item widgets for when moved
    /// See: https://stackoverflow.com/questions/25559221/qtreewidgetitem-issue-items-set-using-setwidgetitem-are-dispearring-after-movin
    void setItemWidget(QTreeWidgetItem* item, int column, QWidget* widget) {
        QTreeWidget::setItemWidget(item, column, new Tree_widget_keeper_wrapper(widget));
        item->setData(column, Qt::UserRole, m_allWidgets.count());
        m_allWidgets << widget;
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{
    virtual const char* className() const override { return "TreeWidget"; }
    virtual const char* namespaceName() const override { return "rev::View::TreeWidget"; }

    /// @brief Returns True if this AbstractService represents a service
    /// @details This is not a service
    virtual bool isService() const override { return false; };

    /// @brief Returns True if this AbstractService represents a tool.
    /// @details This is not a tool
    virtual bool isTool() const override { return false; };

    /// @}
signals:

    void itemDeselected(QTreeWidgetItem* item);

protected slots:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Slots
    /// @{

    void onRowsInserted(QModelIndex parent, int start, int end);

    /// @brief For object scelection
    virtual void onItemClicked(QTreeWidgetItem *item, int column);

    /// @brief What to do on item double click
    virtual void onItemDoubleClicked(QTreeWidgetItem *item, int column);

    /// @brief What to do on item expanded
    virtual void onItemExpanded(QTreeWidgetItem* item);

    /// @brief What to do on item deselected
    virtual void onItemDeselected(QTreeWidgetItem* item);

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{


    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief initialize an item added to the widget
    virtual void initializeItem(QTreeWidgetItem* item);

    /// @brief Override default mouse release event
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

    virtual void dropEvent(QDropEvent* event) override;
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;

    /// @brief Drop event callbacks
    virtual void onDropAbove(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination);
    virtual void onDropBelow(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination);
    virtual void onDropOn(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination);
    virtual void onDropViewport(QDropEvent* event, QTreeWidgetItem* source);
    virtual void onDropFromOtherWidget(QDropEvent* event, QTreeWidgetItem* source, QWidget* otherWidget);

    /// @brief Initialize the widget
    virtual void initializeWidget();

    /// @brief Initialize as a list widget
    void initializeAsList();
    void enableDragAndDrop();

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

    /// @brief For managing item widgets so that they aren't lost when reparenting
    QWidgetList m_allWidgets;

    /// @brief Actions performable in this widget, indexed by action category
    std::vector<std::vector<QAction*>> m_actions;

    /// @brief The items last operated on
    std::vector<QTreeWidgetItem*> m_currentItems;

    /// @brief Core engine for the application
    CoreEngine* m_engine;

    size_t m_numColumns;

    /// @}
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // rev

#endif





