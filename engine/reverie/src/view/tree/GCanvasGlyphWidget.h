#ifndef GB_GLYPH_WIDGET_H
#define GB_GLYPH_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "../../core/GObject.h"
#include "../parameters/GParameterWidgets.h"
#include "../../core/mixins/GLoadable.h"
#include "../../core/canvas/GLabel.h"
#include "../../core/canvas/GIcon.h"
#include "GTreeWidget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

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

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    QComboBox* m_verticalAlignment;
    QComboBox* m_horizontalAlignment;
    //VectorWidget<float, 2>* m_screenOffset;
    TransformWidget* m_transform;
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
        kIcon = 2001,
        kImage = 2002
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
    /// @name rev::Object overrides
    /// @{
    virtual const char* className() const override { return "GlyphItem"; }
    virtual const char* namespaceName() const override { return "rev::View:GlyphItem"; }

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
    /// @name rev::Object overrides
    /// @{

    virtual const char* className() const override { return "CanvasGlyphWidget"; }
    virtual const char* namespaceName() const override { return "rev::View::CanvasGlyphWidget"; }

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

    /// @brief Reparent a glyph item
    void reparent(GlyphItem* item, GlyphItem* newParent);

    /// @brief For parenting glyphs
    virtual void onDropAbove(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination) override;
    virtual void onDropBelow(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination) override;
    virtual void onDropViewport(QDropEvent* event, QTreeWidgetItem* source) override;
    virtual void onDropOn(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination) override;

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

    GlyphWidget* m_selectedGlyphWidget = nullptr;

    CanvasComponent* m_canvas;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H