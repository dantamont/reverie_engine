#pragma once

// Qt
#include <QtWidgets>

// Internal
#include "geppetto/qt/widgets/general/GJsonWidget.h"
#include "fortress/types/GLoadable.h"
#include "geppetto/qt/widgets/tree/GTreeWidget.h"

namespace rev {

class ShaderTreeWidget;
class WidgetManager;

/// @class ShaderJsonWidget
class ShaderJsonWidget : public JsonWidget{
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    ShaderJsonWidget(WidgetManager* wm, const json& objectJson, QWidget* parent = 0);
    ~ShaderJsonWidget();

    /// @}

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
};


/// @class ShaderTreeItem
class ShaderTreeItem : public TreeItem {
public:
    /// @name Static
    /// @{

    enum ShaderItemType {
        kShaderPreset = 2000, // Tree widget item takes a type
    };

    /// @}

    /// @name Constructors and Destructors
    /// @{
    ShaderTreeItem(const json& presetJson);
    ~ShaderTreeItem();

    /// @}
    

    /// @name Public Methods
    /// @{

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    virtual void setWidget() override;
    virtual void removeWidget(int column = 0) override;

    /// @brief Get the resource item type of this tree item
    ShaderItemType itemType() const { return ShaderItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    ShaderTreeWidget* shaderTreeWidget() const;

    /// @}


protected:
    /// @name Friends
    /// @{

    friend class ShaderTreeWidget;

    /// @}

};


/// @class ShaderTreeWidget
class ShaderTreeWidget : public TreeWidget {
    Q_OBJECT
public:

    /// @name Constructors and Destructors
    /// @{
    ShaderTreeWidget(WidgetManager* wm, QWidget* parent = nullptr);
    ~ShaderTreeWidget();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Add sorting layer item to the widget
    void addItem(const json& presetJson);

    /// @brief Remove component item from the widget
    void removeItem(ShaderTreeItem* ShaderTreeItem);

    /// @}


public slots:
    /// @brief Populate the widget
    void repopulate();


protected:

    /// @name Friends
    /// @{

    friend class ShaderTreeItem;

    /// @}

    /// @name Protected Methods
    /// @{

    ShaderTreeItem* currentContextItem() const {
        return static_cast<ShaderTreeItem*>(m_currentItems[kContextClick]);
    }

    /// @brief initialize an item added to the widget
    virtual void initializeItem(QTreeWidgetItem* item) override;

    /// @brief Get an item
    ShaderTreeItem* getItem(const json& presetJson);

    /// @brief Remove an item
    void removeItem(const json& presetJson);

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

    void requestRepopulate() const;

    /// @}

    /// @name Protected Members
    /// @{

    /// @}
};


} // rev
