#pragma once

// Qt
#include <QtWidgets>

// External
#include "fortress/containers/GSortingLayer.h"
#include "fortress/types/GLoadable.h"

// Internal
#include "geppetto/qt/widgets/general/GJsonWidget.h"
#include "geppetto/qt/widgets/tree/GTreeWidget.h"

namespace rev {
class ScriptOrderTreeWidget;

/// @class ScriptJsonWidget
class ScriptJsonWidget : public JsonWidget{
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    ScriptJsonWidget(WidgetManager* wm, const json& sortingLayerJson, QWidget *parent = 0);
    ~ScriptJsonWidget();

    /// @}

signals:

    void editedScriptOrder();

protected:
 
    /// @name Protected Events
    /// @{

    /// @brief Scroll event
    void wheelEvent(QWheelEvent* event) override;

    /// @}

    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Protected Members
    /// @{

    bool m_wasPlaying = true;

    /// @}
};


/// @class ScriptOrderItem
class ScriptOrderItem : public TreeItem {
public:
    /// @name Static
    /// @{

    enum ScriptOrderItemType {
        kSortingLayer = 2000, // Tree widget item takes a type
    };

    /// @}

    /// @name Constructors and Destructors
    /// @{
    ScriptOrderItem(const json& layer);
    ~ScriptOrderItem();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    virtual void setWidget() override;
    virtual void removeWidget(int column = 0) override;

    /// @brief Get the resource item type of this tree item
    ScriptOrderItemType itemType() const { return ScriptOrderItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    ScriptOrderTreeWidget* scriptOrderTreeWidget() const;


    /// @}

protected:
    /// @name Friends
    /// @{

    friend class ScriptOrderTreeWidget;

    /// @}

};


/// @class ScriptOrderTreeWidget
class ScriptOrderTreeWidget : public TreeWidget {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{
    ScriptOrderTreeWidget(WidgetManager* wm, QWidget* parent = nullptr);
    ~ScriptOrderTreeWidget();

    /// @}
    /// @name Public Methods
    /// @{

    /// @brief Add sorting layer item to the widget
    void addItem(const json& sortingLayer);

    /// @brief Remove component item from the widget
    void removeItem(ScriptOrderItem* ScriptOrderItem);

    /// @}

public slots:
    /// @brief Populate the widget
    void repopulate();

protected:
    /// @name Friends
    /// @{

    friend class ScriptOrderItem;

    /// @}

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
    void reorder(Int32_t layerId, Int32_t otherLayerId, bool before);

    virtual void onDropAbove(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination) override;
    virtual void onDropBelow(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination) override;

    ScriptOrderItem* getItem(const json& itemObject);

    /// @brief Remove an item
    void removeItem(const json& itemObject);

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

    /// @}

    /// @name Protected Members
    /// @{

    std::multimap<int, json> m_sortedLayers; ///< Layer JSON, indexed by order

    /// @}
};


// End namespaces        
}
