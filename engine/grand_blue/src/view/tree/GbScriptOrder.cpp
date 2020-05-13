#include "GbScriptOrder.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/loop/GbSimLoop.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/containers/GbSortingLayer.h"
#include "../../core/processes/GbProcessManager.h"

#include "../GbWidgetManager.h"
#include "../../GbMainWindow.h"

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ScriptJsonWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptJsonWidget::ScriptJsonWidget(CoreEngine* core, SortingLayer* sortLayer, QWidget *parent) :
    ParameterWidget(core, parent),
    m_sortingLayer(sortLayer),
    m_engine(core)
{
    initializeWidgets();
    initializeConnections();
    layoutWidgets();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptJsonWidget::~ScriptJsonWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptJsonWidget::wheelEvent(QWheelEvent* event) {
    if (!event->pixelDelta().isNull()) {
        ParameterWidget::wheelEvent(event);
    }
    else {
        // If scrolling has reached top or bottom
        // Accept event and stop propagation if at bottom of scroll area
        event->accept();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptJsonWidget::initializeWidgets()
{
    m_typeLabel = new QLabel("SortingLayer");
    m_typeLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_textEdit = new QTextEdit();
    m_textEdit->setText(JsonReader::getJsonValueAsQString(m_sortingLayer->asJson(), true));
    m_confirmButton = new QPushButton();
    m_confirmButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptJsonWidget::initializeConnections()
{

    // Make component to recolor text on change
    connect(m_textEdit, &QTextEdit::textChanged, this, [this]() {
        // Block signals to avoid infinite loop of signal sends
        m_textEdit->blockSignals(true);

        // Get text and color if not valid JSON
        QString text = m_textEdit->toPlainText();
        QJsonDocument contents = JsonReader::getQStringAsJsonDocument(text);
        QPalette palette;

        // Additional check for if label is used already
        QJsonObject object = contents.object();
        QString label = object["label"].toString();
        bool hasLabel = m_engine->processManager()->hasSortingLayer(label)
            && m_engine->processManager()->sortingLayers().at(label)->getUuid() != m_sortingLayer->getUuid();

        if (contents.isNull() || hasLabel) {
            palette.setColor(QPalette::Text, QColor(255, 0, 0));
            m_textEdit->setPalette(palette);
        }
        else {
            palette.setColor(QPalette::Text, QColor(0, 30, 150));
            m_textEdit->setPalette(palette);
        }

        m_textEdit->blockSignals(false);

    });

    // Make connection to resize component 
    connect(m_confirmButton, &QPushButton::clicked, this, [this](bool checked) {
        //QProgressDialog progress("Reloading resource", "Close", 0, 1, 
        //    m_engine->widgetManager()->mainWindow());
        //progress.setWindowModality(Qt::WindowModal);
        Q_UNUSED(checked)

        // Get text and return if not valid JSON
        const QString& text = m_textEdit->toPlainText();
        QJsonDocument contents = JsonReader::getQStringAsJsonDocument(text);

        // Additional check for if label is used already
        QJsonObject object = contents.object();
        QString label = object["label"].toString();
        bool hasLabel = m_engine->processManager()->hasSortingLayer(label)
            && m_engine->processManager()->sortingLayers().at(label)->getUuid() != m_sortingLayer->getUuid();

        if (contents.isNull() || hasLabel) return;

        // Pause scenario to edit component
        SimulationLoop* simLoop = m_engine->simulationLoop();
        bool wasPlaying = simLoop->isPlaying();
        if (wasPlaying) {
            simLoop->pause();
        }
        m_engine->simulationLoop()->pause();

        // Edit component via JSON
        m_sortingLayer->loadFromJson(contents.object());

        // Unpause scenario
        if (wasPlaying) {
            simLoop->play();
        }

        //progress.setValue(1);
        emit editedScriptOrder();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptJsonWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout;

    // Format widget sizes
    //m_textEdit->setMaximumWidth(300);
    m_textEdit->setMaximumHeight(150);

    // Create new layouts
    //QBoxLayout* labelLayout = new QHBoxLayout;
    //labelLayout->addWidget(m_typeLabel);
    //labelLayout->setAlignment(Qt::AlignCenter);

    QBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addWidget(m_textEdit);
    layout->addWidget(m_confirmButton);

    // Add layouts to main layout
    //m_mainLayout->addLayout(labelLayout);
    m_mainLayout->addLayout(layout);

    // Note, cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    setLayout(m_mainLayout);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ScriptOrderItem
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptOrderItem::ScriptOrderItem(SortingLayer * layer) :
    QTreeWidgetItem((int)getItemType(layer)),
    m_sortingLayer(layer),
    m_widget(nullptr)
{
    initializeItem();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptOrderItem::~ScriptOrderItem()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderItem::setWidget()
{
    // Throw error if the widget already exists
    if (m_widget) throw("Error, item already has a widget");

    // Get parent tree widget
    ScriptOrderTreeWidget * parentWidget = static_cast<ScriptOrderTreeWidget*>(treeWidget());

    // Set widget
    QJsonDocument doc(m_sortingLayer->asJson().toObject());
    QString rep(doc.toJson(QJsonDocument::Indented));
    m_widget = new ScriptJsonWidget(parentWidget->m_engine, m_sortingLayer);

    // Connect signals and slots
    QObject::connect((ScriptJsonWidget*)m_widget, &ScriptJsonWidget::editedScriptOrder,
        parentWidget, &ScriptOrderTreeWidget::repopulate);

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 0, m_widget);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderItem::removeWidget()
{
    // Only ever one column, so don't need to worry about indexing
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
View::ScriptOrderTreeWidget * ScriptOrderItem::scriptOrderTreeWidget() const
{
    return static_cast<View::ScriptOrderTreeWidget*>(treeWidget());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptOrderItem::ScriptOrderItemType ScriptOrderItem::getItemType(SortingLayer* layer)
{
    Q_UNUSED(layer)
    return kSortingLayer;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderItem::initializeItem()
{
    // Set column text
    //refreshText();

    // Set default size of item
    //setSizeHint(0, QSize(200, 400));

    // Set flags to be drag and drop enabled
    //setFlags(flags() | (Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled));
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ScriptOrderTreeWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptOrderTreeWidget::ScriptOrderTreeWidget(CoreEngine* engine,
    const QString& name, 
    QWidget* parent) :
    AbstractService(name),
    QTreeWidget(parent),
    m_engine(engine)
{
    initializeWidget();
    repopulate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptOrderTreeWidget::~ScriptOrderTreeWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::repopulate()
{
    // Clear the widget
    clear();

    // Set header item 
    //setHeaderItem(scenario);
    //invisibleRootItem()->setFlags(invisibleRootItem()->flags() ^ Qt::ItemIsDropEnabled);
    resizeColumns();

    // Get process manager
    ProcessManager* pm = m_engine->processManager();

    // Reorder map of sorting layers
    //const std::map<QString, SortingLayer*>& sortingLayers = pm->sortingLayers();
    m_sortedLayers.clear();
    for (const std::pair<QString, SortingLayer*>& sortPair : pm->sortingLayers()) {
        m_sortedLayers.emplace(sortPair.second->m_order, sortPair.second);
    }

    // Add sorting layers to widget
    for (const std::pair<int, SortingLayer*>& sortPair : m_sortedLayers) {
        addItem(sortPair.second);
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::addItem(SortingLayer * sortingLayer)
{
    // Create resource item
    ScriptOrderItem* layerItem = new View::ScriptOrderItem(sortingLayer);

    // Add resource item
    addItem(layerItem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::addItem(View::ScriptOrderItem * item)
{
    // Insert sorting layer item into the widget
    addTopLevelItem(item);

    // Create widget for component
    item->setWidget();

    // Resize columns to fit widget
    resizeColumns();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::removeItem(ScriptOrderItem * scriptOrderItem)
{
    delete scriptOrderItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::removeItem(SortingLayer* itemObject)
{
    ScriptOrderItem* item = getItem(itemObject);
    delete item;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ScriptOrderItem * ScriptOrderTreeWidget::getItem(SortingLayer * itemObject)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        ScriptOrderItem* item = static_cast<ScriptOrderItem*>(*it);
        if (item->sortingLayer()->getUuid() == itemObject->getUuid()) {
            return item;
        }
        ++it;
    }

    throw("Error, no item found that corresponds to the given object");

    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::resizeColumns()
{
    resizeColumnToContents(0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::onItemDoubleClicked(QTreeWidgetItem * item, int column)
{
    Q_UNUSED(item);
    Q_UNUSED(column);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::onItemExpanded(QTreeWidgetItem * item)
{
    Q_UNUSED(item);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::reorder(SortingLayer* layer, 
    SortingLayer* otherLayer,
    bool before)
{
    // Get iterator for layer next to dropped layer
    std::multimap<int, SortingLayer*>::const_iterator iter =
        std::find_if(m_sortedLayers.begin(),
            m_sortedLayers.end(),
            [&](const std::pair<int, SortingLayer*>& layerPair) {
        return layerPair.second->getUuid() == otherLayer->getUuid();
    });

    if (iter == m_sortedLayers.end()) {
        throw("Error, iterator not found");
    }

    std::multimap<int, SortingLayer*>::const_iterator neighborIter;
    if (before) {
        // Moved before iterator
        if (iter == m_sortedLayers.begin()) {
            layer->m_order = iter->second->m_order - 1;
        }
        else {
            neighborIter = std::prev(iter);
            if (neighborIter->second->m_order == iter->second->m_order) {
                layer->m_order = iter->second->m_order;
            }
            else {
                layer->m_order = iter->second->m_order - 1;
            }
        }
    }
    else {
        // Moved after iterator
        neighborIter = std::next(iter);
        if (neighborIter == m_sortedLayers.end()) {
            layer->m_order = iter->second->m_order + 1;
        }
        else {
            if (neighborIter->second->m_order == iter->second->m_order) {
                layer->m_order = iter->second->m_order;
            }
            else {
                layer->m_order = iter->second->m_order + 1;
            }
        }
    }

    m_engine->processManager()->refreshProcessOrder();
    repopulate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::mouseReleaseEvent(QMouseEvent * event)
{
    QTreeWidget::mouseReleaseEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::dropEvent(QDropEvent * event)
{
    ScriptOrderTreeWidget* widget = static_cast<ScriptOrderTreeWidget*>(event->source());
    if (widget == this) {
        ScriptOrderItem* sourceItem = static_cast<ScriptOrderItem*>(widget->currentItem());
        QModelIndex destIndex = indexAt(event->pos());
        if (destIndex.isValid()) {
            // Dropping onto an item
            DropIndicatorPosition dropIndicator = dropIndicatorPosition();
            ScriptOrderItem* destItem;
            switch (dropIndicator) {
            case QAbstractItemView::AboveItem:
                // Dropping above an item
                destItem = static_cast<ScriptOrderItem*>(itemFromIndex(destIndex));
                reorder(sourceItem->sortingLayer(),
                    destItem->sortingLayer(),
                    true);
                event->setDropAction(Qt::IgnoreAction);
                break;
            case QAbstractItemView::BelowItem:
                // Dropping below an item
                destItem = static_cast<ScriptOrderItem*>(itemFromIndex(destIndex));
                reorder(sourceItem->sortingLayer(),
                    destItem->sortingLayer(),
                    false);
                event->setDropAction(Qt::IgnoreAction);
                break;
            case QAbstractItemView::OnItem:
            {
                // On an item, so add current item as child
                event->setDropAction(Qt::IgnoreAction);
                break;
            }
            case QAbstractItemView::OnViewport:
                // Not on the tree 
                event->setDropAction(Qt::IgnoreAction);
                break;
            }
        }
        else {
            // Not dropping onto any item
            event->setDropAction(Qt::IgnoreAction);
        }
    }
    else {
        // Ignore drops from another widget
        event->setDropAction(Qt::IgnoreAction);
    }
    QTreeWidget::dropEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::dragEnterEvent(QDragEnterEvent * event)
{
    QTreeWidget::dragEnterEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::dragLeaveEvent(QDragLeaveEvent * event)
{
    QTreeWidget::dragLeaveEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::dragMoveEvent(QDragMoveEvent * event)
{
    QTreeWidget::dragMoveEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::initializeWidget()
{
    //setMinimumWidth(350);
    setMinimumSize(350, 350);

    // Set tree widget settings
    setColumnCount(0);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setHeaderLabels(QStringList({ "" }));
    setAlternatingRowColors(true);

    // Enable drag and drop
    setDragEnabled(true);
    setDragDropMode(DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropOverwriteMode(false); // is default, but making explicit for reference

    // Initialize actions
    m_addScriptLayer = new QAction(tr("&Add Sorting Layer"), this);
    m_addScriptLayer->setStatusTip("Add a sorting layer for the script execution order");
    connect(m_addScriptLayer,
        &QAction::triggered,
        m_engine->processManager(),
        [this] {
        // Add sorting layer
        m_engine->processManager()->addSortingLayer();
        repopulate();
    });

    m_removeScriptLayer = new QAction(tr("&Remove Sorting Layer"), this);
    m_removeScriptLayer->setStatusTip("Remove sorting layer for the script execution order");
    connect(m_removeScriptLayer,
        &QAction::triggered,
        m_engine->processManager(),
        [this] {
        // Remove sorting layer and repopulate widget
        m_engine->processManager()->removeSortingLayer(m_currentScriptOrderItem->sortingLayer()->m_label);
        repopulate();
    });



    // Connect signal for double click events
    connect(this, &ScriptOrderTreeWidget::itemDoubleClicked,
        this, &ScriptOrderTreeWidget::onItemDoubleClicked);

    // Connect signal for item expansion
    connect(this, &ScriptOrderTreeWidget::itemExpanded,
        this, &ScriptOrderTreeWidget::onItemExpanded);

}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptOrderTreeWidget::contextMenuEvent(QContextMenuEvent * event)
{
    // Return if there is no scene object selected
    if (!m_currentScriptOrderItem) {
        return;
    }

    // Create menu
    QMenu menu(this);

    // Add actions to the menu
    if (itemAt(event->pos())) {
        auto* item = static_cast<ScriptOrderItem*>(itemAt(event->pos()));
        m_currentScriptOrderItem = item;
        menu.addAction(m_removeScriptLayer);
    }
    else {
        // Options to add script layer when there is no item selected
        menu.addAction(m_addScriptLayer);
    }

    // Display menu at click location
    menu.exec(event->globalPos());
}


///////////////////////////////////////////////////////////////////////////////////////////////////
}
}