#pragma once

// External
#include "fortress/containers/GSortingLayer.h"

#include "enums/GRenderLayerWidgetModeEnum.h"
#include "ripple/network/messages/GAddSceneObjectRenderLayerMessage.h"
#include "ripple/network/messages/GAddCameraRenderLayerMessage.h"
#include "ripple/network/messages/GRemoveCameraRenderLayerMessage.h"
#include "ripple/network/messages/GRemoveSceneObjectRenderLayerMessage.h"

// Internal
#include "geppetto/qt/widgets/types/GParameterWidgets.h"
#include "geppetto/qt/widgets/tree/GTreeWidget.h"

namespace rev {

/// @class RenderLayerInstanceWidget
/// @brief For adding/removing/modifying render layers from a scene object
class RenderLayerInstanceWidget : public ParameterWidget {
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    RenderLayerInstanceWidget(WidgetManager* wm, const json& sortingLayerJson, Int32_t sceneObjectId, ERenderLayerWidgetMode mode, QWidget *parent = 0);
    ~RenderLayerInstanceWidget();

    /// @}

protected:
    friend class RenderLayerSelectWidget;

    /// @name Protected Methods
    /// @{

    const json& renderLayersJson();

    void addRenderLayer(Uint32_t layerId);
    void removeRenderLayer(Uint32_t layerId);

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;
    /// @}

    /// @name Protected Members
    /// @{

    Int32_t m_sceneObjectId;
    json m_sortingLayerJson; ///< JSON for the sorting layer represented by this widget
    QCheckBox* m_checkBox;
    ERenderLayerWidgetMode m_widgetMode;

    GAddSceneObjectRenderLayerMessage m_addSceneObjectRenderLayerMessage;
    GRemoveSceneObjectRenderLayerMessage m_removeSceneObjectRenderLayerMessage;
    GAddCameraRenderLayerMessage m_addCameraRenderLayerMessage;
    GRemoveCameraRenderLayerMessage m_removeCameraRenderLayerMessage;

    /// @}
};

/// @class RenderLayerSelectItem
class RenderLayerSelectItem : public TreeItem {
public:
    /// @name Static
    /// @{

    enum RenderLayerSelectItemType {
        kSortingLayer = 2000, // Tree widget item takes a type
    };

    /// @}

    /// @name Constructors and Destructors
    /// @{
    RenderLayerSelectItem(const json& sortingLayerJson);
    ~RenderLayerSelectItem();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    virtual void setWidget() override;
    virtual void removeWidget(int column = 0) override;

    /// @brief Get the resource item type of this tree item
    RenderLayerSelectItemType itemType() const { return RenderLayerSelectItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    RenderLayerSelectWidget* renderLayerTreeWidget() const;

    /// @}

protected:
    /// @name Friends
    /// @{

    friend class RenderLayerSelectWidget;

    /// @}

};


/// @class RenderLayerSelectWidget
class RenderLayerSelectWidget : public TreeWidget {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{
    RenderLayerSelectWidget(WidgetManager* wm, Int32_t sceneObjectId, ERenderLayerWidgetMode mode, QWidget* parent = nullptr);
    ~RenderLayerSelectWidget();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Add sorting layer item to the widget
    void addItem(const json& sortingLayerJson);

    /// @}


public slots:

    /// @brief Populate the widget
    void repopulate();

protected:

    /// @name Friends
    /// @{

    friend class RenderLayerSelectItem;

    /// @}

    /// @name Protected Methods
    /// @{

    const json& renderLayersJson();

    RenderLayerSelectItem* currentContextItem() const;

    /// @brief initialize an item added to the widget
    virtual void initializeItem(QTreeWidgetItem* item) override;

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

    /// @brief What to do on item double click
    virtual void onItemDoubleClicked(QTreeWidgetItem *item, int column) override;

    /// @}

    ERenderLayerWidgetMode m_widgetMode;
    Int32_t m_sceneObjectId; ///< ID of the scene object
};


} // rev
