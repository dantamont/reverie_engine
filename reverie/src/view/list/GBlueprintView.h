#ifndef GB_BLUEPRINT_LIST_VIEW 
#define GB_BLUEPRINT_LIST_VIEW
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes
#include <vector>

// Qt
#include <QAbstractListModel>
#include <QPixmap>
#include <QMimeData>

// Project
#include "../../core/containers/GString.h"
#include "../../core/service/GService.h"
#include "../../core/mixins/GLoadable.h"
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
class Blueprint;

namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class BlueprintDelegate
class BlueprintDelegate : public QStyledItemDelegate
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

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class BlueprintModelDragTypes {
    kBlueprint = 0, // From within this widget
    kSceneObject = 1 // From SceneTreeWidget
};

class BlueprintModel : public QAbstractListModel, public Object 
{
    Q_OBJECT
public:
    explicit BlueprintModel(CoreEngine* core, QObject *parent = nullptr);

    /// @brief Return data for the specified index with the specified role
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /// @brief Called each time the user edits a cell
    /// @note This is for changing data, not inserting it!
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    /// @brief Return the flags for the item at the given index
    /// @details This indicates which items can be dragged and dropped
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /// @brief Default only checks if:
    /// 1) Data has at least one format in the list of mimeTypes() and if
    /// 2) Action is among the model's supportedDropActions();
    virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

    /// @brief To support additional drop actions, must be reimplemented to accomodate additional  mimeTypes
    /// @note Row and column are -1 when data dropped directly on parent
    /// @param[in] action the action ending a drag and drop operation
    /// @param[in] row the row of an item in the model where the operation ended
    /// @param[in] column the column of an item in the model where the operation ended
    /// @param[in] parent the index of a parent item in the model where the operation ended
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;

    /// @brief Returns the list of supported MIME types
    /// @note Reimplementing mimeData() and dropMimeData() because this is overriden
    virtual QStringList mimeTypes() const override;

    int rowCount(const QModelIndex &parent) const override;

    /// @brief Default is Qt::CopyAction
    Qt::DropActions supportedDropActions() const override;

    /// @brief Implement to support drag and drop operations
    /// @details Remove count rows starting with the given row under parent from the model
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void repopulate();

private:

    Blueprint& blueprint(size_t idx) const;

    CoreEngine* m_engine;
    std::vector<Uuid> m_blueprintIds;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class BlueprintListView
class BlueprintListView : public QListView, public AbstractService {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    BlueprintListView(CoreEngine* engine, const QString& name, QWidget* parent = nullptr);
    ~BlueprintListView();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{
    virtual const char* className() const override { return "BlueprintListView"; }
    virtual const char* namespaceName() const override { return "rev::View::BlueprintListView"; }

    /// @brief Returns True if this AbstractService represents a service
    /// @details This is not a service
    virtual bool isService() const override { return false; };

    /// @brief Returns True if this AbstractService represents a tool.
    /// @details This is not a tool
    virtual bool isTool() const override { return false; };

    /// @}

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

#ifndef QT_NO_CONTEXTMENU
    /// @brief Generates a context menu, overriding default implementation
    /// @note Context menus can be executed either asynchronously using the popup() function or 
    ///       synchronously using the exec() function
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
#endif

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    // Models and delegates aren't deleted by default, so save here and delete with widget
    BlueprintDelegate m_itemDelegate;
    BlueprintModel m_itemModel;

    /// @brief Core engine for the application
    CoreEngine* m_engine;

    /// @}
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // rev

#endif





