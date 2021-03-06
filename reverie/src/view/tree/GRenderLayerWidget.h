#ifndef GB_RENDER_LAYER_WIDGET_H
#define GB_RENDER_LAYER_WIDGET_H


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
class RenderLayerTreeWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RenderLayerWidget
class RenderLayerWidget : public JsonWidget{
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    RenderLayerWidget(CoreEngine* core, 
        SortingLayer* sortLayer, 
        QWidget *parent = 0);
    ~RenderLayerWidget();

    /// @}

signals:
    void editedRenderLayers();

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

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RenderLayerItem
class RenderLayerItem : public TreeItem<SortingLayer> {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum RenderLayerItemType {
        kSortingLayer = 2000, // Tree widget item takes a type
    };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    RenderLayerItem(SortingLayer* layer);
    ~RenderLayerItem();

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
    RenderLayerItemType itemType() const { return RenderLayerItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    View::RenderLayerTreeWidget* renderLayerTreeWidget() const;


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{
    virtual const char* className() const override { return "RenderLayerItem"; }
    virtual const char* namespaceName() const override { return "rev::View:RenderLayerItem"; }

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class RenderLayerTreeWidget;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{
    /// @}

};



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RenderLayerTreeWidget
class RenderLayerTreeWidget : public TreeWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    RenderLayerTreeWidget(CoreEngine* engine, QWidget* parent = nullptr);
    ~RenderLayerTreeWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Add sorting layer item to the widget
    void addItem(SortingLayer* sortingLayer);

    /// @brief Remove component item from the widget
    void removeItem(RenderLayerItem* RenderLayerItem);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{

    virtual const char* className() const override { return "RenderLayerTreeWidget"; }
    virtual const char* namespaceName() const override { return "rev::View::RenderLayerTreeWidget"; }

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

    friend class RenderLayerItem;

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    RenderLayerItem* currentContextItem() const {
        return static_cast<RenderLayerItem*>(m_currentItems[kContextClick]);
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