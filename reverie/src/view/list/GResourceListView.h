#ifndef GB_RESOURCE_LIST_VIEw 
#define GB_RESOURCE_LIST_VIEw
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes
#include <vector>

// Qt
#include <QAbstractListModel>
#include <QPixmap>
//#include <QPoint>
//#include <QStringList>
//#include <QVector>
#include <QMimeData>

// Project
#include "../../core/containers/GString.h"
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
class ResourceHandle;

namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ResourceDelegate
class ResourceDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    
    /// @brief  Sets the contents of the editor to the data for the item
    /// @note The index contains information about the model being used
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    /// @brief Gets data from the editor widget and stores it in the specified model at the item index.
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

//private slots:
    //void commitAndCloseEditor();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ResourcesModel : public QAbstractListModel, public Object {
    Q_OBJECT

public:
    explicit ResourcesModel(CoreEngine* core, QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    /// @brief Return the flags for the item at the given index
    /// @details This indicates whicj items can be dragged and dropped
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /// @brief To support additional drop actions, must be reimplemented to accomodate additional  mimeTypes
    /// @note Row and column are -1 when data dropped directly on parent
    /// @param[in] action the action ending a drag and drop operation
    /// @param[in] row the row of an item in the model where the operation ended
    /// @param[in] column the column of an item in the model where the operation ended
    /// @param[in] parent the index of a parent item in the model where the operation ended
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    /// @brief Returns the list of supported MIME types
    QStringList mimeTypes() const override;
    int rowCount(const QModelIndex &parent) const override;
    Qt::DropActions supportedDropActions() const override;

    /// @brief Implement to support drag and drop operations
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void repopulate();

private:

    std::shared_ptr<ResourceHandle> handle(size_t idx) const;

    CoreEngine* m_engine;
    std::vector<Uuid> m_resourceIDs;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ResourceListView
class ResourceListView : public QListView, public AbstractService {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ResourceListView(CoreEngine* engine, const QString& name, QWidget* parent = nullptr);
    ~ResourceListView();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{
    virtual const char* className() const override { return "ResourceListView"; }
    virtual const char* namespaceName() const override { return "rev::View::ResourceListView"; }

    /// @brief Returns True if this AbstractService represents a service
    /// @details This is not a service
    virtual bool isService() const override { return false; };

    /// @brief Returns True if this AbstractService represents a tool.
    /// @details This is not a tool
    virtual bool isTool() const override { return false; };

    /// @}
signals:

protected slots:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Slots
    /// @{
    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{


    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Initialize the widget
    virtual void initializeWidget();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    // Models and delegates aren't deleted by default, so save here and delete with widget
    ResourceDelegate m_itemDelegate;
    ResourcesModel m_itemModel;

    /// @brief Core engine for the application
    CoreEngine* m_engine;

    /// @}
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // rev

#endif





