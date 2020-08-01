#ifndef GB_SHADER_PRESET_TREE_WIDGET_H 
#define GB_SHADER_PRESET_TREE_WIDGET_H
///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "../../core/GbObject.h"
#include "../parameters/GbParameterWidgets.h"
#include "../../core/mixins/GbLoadable.h"
#include "GbTreeWidget.h"
#include "../../core/rendering/shaders/GbShaderPreset.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class CoreEngine;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace View {
class ShaderTreeWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ShaderJsonWidget
class ShaderJsonWidget : public JsonWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    ShaderJsonWidget(CoreEngine* core,
        ShaderPreset* preset,
        QWidget *parent = 0);
    ~ShaderJsonWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /// @}

signals:

    void editedShaderPreset(const Uuid& uuid);

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
    ShaderPreset* shaderPreset() { return reinterpret_cast<ShaderPreset*>(m_serializable); }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    bool m_wasPlaying = true;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ShaderTreeItem
class ShaderTreeItem : public TreeItem<ShaderPreset> {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum ShaderItemType {
        kShaderPreset = 2000, // Tree widget item takes a type
    };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ShaderTreeItem(ShaderPreset* layer);
    ~ShaderTreeItem();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    virtual void setWidget() override;
    virtual void removeWidget(int column = 0) override;

    /// @brief Return sorting layer represented by this tree item
    inline ShaderPreset* shaderPreset() { return m_object; }

    /// @brief Get the resource item type of this tree item
    ShaderItemType itemType() const { return ShaderItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    View::ShaderTreeWidget* shaderTreeWidget() const;


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{
    virtual const char* className() const override { return "ShaderTreeItem"; }
    virtual const char* namespaceName() const override { return "Gb::View:ShaderTreeItem"; }

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class ShaderTreeWidget;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{
    /// @}

};



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ShaderTreeWidget
class ShaderTreeWidget : public TreeWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ShaderTreeWidget(CoreEngine* engine, QWidget* parent = nullptr);
    ~ShaderTreeWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Add sorting layer item to the widget
    void addItem(ShaderPreset* ShaderPreset);

    /// @brief Remove component item from the widget
    void removeItem(ShaderTreeItem* ShaderTreeItem);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{

    virtual const char* className() const override { return "ShaderTreeWidget"; }
    virtual const char* namespaceName() const override { return "Gb::View::ShaderTreeWidget"; }

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

    friend class ShaderTreeItem;

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    ShaderTreeItem* currentContextItem() const {
        return static_cast<ShaderTreeItem*>(m_currentItems[kContextClick]);
    }

    /// @brief initialize an item added to the widget
    virtual void initializeItem(QTreeWidgetItem* item) override;

    /// @brief Remove an item
    void removeItem(ShaderPreset* itemObject);

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif





