#include "geppetto/qt/widgets/list/GBlueprintView.h"

#include <QIcon>
#include <QMenu>
#include <QRandomGenerator>
#include <QContextMenuEvent>

#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/layer/types/GQtConverter.h"
#include "geppetto/qt/actions/GActionManager.h"
#include "geppetto/qt/actions/commands/GUndoCommand.h"
#include "geppetto/qt/widgets/GWidgetManager.h"

#include "ripple/network/gateway/GMessageGateway.h"
#include "ripple/network/messages/GCreateBlueprintMessage.h"
#include "ripple/network/messages/GBlueprintsDataMessage.h"
#include "ripple/network/messages/GModifyBlueprintMessage.h"
#include "ripple/network/messages/GRequestBlueprintsDataMessage.h"
#include "ripple/network/messages/GScenarioJsonMessage.h"

namespace rev {

void BlueprintDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

QSize BlueprintDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget * BlueprintDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void BlueprintDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}

void BlueprintDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}




BlueprintModel::BlueprintModel(WidgetManager* wm, QObject *parent):
    QAbstractListModel(parent),
    m_widgetManager(wm)
{
    // Connect signal for repopulating on scenario load
    connect(m_widgetManager,
        &WidgetManager::receivedScenarioJsonMessage,
        this,
        [&](const GScenarioJsonMessage&) {
            repopulate(m_widgetManager->scenarioJson()["blueprints"]);
        });

    // Connect signal for repopulating on receipt of blueprint data message
    connect(m_widgetManager,
        &WidgetManager::receivedBlueprintsDataMessage,
        this,
        [&](const GBlueprintsDataMessage& message) {
            json scenarioJson = m_widgetManager->scenarioJson();
            scenarioJson["blueprints"] = GJson::FromBytes(message.getBlueprintsJsonBytes());
            m_widgetManager->setScenarioJson(scenarioJson);
            repopulate(scenarioJson["blueprints"]);
        });
}

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

    const json& bpJson = m_widgetManager->scenarioJson()["blueprints"][idx];

    const std::string& blueprintName = bpJson["name"].get_ref<const std::string&>();
    Uuid blueprintId = bpJson["uuid"].get<Uuid>();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        // The data to be rendered in the form of text
        QString str(blueprintName.c_str());
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
        return QUuid(blueprintId.asString().c_str());
    }
    else if (role == Qt::UserRole + 1) {
        // Type, which is always a Blueprint for data from this widget
        return (int)BlueprintModelDragTypes::kBlueprint;
    }
    //else if (role == Qt::UserRole + 1)
    //    return locations.value(index.row());

    return QVariant();
}

bool BlueprintModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        static GModifyBlueprintMessage s_message;
        s_message.setNewName("");
        s_message.setNewUuid(Uuid::NullID());
        s_message.setUuid(Uuid::NullID());

        if (value.type() == QVariant::Type::Uuid) {
            // Change Uuid if modified
            Uuid id(value.toUuid().toString().toStdString());
            s_message.setNewUuid(id);
            s_message.setUuid(m_blueprintIds[index.row()]);
            if (id != m_blueprintIds[index.row()]) {
                m_blueprintIds[index.row()] = id;
            }
        }
        else if (value.type() == QVariant::Type::String) {
            GString str = value.toString().toStdString();
            s_message.setUuid(m_blueprintIds[index.row()]);
            s_message.setNewName(str.c_str());
        }

        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);

        // Should be emitted when data set successfully, as per documentation
        emit dataChanged(index, index, { role });
        return true;
    }
    return false;
}

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

QStringList BlueprintModel::mimeTypes() const
{
    QStringList types;
    types << "reverie/id/uuid";
    types << "reverie/id/type";
    return types;
}

QMimeData *BlueprintModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes) {
        if (index.isValid()) {
            uint32_t idx = index.row();
            const QUuid& uuid = data(index, Qt::UserRole).toUuid();
            GString uuidStr = uuid.toString().toStdString();
#ifdef DEBUG_MODE
            std::cout << uuidStr;
#endif
            stream << uuid;
        }
    }

    mimeData->setData("reverie/id/uuid", encodedData);
    return mimeData;
}

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
    QUuid itemId;
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
        static GCreateBlueprintMessage s_message;
        Uuid itemUuid = QConverter::FromQt(itemId);
        s_message.setSceneObjectId(static_cast<Uint32_t>(itemUuid));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);

        // Repopulate the widget
        requestRepopulate();

        // Return false, don't want to remove from original widget
        return false;
        break;
    }
    default:
        break;
    }

    return true;
}

int BlueprintModel::rowCount(const QModelIndex &parent) const
{
    return (int)m_blueprintIds.size(); 
}

Qt::DropActions BlueprintModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

bool BlueprintModel::removeRows(int row, int count, const QModelIndex & parent)
{
    // Note, needs to be implemented if dragging and dropping
    if (m_blueprintIds.size()) {
        uint32_t lastRowCount = row + count;
        beginRemoveRows(QModelIndex(), row, lastRowCount - 1);
        m_blueprintIds.erase(m_blueprintIds.begin() + row, m_blueprintIds.begin() + lastRowCount);
        endRemoveRows();
        return true;
    }
    else {
        return false;
    }
}

void BlueprintModel::requestRepopulate()
{
    static GRequestBlueprintsDataMessage s_message;
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}

void BlueprintModel::repopulate(const json& blueprintsJson)
{
    // Clear resource IDs from the model
    if (!m_blueprintIds.empty()) {
        beginRemoveRows(QModelIndex(), 0, (Uint32_t)m_blueprintIds.size() - 1);
        m_blueprintIds.clear();
        endRemoveRows();
    }

    // Add blueprints
    Uint32_t numBlueprints = (Uint32_t)blueprintsJson.size();

    if (numBlueprints == 0) {
        return;
    }

    // Need to wrap in begin/end whenever data is inserted/removed
    // If data were being changed instead of inserted/removed, emit dataChanged()
    beginInsertRows(QModelIndex(), 0, numBlueprints - 1);
    for (const json& blueprint: blueprintsJson) {
        m_blueprintIds.push_back(blueprint["uuid"].get<Uuid>());
    }
    endInsertRows();
}




BlueprintListView::BlueprintListView(WidgetManager* wm, const QString& name, QWidget* parent):
    QListView(parent),
    rev::NameableInterface(name.toStdString()),
    m_itemModel(wm),
    m_widgetManager(wm)
{
    initializeWidget();
}


BlueprintListView::~BlueprintListView()
{
}

void BlueprintListView::contextMenuEvent(QContextMenuEvent * event)
{
    QModelIndex clickedIndex = indexAt(event->pos());

    if (!clickedIndex.isValid()) {
        return;
    }

    // Get blueprint
    const json& blueprints = m_widgetManager->scenarioJson()["blueprints"];
    Uuid blueprintId = QConverter::FromQt(m_itemModel.data(clickedIndex, Qt::UserRole).toUuid());
    auto iter = std::find_if(blueprints.begin(), blueprints.end(),
        [blueprintId](const json& bp) {
            return bp["uuid"].get<Uuid>() == blueprintId; 
        }
    );
#ifdef DEBUG_MODE
    assert(iter != blueprints.end() && "Error, blueprint not found");
#endif

    // Create menu
    QMenu menu(this);
    
    // Add actions to the menu
    QAction* deleteAction = menu.addAction(SAIcon("trash-alt"), "Delete");
    //connect(deleteAction, &QAction::triggered, this, onDeleteAction);

    // Display menu at click location
    menu.exec(event->globalPos());
}

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


} // rev