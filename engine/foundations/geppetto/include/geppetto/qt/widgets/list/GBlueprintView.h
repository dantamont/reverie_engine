#pragma once

// Standard Includes
#include <vector>

// Qt
#include <QAbstractListModel>
#include <QListView>
#include <QPixmap>
#include <QMimeData>
#include <QStyledItemDelegate>

// Project
#include "fortress/encoding/uuid/GUuid.h"
#include "fortress/types/GString.h"
#include "fortress/types/GLoadable.h"
#include "fortress/types/GNameable.h"

namespace rev {

class UndoCommand;
class WidgetManager;

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

enum class BlueprintModelDragTypes {
    kBlueprint = 0, // From within this widget
    kSceneObject = 1 // From SceneTreeWidget
};

class BlueprintModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit BlueprintModel(WidgetManager* wm, QObject *parent = nullptr);

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


private:

    void requestRepopulate();

    void repopulate(const json& blueprintsJson);

    WidgetManager* m_widgetManager;
    std::vector<Uuid> m_blueprintIds;
};

/// @class BlueprintListView
class BlueprintListView : public QListView, public rev::NameableInterface {
    Q_OBJECT
public:

    /// @name Constructors and Destructors
    /// @{
    BlueprintListView(WidgetManager* wm, const QString& name, QWidget* parent = nullptr);
    ~BlueprintListView();

    /// @}

protected:

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

    /// @name Protected Members
    /// @{

    // Models and delegates aren't deleted by default, so save here and delete with widget
    BlueprintDelegate m_itemDelegate;
    BlueprintModel m_itemModel;

    WidgetManager* m_widgetManager; ///< Widget manager for the application

    /// @}
};


} // rev
