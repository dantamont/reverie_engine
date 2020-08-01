#ifndef GB_RENDER_LAYERS_WIDGET_H 
#define GB_RENDER_LAYERS_WIDGET_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project
#include "GbParameterWidgets.h"
#include "../../core/mixins/GbRenderable.h"
#include "../tree/GbTreeWidget.h"
#include "../../core/containers/GbSortingLayer.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RenderLayerInstanceWidget
class RenderLayerInstanceWidget : public ParameterWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    RenderLayerInstanceWidget(CoreEngine* core,
        SortingLayer* sortLayer,
        std::vector<std::weak_ptr<SortingLayer>>& renderLayers,
        QWidget *parent = 0);
    ~RenderLayerInstanceWidget();

    /// @}

protected:
    friend class RenderLayerSelectWidget;
    //---------------------------------------------------------------------------------------
    /// @name Protected Events
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    std::vector<std::shared_ptr<SortingLayer>> renderLayers();
    bool addRenderLayer(const std::shared_ptr<SortingLayer>& layer);
    bool removeRenderLayer(const QString& label);

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    SortingLayer* m_sortingLayer;

    //QComboBox* m_layers;
    QCheckBox* m_checkBox;

    //Renderable& m_renderable;
    std::vector<std::weak_ptr<SortingLayer>>& m_renderLayers;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RenderLayerSelectItem
class RenderLayerSelectItem : public TreeItem<SortingLayer> {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum RenderLayerSelectItemType {
        kSortingLayer = 2000, // Tree widget item takes a type
    };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    RenderLayerSelectItem(SortingLayer* layer);
    ~RenderLayerSelectItem();

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
    RenderLayerSelectItemType itemType() const { return RenderLayerSelectItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    View::RenderLayerSelectWidget* renderLayerTreeWidget() const;


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{
    virtual const char* className() const override { return "RenderLayerSelectItem"; }
    virtual const char* namespaceName() const override { return "Gb::View:RenderLayerSelectItem"; }

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class RenderLayerSelectWidget;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RenderLayerSelectWidget
class RenderLayerSelectWidget : public TreeWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    RenderLayerSelectWidget(CoreEngine* engine, 
        std::vector<std::weak_ptr<SortingLayer>>& renderLayers,
        QWidget* parent = nullptr);
    ~RenderLayerSelectWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Add sorting layer item to the widget
    void addItem(SortingLayer* sortingLayer);

    /// @brief Remove component item from the widget
    void removeItem(RenderLayerSelectItem* RenderLayerItem);

    //Renderable& renderable() { return m_renderable; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{

    virtual const char* className() const override { return "RenderLayerSelectWidget"; }
    virtual const char* namespaceName() const override { return "Gb::View::RenderLayerSelectWidget"; }

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

    friend class RenderLayerSelectItem;

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    RenderLayerSelectItem* currentContextItem() const;

    /// @brief initialize an item added to the widget
    virtual void initializeItem(QTreeWidgetItem* item) override;

    /// @brief Remove an item
    void removeItem(SortingLayer* itemObject);

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

    /// @brief What to do on item double click
    virtual void onItemDoubleClicked(QTreeWidgetItem *item, int column) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Referenced render layers
    std::vector<std::weak_ptr<SortingLayer>>& m_renderLayers;

    /// @brief Sorted render layers for internal use
    std::multimap<int, SortingLayer*> m_sortedLayers;

    /// @}
};




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif