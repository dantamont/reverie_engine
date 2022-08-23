#pragma once

// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidget.h"
#include "geppetto/qt/widgets/types/GRenderableWidget.h"

#include "ripple/network/messages/GModifyCanvasBillboardFlagsMessage.h"
#include "ripple/network/messages/GModifyCanvasGlyphModeMessage.h"

namespace rev {

class CanvasGlyphWidget;

/// @class CanvasSubWidget
class CanvasSubWidget : public ParameterWidget {
    Q_OBJECT
public:
    CanvasSubWidget(WidgetManager* wm, json& componentJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);

protected:

    Int32_t m_sceneObjectId{ -1 };
    json& m_canvasComponentJson;
};


/// @class BillboardFlagsWidget
class BillboardFlagsWidget : public CanvasSubWidget {
    Q_OBJECT
public:
    BillboardFlagsWidget(WidgetManager* wm, json& componentJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);

    virtual void update() override;

protected:
    friend class CanvasComponentWidget;

    void requestBillboardFlagChange();

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    GModifyCanvasBillboardFlagsMessage m_billboardFlagsMessage;
    QScrollArea* m_area{ nullptr };
    QWidget* m_areaWidget{ nullptr };
    std::vector<QCheckBox*> m_checkBoxes;
};

/// @class CanvasComponentWidget
/// @brief Widget representing a character controller component
class CanvasComponentWidget : public SceneObjectComponentWidget {
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    CanvasComponentWidget(WidgetManager* wm, const json& componentJson, Uint32_t sceneObjectId, QWidget *parent = 0);
    ~CanvasComponentWidget();

    /// @}

    /// @name Public methods
    /// @{

    virtual void update() override;

    /// @}

public slots:
signals:
private slots:
private:
    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Private Members
    /// @{

    GModifyCanvasGlyphModeMessage m_glyphModeMessage;
    QComboBox* m_glyphMode{ nullptr };
    BillboardFlagsWidget* m_billboardFlags{ nullptr };
    CanvasGlyphWidget* m_glyphs{ nullptr };

    /// @}
};


// End namespaces        
}
