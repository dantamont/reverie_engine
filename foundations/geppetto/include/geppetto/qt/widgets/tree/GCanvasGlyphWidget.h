#pragma once

// Qt
#include <QtWidgets>

// External
#include "fortress/containers/GColor.h"
#include "fortress/types/GLoadable.h"

// Internal
#include "geppetto/qt/widgets/general/GJsonWidget.h"
#include "geppetto/qt/widgets/tree/GTreeWidget.h"
#include "ripple/network/messages/GModifyGlyphAlignmentMessage.h"
#include "ripple/network/messages/GModifyLabelTextMessage.h"
#include "ripple/network/messages/GModifyLabelFontMessage.h"
#include "ripple/network/messages/GModifyLabelSpacingMessage.h"
#include "ripple/network/messages/GModifyLabelColorMessage.h"
#include "ripple/network/messages/GModifyIconFontSizeMessage.h"
#include "ripple/network/messages/GModifyIconNameMessage.h"
#include "ripple/network/messages/GModifyIconColorMessage.h"
#include "ripple/network/messages/GAddGlyphMessage.h"
#include "ripple/network/messages/GReparentGlyphMessage.h"
#include "ripple/network/messages/GRemoveGlyphMessage.h"
#include "ripple/network/messages/GCanvasComponentDataMessage.h"

namespace rev {

class GlyphWidget;
class CanvasGlyphWidget;
class TransformWidget;
class ColorWidget;
template<typename T, size_t N> class VectorWidget;

/// @class GlyphWidget
class GlyphWidget : public JsonWidgetInterface {
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    GlyphWidget(WidgetManager* wm, const json& glyphJson, Uint32_t sceneObjectId, QWidget *parent = 0);
    ~GlyphWidget();

    /// @}

signals:
    void editedGlyphs();

protected:

    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    void requestAlignmentUpdate(Int32_t vertAlign, Int32_t horAlign);

    /// @}

    /// @name Protected Members
    /// @{

    GModifyGlyphAlignmentMessage m_alignmentMessage; ///< Message to send to set alignments for a glyph
    json m_glyphJson;
    Int32_t m_sceneObjectId{ -1 };
    QComboBox* m_verticalAlignment{ nullptr };
    QComboBox* m_horizontalAlignment{ nullptr };
    TransformWidget* m_transform{ nullptr };
    bool m_wasPlaying = true;

    /// @}
};

/// @class LabelWidget
class LabelWidget : public GlyphWidget {
public:
    /// @name Constructors/Destructor
    /// @{

    LabelWidget(WidgetManager* wm, const json& glyphJson, Uint32_t sceneObjectId, QWidget *parent = 0);
    ~LabelWidget();

    /// @}

protected:

    /// @name Protected Methods
    /// @{

    void requestLabelTextUpdate(const GString& text);
    void requestLabelFontUpdate(double FontSize, const GString& text);
    void requestLabelSpacingUpdate(double maxLineWidth, double verticalLineSpacing);
    void requestLabelColorUpdate(const Color& color);

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Protected Members
    /// @{

    GModifyLabelTextMessage m_textMessage;
    GModifyLabelFontMessage m_fontMessage;
    GModifyLabelSpacingMessage m_spacingMessage;
    GModifyLabelColorMessage m_colorMessage;
    Color m_cachedColor; ///< Color to be used with color widget
    QTextEdit* m_text{ nullptr };
    QLineEdit* m_fontSize{ nullptr };
    QComboBox* m_fontFace{ nullptr };
    QLineEdit* m_lineMaxSize{ nullptr };
    QLineEdit* m_lineSpacing{ nullptr };
    ColorWidget* m_color{ nullptr };

    /// @}
};

/// @class IconWidget
class IconWidget : public GlyphWidget {
public:
    /// @name Constructors/Destructor
    /// @{

    IconWidget(WidgetManager* wm, const json& glyphJson, Uint32_t sceneObjectId, QWidget *parent = 0);
    ~IconWidget();

    /// @}

protected:
    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    void requestIconUpdate(const GString& iconName);
    void requestIconSizeUpdate(double fontSize);
    void requestIconColorUpdate(const Color& color);

    /// @}

    /// @name Protected Members
    /// @{

    GModifyIconNameMessage m_nameMessage;
    GModifyIconFontSizeMessage m_sizeMessage;
    GModifyIconColorMessage m_colorMessage;
    Color m_cachedColor; ///< Color to be used with color widget
    QComboBox* m_fontAwesomeIcon{ nullptr };
    QLineEdit* m_fontSize{ nullptr };
    ColorWidget* m_color{ nullptr };

    /// @}
};

/// @class SpriteWidget
class SpriteWidget : public GlyphWidget {
public:
    /// @name Constructors/Destructor
    /// @{

    SpriteWidget(WidgetManager* wm, const json& glyphJson, Uint32_t sceneObjectId, QWidget* parent = 0);
    ~SpriteWidget();

    /// @}

protected:
    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Protected Members
    /// @{

    JsonWidget* m_jsonWidget{ nullptr };

    /// @}
};


/// @class GlyphItem
class GlyphItem : public TreeItem {
public:
    /// @name Static
    /// @{

    enum GlyphItemType {
        kLabel = 2000, // Tree widget item takes a type
        kIcon = 2001,
        kSprite = 2002
    };

    /// @}

    /// @name Constructors and Destructors
    /// @{
    GlyphItem(const json& glyphJson, GlyphItemType type, Int32_t id);
    ~GlyphItem();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    virtual void setWidget() override;
    virtual void removeWidget(int column = 0) override;

    /// @brief Return glyph JSON represented by this tree item
    inline json& glyphJson() { return m_data.m_data.m_json; }

    /// @brief Get the resource item type of this tree item
    GlyphItemType itemType() const { return GlyphItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    CanvasGlyphWidget* canvasGlyphWidget() const;

    /// @}

protected:
    /// @name Friends
    /// @{

    friend class CanvasGlyphWidget;

    /// @}
};


/// @class CanvasGlyphWidget
class CanvasGlyphWidget : public TreeWidget {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{
    CanvasGlyphWidget(WidgetManager* wm, json& canvasComponentJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);
    ~CanvasGlyphWidget();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Add glyph item to the widget
    void addItem(json& glyphJson);

    /// @brief Remove component item from the widget
    void removeItem(GlyphItem* glyphItem);

    /// @}

public slots:
    /// @brief Populate the widget
    void repopulate();

protected:
    /// @name Friends
    /// @{

    friend class GlyphItem;

    /// @}

    /// @name Protected Methods
    /// @{

    void requestLabel();
    void requestIcon();
    void requestSprite();
    void requestRemoveGlyph(const Uuid& uuid);
    void requestReparentGlyph(const Uuid& glyphId, const Uuid& parentId);

    /// @brief Get item from index of glyph in canvas list
    GlyphItem* getItem(Int32_t glyphIndexInCanvas);

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
    void removeItem(const json& glyphJson);

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

    /// @brief What to do on item double click
    virtual void onItemDoubleClicked(QTreeWidgetItem *item, int column) override;

    /// @}

    /// @name Protected Members
    /// @{

    GAddGlyphMessage m_addMessage;
    GReparentGlyphMessage m_reparentMessage;
    GRemoveGlyphMessage m_removeMessage;
    GlyphWidget* m_selectedGlyphWidget{ nullptr };
    json& m_canvasComponentJson;
    Int32_t m_sceneObjectId{ -1 };

    /// @}
};

// End namespaces        
}
