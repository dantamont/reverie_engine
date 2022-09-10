#pragma once

// Standard Includes
#include <vector>

// Qt
#include <QListView>
#include <QAbstractListModel>
#include <QPixmap>
#include <QMimeData>
#include <QStyledItemDelegate> 

// Project
#include "fortress/encoding/uuid/GUuid.h"
#include "fortress/string/GString.h"
#include "fortress/types/GLoadable.h"
#include "fortress/types/GNameable.h"

namespace rev {

class UndoCommand;
class WidgetManager;

/// @class ResourceDelegate
/// @brief For custom display features
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

};

/// @class ResourcesModel
/// @brief Contains data to be used in view
class ResourcesModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit ResourcesModel(WidgetManager* wm, QObject *parent = nullptr);

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

    void requestRepopulate();
    void repopulate();

    void requestResourceIdChange(const Uuid& resourceId, const Uuid& newId);
    void requestResourceNameChange(const Uuid& resourceId, const GString& resourceName);

private:

    WidgetManager* m_widgetManager{ nullptr };
    std::vector<Uuid> m_resourceIDs;
};


/// @class ResourceListView
class ResourceListView : public QListView, public rev::NameableInterface {
public:
    /// @name Constructors and Destructors
    /// @{
    ResourceListView(WidgetManager* wm, const QString& name, QWidget* parent = nullptr);
    ~ResourceListView();

    /// @}

protected:
    /// @name Protected Methods
    /// @{

    /// @brief Initialize the widget
    virtual void initializeWidget();

    /// @}

    /// @name Protected Members
    /// @{

    // Models and delegates aren't deleted by default, so save here and delete with widget
    ResourceDelegate m_itemDelegate;
    ResourcesModel m_itemModel;

    WidgetManager* m_widgetManager{ nullptr }; ///< Widget manager for the application

    /// @}
};

} // rev
