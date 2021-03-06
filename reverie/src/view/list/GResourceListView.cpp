#include "GResourceListView.h"

#include <QIcon>
#include <QRandomGenerator>

#include "../../model_control/commands/GActionManager.h"
#include "../../core/GCoreEngine.h"
#include "../../core/resource/GResource.h"
#include "../../core/resource/GResourceCache.h"
#include "../../core/scene/GScenario.h"
#include "../../model_control/commands/GUndoCommand.h"
#include "../style/GFontIcon.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {
namespace View {


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IconDelegate
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    // TODO: Paint
    // See: https://stackoverflow.com/questions/43035378/qtreeview-item-hover-selected-background-color-based-on-current-color
    
    // background
    //QColor bgColor;
    //int bgColorType(0);
    //bgColorType = index.data(Qt::UserRole + 9).toInt();//custom flag I set to determine which color i want

    ////color logic
    //if (bgColorType == 0)
    //    bgColor = QColor(Qt::transparent);//default is transparent to retain alternate row colors
    //else if (bgColorType == 1)
    //    bgColor = qRgba(237, 106, 106, 255);//red
    //else if (bgColorType == 2)
    //    bgColor = qRgba(241, 167, 226, 255);//pink
    ////etc...

    //QStyleOptionViewItem opt(option);

    //if (option.state & QStyle::State_Selected)//check if item is selected
    //{
    //    //more color logic
    //    if (bgColorType == 0)
    //        bgColor = qRgba(190, 220, 240, 255);
    //    else
    //        bgColor = qRgba(bgColor.red() - 25, bgColor.green() - 25, bgColor.blue() - 25, 255);

    //    //background color won't show on selected items unless you do this
    //    opt.palette.setBrush(QPalette::Highlight, QBrush(bgColor));
    //}

    //if (option.state & QStyle::State_MouseOver)//check if item is hovered
    //{
    //    //more color logic
    //    bgColor = qRgba(bgColor.red() - 25, bgColor.green() - 25, bgColor.blue() - 25, 255);

    //    if (option.state & QStyle::State_Selected)//check if it is hovered AND selected
    //    {
    //        //more color logic
    //        if (bgColorType == 0)
    //        {
    //            bgColor = qRgba(148, 200, 234, 255);
    //        }

    //        //background color won't show on selected items unless you do this
    //        opt.palette.setBrush(QPalette::Highlight, QBrush(bgColor));
    //    }
    //}


    ////set the backgroundBrush to our color. This affects unselected items.
    //opt.backgroundBrush = QBrush(bgColor);

    ////draw the item background
    //option.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter);

    ////icon
    //QRect iconRect = option.rect;
    //iconRect.setLeft(iconRect.left() + 3);//offset it a bit to the right
    ////draw in icon, this can be grabbed from Qt::DecorationRole
    ////altho it appears icons must be set with setIcon()
    //option.widget->style()->drawItemPixmap(painter, iconRect, Qt::AlignLeft | Qt::AlignVCenter, QIcon(index.data(Qt::DecorationRole).value<QIcon>()).pixmap(16, 16));

    ////text
    //QRect textRect = option.rect;
    //textRect.setLeft(textRect.left() + 25);//offset it a bit to the right
    ////draw in text, this can be grabbed from Qt::DisplayRole
    //option.widget->style()->drawItemText(painter, textRect, Qt::AlignLeft | Qt::AlignVCenter, option.palette, true, index.data(Qt::DisplayRole).toString());


    QStyledItemDelegate::paint(painter, option, index);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QSize ResourceDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QWidget * ResourceDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QStyledItemDelegate::createEditor(parent, option, index);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourcesModel
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ResourcesModel::ResourcesModel(CoreEngine* core, QObject *parent):
    QAbstractListModel(parent),
    m_engine(core)
{

    // Connect signal for repopulating on scenario load
    connect(m_engine,
        &CoreEngine::scenarioLoaded,
        this,
        &ResourcesModel::repopulate,
        Qt::QueuedConnection);

    // Connect signals for repopulating on resource modifications
    connect(m_engine->resourceCache(),
        &ResourceCache::resourceChanged,
        this,
        [this](const Uuid&) {repopulate(); },
        Qt::QueuedConnection);
    connect(m_engine->resourceCache(),
        &ResourceCache::resourceAdded,
        this,
        [this](const Uuid&) {repopulate(); },
        Qt::QueuedConnection);
    connect(m_engine->resourceCache(),
        &ResourceCache::resourceDeleted,
        this,
        [this](const Uuid&) {repopulate(); },
        Qt::QueuedConnection);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QVariant ResourcesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int idx = index.row();
    if (idx < 0) {
        // No item selected, so return
        return QVariant();
    }

    std::shared_ptr<ResourceHandle> handle = ResourcesModel::handle(idx);
    if(!handle){
        //throw("Something has gone awry with resource list size");
        return QVariant();
    }
    
    const Uuid& handleID = handle->getUuid();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        // The data to be rendered in the form of text
        QString str(handle->getName().c_str());
        return str;
    }
    else if (role == Qt::DecorationRole) {
        // The data to be rendered as a decoration in the form of an icon
        QIcon icon;
        switch (handle->getResourceType()) {
        case ResourceType::kImage:
        case ResourceType::kTexture:
            icon = SAIcon("file-image");
            break;
        case ResourceType::kMaterial:
            icon = SAIcon("images");
            break;
        case ResourceType::kMesh:
            icon = SAIcon("dice-d20");
            break;
        case ResourceType::kCubeTexture:
            icon = SAIcon("cube");
            break;
        case ResourceType::kAnimation:
            icon = SAIcon("running");
            break;
        case ResourceType::kModel:
            icon = SAIcon("blender");
            break;
        case ResourceType::kShaderProgram:
            icon = SAIcon("paint-brush");
            break;
        case ResourceType::kPythonScript:
            icon = SAIcon("python");
            break;
        case ResourceType::kSkeleton:
            icon = SAIcon("male");
            break;
        case ResourceType::kAudio:
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
        return handleID;
    }
    //else if (role == Qt::UserRole + 1)
    //    return locations.value(index.row());

    return QVariant();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ResourcesModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        if (value.type() == QVariant::Type::Uuid) {
            // Change Uuid if modified
            Uuid id = value.toUuid();
            if (id != m_resourceIDs[index.row()]) {
                m_resourceIDs[index.row()] = id;
            }
        }
        else if (value.type() == QVariant::Type::String) {
            QString str = value.toString();
            auto h = handle(index.row());
            h->setName(str);
        }
        emit dataChanged(index, index, { role });
        return true;
    }
    return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QStringList ResourcesModel::mimeTypes() const
{
    QStringList types;
    types << "reverie/id/uuid";
    return types;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QMimeData *ResourcesModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    const ResourceCache& cache = *m_engine->resourceCache();
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
    Uuid handleID;
    while (!stream.atEnd()) {
        // Should only do this once, but usinng this since reinterpret_cast with encoded data can be a mess
        stream >> handleID;
    }

#ifdef DEBUG_MODE
    std::shared_ptr<ResourceHandle> handle = m_engine->resourceCache()->getHandle(handleID);
    Logger::LogInfo(handleID.toString());
    if (!handle) {
        Logger::LogError("Handle not found");
    }
    else {
        Logger::LogInfo("Dropped resource " + handle->getName());
    }
#endif
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ResourcesModel::rowCount(const QModelIndex &parent) const
{
    return m_resourceIDs.size(); 
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Qt::DropActions ResourcesModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ResourcesModel::removeRows(int row, int count, const QModelIndex & parent)
{
    // Note, needs to be implemented if dragging and dropping
    if (m_resourceIDs.size()) {
        size_t lastRowCount = row + count;
        beginRemoveRows(QModelIndex(), row, lastRowCount - 1);
        m_resourceIDs.erase(m_resourceIDs.begin() + row, m_resourceIDs.begin() + lastRowCount);
        endRemoveRows();
        return true;
    }
    else {
        return false;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ResourcesModel::repopulate()
{
    //static std::mutex repopulateMutex;
    //repopulateMutex.lock();

    // Clear resource IDs from the model
    if (!m_resourceIDs.empty()) {
        beginRemoveRows(QModelIndex(), 0, m_resourceIDs.size() - 1);
        m_resourceIDs.clear();
        endRemoveRows();
    }

    // Add top-level resource IDs
    const ResourceCache& cache = *m_engine->resourceCache();
    const std::deque<std::shared_ptr<ResourceHandle>>& handleList = cache.topLevelResources();
    size_t numTopResources = handleList.size();

    if (numTopResources == 0) {
        return;
    }

    // Need to wrap in begin/end whenever data is inserted/removed
    // If data were being changed instead of inserted/removed, emit dataChanged()
    beginInsertRows(QModelIndex(), 0, numTopResources - 1);
    for (const std::shared_ptr<ResourceHandle>& handle: handleList) {
        m_resourceIDs.push_back(handle->getUuid());
    }
    endInsertRows();

    //repopulateMutex.unlock();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourcesModel::handle(size_t idx) const
{
    const ResourceCache& cache = *m_engine->resourceCache();
    const std::deque<std::shared_ptr<ResourceHandle>>& handleList = cache.topLevelResources();
    std::shared_ptr<ResourceHandle> handle;
    if (handleList.size() > (size_t)idx)
    {
        return handleList[idx];
    }
    else {
        //throw("Something has gone awry with resource list size");
        return nullptr;
    }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceListView
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ResourceListView::ResourceListView(CoreEngine* engine, const QString& name, QWidget* parent):
    QListView(parent),
    AbstractService(name),
    m_itemModel(engine),
    m_engine(engine)
{
    initializeWidget();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ResourceListView::~ResourceListView()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}// View
} // rev