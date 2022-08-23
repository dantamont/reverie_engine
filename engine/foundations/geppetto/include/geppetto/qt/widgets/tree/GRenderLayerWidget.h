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
class RenderLayerTreeWidget;

/// @class RenderLayerWidget
class RenderLayerWidget : public JsonWidget{
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    RenderLayerWidget(WidgetManager* wm, const json& sortLayer, QWidget *parent = 0);
    ~RenderLayerWidget();

    /// @}

protected:
    friend class RenderLayerItem;

    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void layoutWidgets() override;

    /// @}

};


/// @class RenderLayerItem
class RenderLayerItem : public TreeItem {
public:
    /// @name Static
    /// @{

    enum RenderLayerItemType {
        kSortingLayer = 2000, // Tree widget item takes a type
    };

    /// @}

    /// @name Constructors and Destructors
    /// @{
    RenderLayerItem(const json& renderLayerJson);
    ~RenderLayerItem();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    virtual void setWidget() override;
    virtual void removeWidget(int column = 0) override;

    /// @brief Get the resource item type of this tree item
    RenderLayerItemType itemType() const { return RenderLayerItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    RenderLayerTreeWidget* renderLayerTreeWidget() const;


    /// @}


protected:
    /// @name Friends
    /// @{

    friend class RenderLayerTreeWidget;

    /// @}

};




/// @class RenderLayerTreeWidget
class RenderLayerTreeWidget : public TreeWidget {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{
    RenderLayerTreeWidget(WidgetManager* wm, QWidget* parent = nullptr);
    ~RenderLayerTreeWidget();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Add sorting layer item to the widget
    void addItem(const json& sortingLayer);

    /// @brief Remove component item from the widget
    void removeItem(RenderLayerItem* RenderLayerItem);

    /// @}

public slots:
    /// @brief Populate the widget
    void repopulate();

protected:

    /// @name Friends
    /// @{

    friend class RenderLayerItem;

    /// @}

    /// @name Protected Methods
    /// @{

    RenderLayerItem* getItem(const json& renderLayerJson);

    RenderLayerItem* currentContextItem() const {
        return static_cast<RenderLayerItem*>(m_currentItems[kContextClick]);
    }

    /// @brief initialize an item added to the widget
    virtual void initializeItem(QTreeWidgetItem* item) override;

    /// @brief How to reorder with respect to another Sorting Layer
    void reorder(Int32_t layerId, Int32_t otherLayerId, bool before);

    virtual void onDropAbove(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination) override;
    virtual void onDropBelow(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination) override;

    /// @brief Remove an item
    void removeItem(const json& renderJson);

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

    const json& renderLayersJson();

    /// @}

    /// @name Protected Members
    /// @{

    std::multimap<int, json> m_sortedLayers; ///< Map of render layer JSON values, sorted for convenience

    /// @}
};



// End namespaces        
}
