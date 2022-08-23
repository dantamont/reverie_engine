#include "geppetto/qt/widgets/list/GResourceListView.h"

#include <QIcon>
#include <QRandomGenerator>

#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/layer/types/GQtConverter.h"
#include "geppetto/qt/actions/GActionManager.h"
#include "geppetto/qt/actions/commands/GUndoCommand.h"
#include "geppetto/qt/widgets/GWidgetManager.h"

#include "enums/GResourceTypeEnum.h"
#include "ripple/network/gateway/GMessageGateway.h"
#include "ripple/network/messages/GModifyResourceMessage.h"
#include "ripple/network/messages/GRequestResourcesDataMessage.h"
#include "ripple/network/messages/GResourcesDataMessage.h"
#include "ripple/network/messages/GResourceAddedMessage.h"
#include "ripple/network/messages/GResourceModifiedMessage.h"
#include "ripple/network/messages/GResourceRemovedMessage.h"
#include "ripple/network/messages/GScenarioJsonMessage.h"

namespace rev {

void ResourceDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    // TODO: Paint
    /// \see https://stackoverflow.com/questions/43035378/qtreeview-item-hover-selected-background-color-based-on-current-color

    QStyledItemDelegate::paint(painter, option, index);
}

QSize ResourceDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget * ResourceDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void ResourceDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}

void ResourceDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}



ResourcesModel::ResourcesModel(WidgetManager* wm, QObject *parent):
    QAbstractListModel(parent),
    m_widgetManager(wm)
{

    // Connect signal for repopulating on scenario load
    connect(m_widgetManager,
        &WidgetManager::receivedScenarioJsonMessage,
        this,
        [&](const GScenarioJsonMessage&) { repopulate(); },
        Qt::DirectConnection);
    connect(m_widgetManager,
        &WidgetManager::receivedResourcesDataMessage,
        this,
        [&](const GResourcesDataMessage&) { repopulate(); },
        Qt::DirectConnection);

    // Connect signals for repopulating on resource modifications
    connect(m_widgetManager,
        &WidgetManager::receivedResourceModifiedMessage,
        this,
        [this](const GResourceModifiedMessage&) { requestRepopulate(); },
        Qt::DirectConnection);
    connect(m_widgetManager,
        &WidgetManager::receivedResourceAddedMessage,
        this,
        [this](const GResourceAddedMessage&) { requestRepopulate(); },
        Qt::DirectConnection);
    connect(m_widgetManager,
        &WidgetManager::receivedResourceRemovedMessage,
        this,
        [this](const GResourceRemovedMessage&) { requestRepopulate(); },
        Qt::DirectConnection);
}

QVariant ResourcesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int idx = index.row();
    if (idx < 0) {
        // No item selected, so return
        return QVariant();
    }

    const json& resourcesJson = m_widgetManager->scenarioJson()["resourceCache"]["resources"];
    if(idx >= resourcesJson.size()){
        //Logger::Throw("Something has gone awry with resource list size");
        return QVariant();
    }
    
    const json& handleJson = resourcesJson[idx];
    const Uuid handleId = handleJson["uuid"].get<Uuid>();
    const std::string& name = handleJson["name"].get_ref<const std::string&>();
    const EResourceType type = static_cast<EResourceType>(handleJson["type"].get<Int32_t>());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        // The data to be rendered in the form of text
        QString str(name.c_str());
        return str;
    }
    else if (role == Qt::DecorationRole) {
        // The data to be rendered as a decoration in the form of an icon
        QIcon icon;
        switch (type) {
        case EResourceType::eTexture:
            icon = SAIcon("file-image");
            break;
        case EResourceType::eMaterial:
            icon = SAIcon("images");
            break;
        case EResourceType::eMesh:
            icon = SAIcon("dice-d20");
            break;
        case EResourceType::eCubeTexture:
            icon = SAIcon("cube");
            break;
        case EResourceType::eAnimation:
            icon = SAIcon("running");
            break;
        case EResourceType::eModel:
            icon = SAIcon("blender");
            break;
        case EResourceType::eShaderProgram:
            icon = SAIcon("paint-brush");
            break;
        case EResourceType::ePythonScript:
            icon = SAIcon("python");
            break;
        case EResourceType::eSkeleton:
            icon = SAIcon("male");
            break;
        case EResourceType::eAudio:
            icon = SAIcon("file-audio");
            break;
        default:
            icon = SAIcon("file");
            break;
        }
        return icon;
    }
    else if (role == Qt::ForegroundRole) {
        // Set text (but not icon) color
        //QColor color(Qt::GlobalColor::cyan);
        QPalette palette = QApplication::palette();
        return palette.text();
    }
    else if (role == Qt::UserRole) {
        return QConverter::ToQt(handleId);
    }
    //else if (role == Qt::UserRole + 1)
    //    return locations.value(index.row());

    return QVariant();
}

bool ResourcesModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        if (value.type() == QVariant::Type::Uuid) {
            // Change Uuid if modified
            Uuid id = QConverter::FromQt(value.toUuid());
            requestResourceIdChange(m_resourceIDs[index.row()], id);
            if (id != m_resourceIDs[index.row()]) {
                m_resourceIDs[index.row()] = id;
            }
        }
        else if (value.type() == QVariant::Type::String) {
            GString str = value.toString().toStdString();
            Uuid id = m_resourceIDs[index.row()];
            requestResourceNameChange(id, str);
        }
        emit dataChanged(index, index, { role }); // Qt requirement to emit this
        return true;
    }
    return false;
}

Qt::ItemFlags ResourcesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    // Allow dragging of items, but only dropping for top level of the model
    if (index.isValid())
        // Also make editable
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QStringList ResourcesModel::mimeTypes() const
{
    QStringList types;
    types << "reverie/id/uuid";
    return types;
}

QMimeData *ResourcesModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes) {
        if (index.isValid()) {
            uint32_t idx = index.row();
            QUuid uuid = data(index, Qt::UserRole).toUuid();
            QString uuidStr = uuid.toString();
#ifdef DEBUG_MODE
            std::cout << uuidStr.toStdString();
#endif
            stream << uuid;
        }
    }

    mimeData->setData("reverie/id/uuid", encodedData);
    return mimeData;
}

bool ResourcesModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
    int row, int column, const QModelIndex &parent)
{
    // NOTE: Can forbid dropping on certain items by reimplementing QAbstractItemModel::canDropMimeData()
    // TODO: Move these to that function
    if (!data->hasFormat("reverie/id/uuid"))
        return false;

    if (action == Qt::IgnoreAction)
        return true;

    if (column > 0)
        return false;

    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    QByteArray encodedData = data->data("reverie/id/uuid");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QUuid handleID;
    while (!stream.atEnd()) {
        // Should only do this once, but usinng this since reinterpret_cast with encoded data can be a mess
        stream >> handleID;
    }

    return true;
}

int ResourcesModel::rowCount(const QModelIndex &parent) const
{
    return m_resourceIDs.size(); 
}

Qt::DropActions ResourcesModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

bool ResourcesModel::removeRows(int row, int count, const QModelIndex & parent)
{
    // Note, needs to be implemented if dragging and dropping
    if (m_resourceIDs.size()) {
        uint32_t lastRowCount = row + count;
        beginRemoveRows(QModelIndex(), row, lastRowCount - 1);
        m_resourceIDs.erase(m_resourceIDs.begin() + row, m_resourceIDs.begin() + lastRowCount);
        endRemoveRows();
        return true;
    }
    else {
        return false;
    }
}

void ResourcesModel::requestRepopulate()
{
    static GRequestResourcesDataMessage s_message;
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}

void ResourcesModel::repopulate()
{
    // Clear resource IDs from the model
    if (!m_resourceIDs.empty()) {
        beginRemoveRows(QModelIndex(), 0, m_resourceIDs.size() - 1);
        m_resourceIDs.clear();
        endRemoveRows();
    }

    // Add top-level resource IDs
    const json& resourcesJson = m_widgetManager->scenarioJson()["resourceCache"]["resources"];
    size_t numTopResources = resourcesJson.size();

    if (numTopResources == 0) {
        return;
    }

    // Need to wrap in begin/end whenever data is inserted/removed
    // If data were being changed instead of inserted/removed, emit dataChanged()
    beginInsertRows(QModelIndex(), 0, (uint32_t)numTopResources - 1);
    for (const json& handleJson: resourcesJson) {
        m_resourceIDs.push_back(handleJson["uuid"].get<Uuid>());
    }
    endInsertRows();
}

void ResourcesModel::requestResourceIdChange(const Uuid& resourceId, const Uuid& newId)
{
    static GModifyResourceMessage s_message;
    s_message.setUuid(resourceId);
    s_message.setNewUuid(newId);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}

void ResourcesModel::requestResourceNameChange(const Uuid& resourceId, const GString& resourceName)
{
    static GModifyResourceMessage s_message;
    s_message.setUuid(resourceId);
    s_message.setNewName(resourceName.c_str());
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}



ResourceListView::ResourceListView(WidgetManager* wm, const QString& name, QWidget* parent):
    QListView(parent),
    rev::NameableInterface(name.toStdString()),
    m_itemModel(wm),
    m_widgetManager(wm)
{
    initializeWidget();
}


ResourceListView::~ResourceListView()
{
}

void ResourceListView::initializeWidget() 
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
    setDragDropMode(QAbstractItemView::InternalMove); // So that copy isn't used

    // Set the model to use
    setModel(&m_itemModel);

    // Set the item delegate to use
    setItemDelegate(&m_itemDelegate);
}

} // rev