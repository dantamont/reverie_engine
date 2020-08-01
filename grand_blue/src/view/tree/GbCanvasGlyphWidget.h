#ifndef GB_GLYPH_WIDGET_H
#define GB_GLYPH_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "../../core/GbObject.h"
#include "../parameters/GbParameterWidgets.h"
#include "../../core/mixins/GbLoadable.h"
#include "../../core/canvas/GbLabel.h"
#include "../../core/canvas/GbIcon.h"
#include "GbTreeWidget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class CoreEngine;
class Label;
class Icon;
class CanvasComponent;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace View {

class GlyphWidget;
class CanvasGlyphWidget;
class TransformWidget;
class ColorWidget;
template<typename T, size_t N> class VectorWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class BillboardFlagsWidget
class BillboardFlagsWidget : public ParameterWidget {
    Q_OBJECT
public:
    BillboardFlagsWidget(CoreEngine* core, Glyph* glyph, QWidget* parent = nullptr);

    virtual void update() override;

protected:
    friend class GlyphWidget;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    QScrollArea* m_area;
    QWidget* m_areaWidget;
    std::vector<QCheckBox*> m_checkBoxes;

    Glyph* m_glyph;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class GlyphWidget
class GlyphWidget : public JsonWidget{
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    GlyphWidget(CoreEngine* core,
        Glyph* glyph, 
        QWidget *parent = 0);
    ~GlyphWidget();

    /// @}

signals:
    void editedGlyphs();

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

    /// @brief Serializable object
    Glyph* glyph() { return dynamic_cast<Glyph*>(m_serializable); }

    /// @brief What to do prior to reloading serializable
    virtual void preLoad() override;

    /// @brief What do do after reloading serializable
    virtual void postLoad() override;

    /// @brief Set the glyph's transform given a scene object name
    void setGlyphTransform(const QString& sceneObjectName);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    BillboardFlagsWidget* m_billboardFlags;
    QComboBox* m_glyphMode;
    QComboBox* m_movementMode;
    QComboBox* m_verticalAlignment;
    QComboBox* m_horizontalAlignment;
    VectorWidget<float, 2>* m_screenOffset;
    TransformWidget* m_transform;
    QLineEdit* m_trackedSceneObject;
    //RenderableWidget<Glyph> m_renderableWidget; // Might not want this exposed to user

    bool m_wasPlaying = true;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LabelWidget
class LabelWidget : public GlyphWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    LabelWidget(CoreEngine* core,
        Label* label,
        QWidget *parent = 0);
    ~LabelWidget();

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Protected Events
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @brief Whether or not the specified object is valid
    virtual bool isValidObject(const QJsonObject& object) override;

    /// @brief Serializable object
    Label* label() { return dynamic_cast<Label*>(m_serializable); }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    QTextEdit* m_text;

    QLineEdit* m_fontSize;

    QComboBox* m_fontFace;

    QLineEdit* m_lineMaxSize;
    QLineEdit* m_lineSpacing;

    ColorWidget* m_color;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class IconWidget
class IconWidget : public GlyphWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    IconWidget(CoreEngine* core,
        Icon* icon,
        QWidget *parent = 0);
    ~IconWidget();

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Protected Events
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @brief Whether or not the specified object is valid
    virtual bool isValidObject(const QJsonObject& object) override;

    /// @brief Serializable object
    Icon* icon() { return dynamic_cast<Icon*>(m_serializable); }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    QComboBox* m_fontAwesomeIcon;
    QLineEdit* m_fontSize;
    ColorWidget* m_color;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class GlyphItem
class GlyphItem : public TreeItem<Glyph> {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum GlyphItemType {
        kLabel = 2000, // Tree widget item takes a type
        kIcon = 2001
    };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    GlyphItem(Glyph* glyph, GlyphItemType type = kLabel);
    ~GlyphItem();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    virtual void setWidget() override;
    virtual void removeWidget(int column = 0) override;

    /// @brief Return glyph represented by this tree item
    inline Glyph* glyph() { return m_object; }

    /// @brief Get the resource item type of this tree item
    GlyphItemType itemType() const { return GlyphItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    View::CanvasGlyphWidget* canvasGlyphWidget() const;


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{
    virtual const char* className() const override { return "GlyphItem"; }
    virtual const char* namespaceName() const override { return "Gb::View:GlyphItem"; }

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class CanvasGlyphWidget;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{
    /// @}

};



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CanvasGlyphWidget
class CanvasGlyphWidget : public TreeWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    CanvasGlyphWidget(CoreEngine* engine, 
        CanvasComponent* canvas,
        QWidget* parent = nullptr);
    ~CanvasGlyphWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    CanvasComponent* canvas() const { return m_canvas; }

    /// @brief Add glyph item to the widget
    void addItem(Glyph* glyph);

    /// @brief Remove component item from the widget
    void removeItem(GlyphItem* glyphItem);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{

    virtual const char* className() const override { return "CanvasGlyphWidget"; }
    virtual const char* namespaceName() const override { return "Gb::View::CanvasGlyphWidget"; }

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

    friend class GlyphItem;

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    GlyphItem* currentContextItem() const {
        return static_cast<GlyphItem*>(m_currentItems[kContextClick]);
    }

    /// @brief initialize an item added to the widget
    virtual void initializeItem(QTreeWidgetItem* item) override;

    /// @brief Remove an item
    void removeItem(Glyph* itemObject);

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

    /// @brief What to do on item double click
    virtual void onItemDoubleClicked(QTreeWidgetItem *item, int column) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    CanvasComponent* m_canvas;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H