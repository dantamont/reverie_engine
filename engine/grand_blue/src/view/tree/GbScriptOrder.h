#ifndef GB_SCRIPT_ORDER_WIDGETS_H
#define GB_SCRIPT_ORDER_WIDGETS_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "../../core/GbObject.h"
#include "../parameters/GbParameterWidgets.h"
#include "../../core/mixins/GbLoadable.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class CoreEngine;
struct SortingLayer;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace View {
class ScriptOrderTreeWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ScriptJsonWidget
class ScriptJsonWidget : public ParameterWidget{
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    ScriptJsonWidget(CoreEngine* core, 
        SortingLayer* sortLayer, 
        QWidget *parent = 0);
    ~ScriptJsonWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}

signals:

    void editedScriptOrder();

public slots:

protected slots:


protected:
    //---------------------------------------------------------------------------------------
    /// @name Protected Events
    /// @{

    /// @brief Scroll event
    void wheelEvent(QWheelEvent* event) override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    CoreEngine* m_engine;

    QLabel* m_typeLabel;

    /// @brief Serializable object
    SortingLayer* m_sortingLayer;

    QTextEdit* m_textEdit;
    QPushButton* m_confirmButton;
    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ScriptOrderItem
class ScriptOrderItem : public QTreeWidgetItem, public Gb::Object {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum ScriptOrderItemType {
        kSortingLayer = 2000, // Tree widget item takes a type
    };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ScriptOrderItem(SortingLayer* layer);
    ~ScriptOrderItem();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    void setWidget();
    void removeWidget();

    /// @brief Return sorting layer represented by this tree item
    inline SortingLayer* sortingLayer() { return m_sortingLayer; }

    /// @brief Get the resource item type of this tree item
    ScriptOrderItemType itemType() const { return ScriptOrderItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    View::ScriptOrderTreeWidget* scriptOrderTreeWidget() const;


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{
    virtual const char* className() const override { return "ScriptOrderItem"; }
    virtual const char* namespaceName() const override { return "Gb::View:ScriptOrderItem"; }

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class ScriptOrderTreeWidget;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Get the resource item type of the given layer
    ScriptOrderItemType getItemType(SortingLayer* layer);

    /// @brief Initialize the component tree item
    void initializeItem();


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Pointer to the sorting layer corresponding to this tree item
    SortingLayer* m_sortingLayer;

    /// @brief Pointer to widget if this item has one
    QWidget* m_widget;

    /// @}
};



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ScriptOrderTreeWidget
class ScriptOrderTreeWidget : public QTreeWidget, public AbstractService {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ScriptOrderTreeWidget(CoreEngine* engine, const QString& name, QWidget* parent = nullptr);
    ~ScriptOrderTreeWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Add sorting layer item to the widget
    void addItem(SortingLayer* sortingLayer);
    void addItem(View::ScriptOrderItem* item);

    /// @brief Remove component item from the widget
    void removeItem(ScriptOrderItem* ScriptOrderItem);

    /// @brief Get tree item corresponding to the given sorting layer
    View::ScriptOrderItem* getItem(SortingLayer* itemObject);

    /// @brief Resize columns to fit content
    void resizeColumns();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{

    virtual const char* className() const override { return "ScriptOrderTreeWidget"; }
    virtual const char* namespaceName() const override { return "Gb::View::ScriptOrderTreeWidget"; }

    virtual bool isService() const override { return false; };
    virtual bool isTool() const override { return false; };

    /// @}

public slots:
    /// @brief Populate the widget
    void repopulate();


protected slots:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Slots
    /// @{

    /// @brief What to do on item double click
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

    /// @brief What to do on item expanded
    void onItemExpanded(QTreeWidgetItem* item);

    /// @brief What to do on current item change
    //void onCurrentItemChanged(QTreeWidgetItem *item, QTreeWidgetItem *previous);

    /// @}
protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class ScriptOrderItem;

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief How to reorder with respect to another Sorting Layer
    void reorder(SortingLayer* layer,
        SortingLayer* otherLayer,
        bool before);

    /// @brief Override default mouse release event
    void mouseReleaseEvent(QMouseEvent *event) override;

    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

    /// @brief Remove an item
    void removeItem(SortingLayer* itemObject);

    /// @brief Initialize the widget
    void initializeWidget();

#ifndef QT_NO_CONTEXTMENU
    /// @brief Generates a context menu, overriding default implementation
    /// @note Context menus can be executed either asynchronously using the popup() function or 
    ///       synchronously using the exec() function
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    std::multimap<int, SortingLayer*> m_sortedLayers;

    /// @brief Actions performable in this widget
    QAction* m_addScriptLayer;
    QAction* m_removeScriptLayer;

    /// @brief The resource clicked by a right-mouse operation
    ScriptOrderItem* m_currentScriptOrderItem;

    /// @brief Core engine for the application
    CoreEngine* m_engine;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H