#include "GBlueprintView.h"

#include <QIcon>
#include <QRandomGenerator>

#include "../../model_control/commands/GActionManager.h"
#include "../../core/GCoreEngine.h"
#include "../../core/resource/GResource.h"
#include "../../core/resource/GResourceCache.h"
#include "../../core/scene/GScenario.h"
#include "../../model_control/commands/GUndoCommand.h"
#include "../style/GFontIcon.h"

#include <core/scene/GSceneObject.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {
namespace View {


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IconDelegate
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlueprintDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QSize BlueprintDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QWidget * BlueprintDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QStyledItemDelegate::createEditor(parent, option, index);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlueprintDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlueprintDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BlueprintModel
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BlueprintModel::BlueprintModel(CoreEngine* core, QObject *parent):
    QAbstractListModel(parent),
    m_engine(core)
{

    // Connect signal for repopulating on scenario load
    connect(m_engine,
        &CoreEngine::scenarioLoaded,
        this,
        &BlueprintModel::repopulate,
        Qt::QueuedConnection);

    // Connect signals for repopulating on resource modifications
    //connect(m_engine->resourceCache(),
    //    &CoreEngine::scenarioChanged,
    //    this,
    //    [this](){repopulate(); },
    //    Qt::QueuedConnection);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QVariant BlueprintModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    int idx = index.row();
    if (idx < 0) {
        // No item selected, so return
        return QVariant();
    }

    Blueprint& bp = BlueprintModel::blueprint(idx);

    const Uuid& blueprintId = bp.getUuid();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        // The data to be rendered in the form of text
        QString str(bp.getName().c_str());
        return str;
    }
    else if (role == Qt::DecorationRole) {
        // The data to be rendered as a decoration in the form of an icon
        QIcon icon = SAIcon("drafting-compass");
        return icon;
    }
    else if (role == Qt::ForegroundRole) {
        // Set text (but not icon) color
        //QColor color(Qt::GlobalColor::cyan);
        QPalette palette = QApplication::palette();
        return palette.text();
    }
    else if (role == Qt::UserRole) {
        return bp.getUuid();
    }
    else if (role == Qt::UserRole + 1) {
        // Type, which is always a Blueprint for data from this widget
        return (int)BlueprintModelDragTypes::kBlueprint;
    }
    //else if (role == Qt::UserRole + 1)
    //    return locations.value(index.row());

    return QVariant();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BlueprintModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        if (value.type() == QVariant::Type::Uuid) {
            // Change Uuid if modified
            Uuid id = value.toUuid();
            if (id != m_blueprintIds[index.row()]) {
                m_blueprintIds[index.row()] = id;
            }
        }
        else if (value.type() == QVariant::Type::String) {
            QString str = value.toString();
            Blueprint& bp = blueprint(index.row());
            bp.setName(str);
        }

        // Should be emitted when data set successfully, as per documentation
        emit dataChanged(index, index, { role });
        return true;
    }
    return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Qt::ItemFlags BlueprintModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    // Allow dragging of items, but only dropping for top level of the model
    if (index.isValid())
    {
        // Also make editable
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | defaultFlags;
    }
    else
    {
        return Qt::ItemIsDropEnabled | defaultFlags;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BlueprintModel::canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const
{
    if (!data->hasFormat("reverie/id/uuid"))
        return false;

    if (action == Qt::IgnoreAction)
        return true;

    if (column > 0)
        return false;

    bool canDrop = QAbstractListModel::canDropMimeData(data, action, row, column, parent);

    return canDrop;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QStringList BlueprintModel::mimeTypes() const
{
    QStringList types;
    types << "reverie/id/uuid";
    types << "reverie/id/type";
    return types;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QMimeData *BlueprintModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes) {
        if (index.isValid()) {
            size_t idx = index.row();
            const Uuid& uuid = data(index, Qt::UserRole).toUuid();
            QString uuidStr = uuid.toString();
#ifdef DEBUG_MODE
            Logger::LogInfo(uuidStr);
#endif
            stream << uuid;
        }
    }

    mimeData->setData("reverie/id/uuid", encodedData);
    return mimeData;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BlueprintModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
    int row, int column, const QModelIndex &parent)
{
    // Forbid dropping on certain items by reimplementing QAbstractItemModel::canDropMimeData()
    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    QByteArray encodedIdData = data->data("reverie/id/uuid");
    QByteArray encodedTypeData = data->data("reverie/id/type");
    QDataStream stream(&encodedIdData, QIODevice::ReadOnly);
    QDataStream typeStream(&encodedTypeData, QIODevice::ReadOnly);
    Uuid itemId;
    int itemType = -1;
    while (!stream.atEnd()) {
        // Should only do this once, but using this since reinterpret_cast with encoded data can be a mess
        stream >> itemId;
    }

    while (!typeStream.atEnd()) {
        // Won't be reached if no type
        typeStream >> itemType;
    }

    switch (BlueprintModelDragTypes(itemType)) {
    case BlueprintModelDragTypes::kSceneObject: {
        // Add item to scenario
        const std::shared_ptr<SceneObject>& so = m_engine->scenario()->scene().getSceneObject((size_t)itemId);
        so->createBlueprint();

        // Repopulate the widget
        repopulate();

        // Return false, don't want to remove from original widget
        return false;
        break;
    }
    default:
        break;
    }

    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int BlueprintModel::rowCount(const QModelIndex &parent) const
{
    return m_blueprintIds.size(); 
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Qt::DropActions BlueprintModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BlueprintModel::removeRows(int row, int count, const QModelIndex & parent)
{
    // Note, needs to be implemented if dragging and dropping
    if (m_blueprintIds.size()) {
        size_t lastRowCount = row + count;
        beginRemoveRows(QModelIndex(), row, lastRowCount - 1);
        m_blueprintIds.erase(m_blueprintIds.begin() + row, m_blueprintIds.begin() + lastRowCount);
        endRemoveRows();
        return true;
    }
    else {
        return false;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlueprintModel::repopulate()
{
    // Clear resource IDs from the model
    if (!m_blueprintIds.empty()) {
        beginRemoveRows(QModelIndex(), 0, m_blueprintIds.size() - 1);
        m_blueprintIds.clear();
        endRemoveRows();
    }

    // Add blueprints
    const std::vector<Blueprint>& blueprints = m_engine->scenario()->blueprints();
    size_t numBlueprints = m_engine->scenario()->blueprints().size();

    if (numBlueprints == 0) {
        return;
    }

    // Need to wrap in begin/end whenever data is inserted/removed
    // If data were being changed instead of inserted/removed, emit dataChanged()
    beginInsertRows(QModelIndex(), 0, numBlueprints - 1);
    for (const Blueprint& blueprint: blueprints) {
        m_blueprintIds.push_back(blueprint.getUuid());
    }
    endInsertRows();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Blueprint& BlueprintModel::blueprint(size_t idx) const
{
    std::vector<Blueprint>& blueprints = m_engine->scenario()->blueprints();
    if (blueprints.size() > (size_t)idx)
    {
        return blueprints[idx];
    }
    else {
        throw("Something has gone awry with blueprint list size");
        return Blueprint();
    }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceListView
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BlueprintListView::BlueprintListView(CoreEngine* engine, const QString& name, QWidget* parent):
    QListView(parent),
    AbstractService(name),
    m_itemModel(engine),
    m_engine(engine)
{
    initializeWidget();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BlueprintListView::~BlueprintListView()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlueprintListView::contextMenuEvent(QContextMenuEvent * event)
{
    QModelIndex clickedIndex = indexAt(event->pos());

    if (!clickedIndex.isValid()) {
        return;
    }

    // Get blueprint
    Uuid blueprintId = m_itemModel.data(clickedIndex, Qt::UserRole).toUuid();
    auto iter = std::find_if(m_engine->scenario()->blueprints().begin(),
        m_engine->scenario()->blueprints().end(),
        [blueprintId](const Blueprint& bp) {return bp.getUuid() == blueprintId; });
    if (iter == m_engine->scenario()->blueprints().end()) {
        throw("Error, blueprint not found");
    }
    Blueprint& bp = *iter;

    // Create menu
    QMenu menu(this);
    
    // Add actions to the menu
    QAction* deleteAction = menu.addAction(SAIcon("trash-alt"), "Delete");
    //connect(deleteAction, &QAction::triggered, this, onDeleteAction);

    // Display menu at click location
    menu.exec(event->globalPos());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlueprintListView::initializeWidget()
{
    // Set list-view settings
    setViewMode(QListView::IconMode);
    //setIconSize(QSize(32, 32));
    setResizeMode(QListView::Adjust);
    setWordWrap(true);

    setSelectionRectVisible(true);
    setSpacing(10);
    //setFont(QFont("Tahoma", 8.25));

    // Enable drag and drop
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);

    // Set the model to use
    setModel(&m_itemModel);

    // Set the item delegate to use
    setItemDelegate(&m_itemDelegate);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}// View
} // rev