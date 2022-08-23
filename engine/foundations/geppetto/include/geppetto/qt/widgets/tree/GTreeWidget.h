#pragma once

// Standard Includes
#include <vector>

// Qt
#include <QtWidgets>

// Project
#include "fortress/json/GJson.h"
#include "fortress/types/GLoadable.h"
#include "fortress/types/GNameable.h"
#include "geppetto/qt/widgets/data/GWidgetData.h"


namespace rev {

class ActionManager;
class UndoCommand;
class WidgetManager;

class TreeItem : public QTreeWidgetItem {
public:

    /// @brief Need to construct statically for initialization to work properly
    template<typename ItemType, typename ...Args>
    static TreeItem* Create(const Args&... args) {
        static_assert(std::is_base_of_v<TreeItem, ItemType>, "Item Type must be a subclass of TreeItem");
        TreeItem* item = ItemType(args...);
        item->initializeItem();
        return item;
    }

    virtual ~TreeItem() {
    }

    /// @name Public Methods
    /// @{

    /// @brief Return pointer to the item's widget, may be null
    QWidget* widget() { return m_widget; }


    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    virtual void setWidget() {}
    virtual void removeWidget(int column = 0) {
        treeWidget()->removeItemWidget(this, column);
        m_widget = nullptr;
    }

    /// @brief Return JSON object represented by this tree item
    inline ItemizedWidgetData& data() { return m_data; }
    inline const ItemizedWidgetData& data() const { return m_data; }

    /// @}

protected:

    /// @brief Initialize the component tree item
    virtual void initializeItem() {
        // Set default size of item
        //setSizeHint(0, QSize(200, 400));

        // Set flags to be drag and drop enabled
        setFlags(flags() | (Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled));
    }

    ItemizedWidgetData m_data; ///< The data contained by this tree item
    QWidget* m_widget{ nullptr }; ///< Pointer to widget if this item has one

protected:

    /// @brief Tree item constructor
    /// @param[in] id the unique ID of the item
    /// @param[in] type the unique type of the item
    explicit TreeItem(Uint32_t id, Int32_t type) :
        QTreeWidgetItem(type),
        m_data(id)
    {
    }

    /// @brief Tree item constructor
    /// @param[in] id the unique ID of the item
    /// @param[in] type the unique type of the item
    /// @param[in] data the JSON used to initialize the tree item
    explicit TreeItem(Uint32_t id, Int32_t type, const json& data) :
        QTreeWidgetItem(type),
        m_data(id, data)
    {
    }
};


/// @class TreeWidgetKeeperWrapper
/// @brief Ensures that an item's embedded widget is not deleted when its item is deleted,
///    which is what happens when Qt reeparents widgets
class TreeWidgetKeeperWrapper : public QWidget {
    Q_OBJECT
public:
    TreeWidgetKeeperWrapper(QWidget* child);
    ~TreeWidgetKeeperWrapper();

    QWidget* child();

private:
    QWidget* m_child{ nullptr };

};

/// @class TreeWidget
/// @details Contains helper methods to make setting some callbacks easier, but also fixes how reparenting items
/// breaks widgets in default QTreeWidget implementation:
/// See: https://stackoverflow.com/questions/25559221/qtreewidgetitem-issue-items-set-using-setwidgetitem-are-dispearring-after-movin
class TreeWidget : public QTreeWidget, public rev::NameableInterface {
    Q_OBJECT
public:

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

    /// @name Constructors and Destructors
    /// @{
    TreeWidget(WidgetManager* widgetManager, ActionManager* actionManager, const QString& name, QWidget* parent = nullptr, uint32_t numColumns = 1);
    ~TreeWidget();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Add component item to the widget
    virtual void addItem(QTreeWidgetItem* item);

    /// @brief Remove component item from the widget
    virtual void removeItem(QTreeWidgetItem* item);

    ///// @brief Get tree item corresponding to the given component
    ///// @todo Use JSON for this
    //template<typename T>
    //QTreeWidgetItem* getItem(const T& object) {
    //    QTreeWidgetItemIterator it(this);
    //    while (*it) {
    //        TreeItem<T>* item = static_cast<TreeItem<T>*>(*it);
    //        if constexpr (std::is_base_of_v<IdentifiableInterface, T>) {
    //            // Check Uuid if available
    //            if (dynamic_cast<IdentifiableInterface*>(item->object())->getUuid() == dynamic_cast<const IdentifiableInterface*>(&object)->getUuid()) {
    //                return item;
    //            }
    //        }
    //        else {
    //            /// @todo Check if this is actually bueno
    //            // Otherwise, check object address
    //            if (item->object() == &object) {
    //                return item;
    //            }
    //        }
    //        ++it;
    //    }

    //    Logger::Throw("Error, no item found that corresponds to the given object");

    //    return nullptr;
    //}

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
            m_actionManager,
            [this] {m_actionManager->performAction(
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
        QTreeWidget::setItemWidget(item, column, new TreeWidgetKeeperWrapper(widget));
        item->setData(column, Qt::UserRole, m_allWidgets.count());
        m_allWidgets << widget;
    }

    /// @}

signals:

    void itemDeselected(QTreeWidgetItem* item);

protected slots:

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

    WidgetManager* m_widgetManager{ nullptr }; ///< For managing widgets globally
    ActionManager* m_actionManager{ nullptr }; ///< For managing undo/redo commands
    QWidgetList m_allWidgets; ///< For managing item widgets so that they aren't lost when reparenting
    std::vector<std::vector<QAction*>> m_actions; ///< Actions performable in this widget, indexed by action category
    std::vector<QTreeWidgetItem*> m_currentItems; ///< The items last operated on
    Uint32_t m_numColumns{ 0 }; ///< The number of columns in the tree widget

};


} // rev
