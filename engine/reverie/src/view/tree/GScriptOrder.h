#ifndef GB_SCRIPT_ORDER_WIDGETS_H
#define GB_SCRIPT_ORDER_WIDGETS_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "../../core/GObject.h"
#include "../parameters/GParameterWidgets.h"
#include "../../core/mixins/GLoadable.h"
#include "GTreeWidget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

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
class ScriptJsonWidget : public JsonWidget{
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
    virtual void layoutWidgets() override;

    /// @brief Whether or not the specified object is valid
    virtual bool isValidObject(const QJsonObject& object) override;

    /// @brief What to do prior to reloading serializable
    virtual void preLoad() override;

    /// @brief What do do after reloading serializable
    virtual void postLoad() override;

    /// @brief Serializable object
    SortingLayer* sortingLayer() { return reinterpret_cast<SortingLayer*>(m_serializable); }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    bool m_wasPlaying = true;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ScriptOrderItem
class ScriptOrderItem : public TreeItem<SortingLayer> {
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
    virtual void setWidget() override;
    virtual void removeWidget(int column = 0) override;

    /// @brief Return sorting layer represented by this tree item
    inline SortingLayer* sortingLayer() { return m_object; }

    /// @brief Get the resource item type of this tree item
    ScriptOrderItemType itemType() const { return ScriptOrderItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    View::ScriptOrderTreeWidget* scriptOrderTreeWidget() const;


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{
    virtual const char* className() const override { return "ScriptOrderItem"; }
    virtual const char* namespaceName() const override { return "rev::View:ScriptOrderItem"; }

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
    /// @}

};



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ScriptOrderTreeWidget
class ScriptOrderTreeWidget : public TreeWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ScriptOrderTreeWidget(CoreEngine* engine, QWidget* parent = nullptr);
    ~ScriptOrderTreeWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Add sorting layer item to the widget
    void addItem(SortingLayer* sortingLayer);

    /// @brief Remove component item from the widget
    void removeItem(ScriptOrderItem* ScriptOrderItem);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{

    virtual const char* className() const override { return "ScriptOrderTreeWidget"; }
    virtual const char* namespaceName() const override { return "rev::View::ScriptOrderTreeWidget"; }

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


#ifndef QT_NO_CONTEXTMENU
    /// @brief Generates a context menu, overriding default implementation
    /// @note Context menus can be executed either asynchronously using the popup() function or 
    ///       synchronously using the exec() function
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU


    ScriptOrderItem* currentContextItem() const {
        return static_cast<ScriptOrderItem*>(m_currentItems[kContextClick]);
    }

    /// @brief initialize an item added to the widget
    virtual void initializeItem(QTreeWidgetItem* item) override;

    /// @brief How to reorder with respect to another Sorting Layer
    void reorder(SortingLayer* layer, SortingLayer* otherLayer, bool before);

    virtual void onDropAbove(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination) override;
    virtual void onDropBelow(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination) override;

    /// @brief Remove an item
    void removeItem(SortingLayer* itemObject);

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    std::multimap<int, SortingLayer*> m_sortedLayers;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H