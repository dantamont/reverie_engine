#include "GTreeWidget.h"

#include "../../model_control/commands/GActionManager.h"
#include "../../core/GCoreEngine.h"
#include "../../core/scene/GScenario.h"
#include "../../model_control/commands/GUndoCommand.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TreeWidget::TreeWidget(CoreEngine* engine, const QString & name, QWidget * parent, size_t numColumns) :
    AbstractService(name),
    QTreeWidget(parent),
    m_engine(engine),
    m_numColumns(numColumns)
{
    m_currentItems.resize(kMAX_INTERACTION_TYPE);
    m_actions.resize(kMAX_NUM_CATEGORIES);
    // Need to call initializeWidget after construction

    // When rows added, make sure item widget is added back to any moved widgets
    connect(model(), &QAbstractItemModel::rowsInserted, this, &TreeWidget::onRowsInserted);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TreeWidget::~TreeWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::onItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    m_currentItems[kLeftClick] = item;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    m_currentItems[kDoubleClick] = item;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::onItemExpanded(QTreeWidgetItem * item)
{
    m_currentItems[kExpanded] = item;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::onItemDeselected(QTreeWidgetItem * item)
{
    m_currentItems[kDeselected] = item;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::addItem(QTreeWidgetItem * item)
{
    // Insert component item into the widget
    addTopLevelItem(item);

    // Create widget for component or other initialization
    initializeItem(item);

    // Resize columns to fit widget
    resizeColumns();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::removeItem(const Object& object)
{
    removeItem(getItem(object));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::removeItem(QTreeWidgetItem * item)
{
    delete item;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::resizeColumns()
{
    for (unsigned int i = 0; i < m_numColumns; i++) {
        resizeColumnToContents(i);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::initializeItem(QTreeWidgetItem * item)
{
    Q_UNUSED(item);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::onRowsInserted(QModelIndex parent, int start, int end) {
    for (int column = 0; column < model()->columnCount(parent); column++) {
        for (int row = start; row <= end; row++) {
            QModelIndex index = model()->index(row, column, parent);
            QVariant data = model()->data(index, Qt::UserRole);
            if (data.type() == QVariant::Int) {
                int i = data.toInt();
                QTreeWidgetItem* item = itemFromIndex(index);
                if (item && i >= 0 && i < m_allWidgets.count()) {
                    setItemWidget(item, column, m_allWidgets[i]);
                    m_allWidgets[i]->show();
                }

            }
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::mouseReleaseEvent(QMouseEvent * event)
{
    QTreeWidget::mouseReleaseEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::dropEvent(QDropEvent * event)
{
    QTreeWidget* widget = static_cast<QTreeWidget*>(event->source());
    QTreeWidgetItem* sourceItem = widget->currentItem();
    if (widget == this) {
        QModelIndex destIndex = indexAt(event->pos());
        if (destIndex.isValid()) {
            // Dropping onto an item
            DropIndicatorPosition dropIndicator = dropIndicatorPosition();
            QTreeWidgetItem* destItem;
            switch (dropIndicator) {
            case QAbstractItemView::AboveItem:
                // Dropping above an item
                destItem = itemFromIndex(destIndex);
                onDropAbove(event, sourceItem, destItem);
                break;
            case QAbstractItemView::BelowItem:
                // Dropping below an item
                destItem = itemFromIndex(destIndex);
                onDropBelow(event, sourceItem, destItem);
                break;
            case QAbstractItemView::OnItem:
            {
                // Dropping directly onto an item
                destItem = itemFromIndex(destIndex);
                onDropOn(event, sourceItem, destItem);
                break;
            }
            case QAbstractItemView::OnViewport:
                // Not on the tree (may not be triggered here)
                onDropViewport(event, sourceItem);
                break;
            }
        }
        else {
            // Not dropping onto any item
            onDropViewport(event, sourceItem);
        }
    }
    else {
        // Drops from another widget
        onDropFromOtherWidget(event, sourceItem, widget);
    }
    QTreeWidget::dropEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::dragEnterEvent(QDragEnterEvent * event)
{
    QTreeWidget::dragEnterEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::dragLeaveEvent(QDragLeaveEvent * event)
{
    QTreeWidget::dragLeaveEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::dragMoveEvent(QDragMoveEvent * event)
{
    QTreeWidget::dragMoveEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::onDropAbove(QDropEvent* event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    Q_UNUSED(source);
    Q_UNUSED(destination);
    event->setDropAction(Qt::IgnoreAction);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::onDropBelow(QDropEvent* event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    Q_UNUSED(source);
    Q_UNUSED(destination);
    event->setDropAction(Qt::IgnoreAction);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::onDropOn(QDropEvent* event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    Q_UNUSED(source);
    Q_UNUSED(destination);
    event->setDropAction(Qt::IgnoreAction);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::onDropViewport(QDropEvent* event, QTreeWidgetItem * source)
{
    Q_UNUSED(source);
    event->setDropAction(Qt::IgnoreAction);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::onDropFromOtherWidget(QDropEvent* event, QTreeWidgetItem * source, QWidget * otherWidget)
{
    Q_UNUSED(source);
    Q_UNUSED(otherWidget);
    event->setDropAction(Qt::IgnoreAction);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::initializeWidget()
{
    //setMinimumWidth(350);
    setColumnCount(m_numColumns);

    // Connect signal for click events
    connect(this, &TreeWidget::itemClicked,
        this, &TreeWidget::onItemClicked);

    // Connect signal for double click events
    connect(this, &TreeWidget::itemDoubleClicked,
        this, &TreeWidget::onItemDoubleClicked);

    // Connect signal for item expansion
    connect(this, &TreeWidget::itemExpanded,
        this, &TreeWidget::onItemExpanded);

    // Connect signal for deselecting item
    connect(this, &TreeWidget::currentItemChanged,
        this, [this](QTreeWidgetItem *current, QTreeWidgetItem *previous) {
        Q_UNUSED(current);
        if (previous) {
            emit itemDeselected(previous);
        }
    });
    connect(this, &TreeWidget::itemDeselected, this, &TreeWidget::onItemDeselected);


    // Connect signal for clearing on scenario load
    //connect(m_engine, &CoreEngine::scenarioChanged, this, &TreeWidget::clear);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::initializeAsList()
{    
    // Set tree widget settings
    setColumnCount(0);
    m_numColumns = 0;
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setHeaderLabels(QStringList({ "" }));
    setAlternatingRowColors(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::enableDragAndDrop()
{
    setDragEnabled(true);
    setDragDropMode(DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropOverwriteMode(false); // is default, but making explicit for reference
}

#ifndef QT_NO_CONTEXTMENU
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreeWidget::contextMenuEvent(QContextMenuEvent * event)
{
    m_currentItems[kContextClick] = itemAt(event->pos());
    // Create menu
    QMenu menu(this);

    // Add actions to the menu
    QTreeWidgetItem* item = itemAt(event->pos());
    if (item) {
        m_currentItems[kContextClick] = item;
        for (QAction* action : m_actions[kItemSelected]) {
            menu.addAction(action);
        }
    }
    else {
        m_currentItems[kContextClick] = nullptr;
        for (QAction* action : m_actions[kNoItemSelected]) {
            menu.addAction(action);
        }
    }

    // Display menu at click location
    menu.exec(event->globalPos());
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}// View
} // rev