#include "geppetto/qt/widgets/components/GCanvasComponentWidget.h"

#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/types/GRenderableWidget.h"
#include "geppetto/qt/widgets/tree/GCanvasGlyphWidget.h"
#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "geppetto/qt/widgets/GWidgetManager.h"

#include "fortress/json/GJson.h"
#include "fortress/containers/math/GEulerAngles.h"

#include "enums/GBillboardFlagEnum.h"
#include "enums/GCanvasGlyphModeEnum.h"
#include "ripple/network/gateway/GMessageGateway.h"

namespace rev {

CanvasSubWidget::CanvasSubWidget(WidgetManager* wm, json& componentJson, Uint32_t sceneObjectId, QWidget* parent) :
    ParameterWidget(wm, parent),
    m_sceneObjectId(sceneObjectId),
    m_canvasComponentJson(componentJson)
{
}



BillboardFlagsWidget::BillboardFlagsWidget(WidgetManager* wm, json& componentJson, Uint32_t sceneObjectId, QWidget* parent) :
    CanvasSubWidget(wm, componentJson, sceneObjectId, parent)
{
    initialize();
    m_billboardFlagsMessage.setSceneObjectId(sceneObjectId);
}

void BillboardFlagsWidget::update()
{
}

void BillboardFlagsWidget::requestBillboardFlagChange()
{
    EBillboardFlags flags;
    flags.setFlag(EBillboardFlag::eScale, m_checkBoxes[0]->isChecked());
    flags.setFlag(EBillboardFlag::eFaceCamera, m_checkBoxes[1]->isChecked());
    flags.setFlag(EBillboardFlag::eAlwaysOnTop, m_checkBoxes[2]->isChecked());

    m_billboardFlagsMessage.setBillboardFlags(flags);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_billboardFlagsMessage);
}

void BillboardFlagsWidget::initializeWidgets()
{
    m_areaWidget = new QWidget();
    QVBoxLayout* areaLayout = new QVBoxLayout;
    m_checkBoxes.push_back(new QCheckBox("Constant Screen Size"));
    m_checkBoxes.push_back(new QCheckBox("Always Face Camera"));
    m_checkBoxes.push_back(new QCheckBox("Always On Top"));

    m_checkBoxes[0]->setToolTip("Whether or not to preserve screen size on camera zoom.");

    m_checkBoxes[0]->setChecked(m_canvasComponentJson.value("scaleWithDistance", true));
    m_checkBoxes[1]->setChecked(m_canvasComponentJson.value("faceCamera", false));
    m_checkBoxes[2]->setChecked(m_canvasComponentJson.value("onTop", false));

    areaLayout->addWidget(m_checkBoxes[0]);
    areaLayout->addWidget(m_checkBoxes[1]);
    areaLayout->addWidget(m_checkBoxes[2]);
    m_areaWidget->setLayout(areaLayout);

    // Toggle checkboxes based on display mode
    bool enabled;
    if (m_canvasComponentJson["mode"].get<Int32_t>() == (Int32_t)ECanvasGlyphMode::eGui) {
        enabled = false;
    }
    else {
        enabled = true;
    }
    m_checkBoxes[0]->setEnabled(enabled);
    m_checkBoxes[1]->setEnabled(enabled);

    m_area = new QScrollArea();
    m_area->setWidget(m_areaWidget);
}

void BillboardFlagsWidget::initializeConnections()
{
    // Flag options
    connect(m_checkBoxes[0], &QCheckBox::stateChanged,
        this,
        [&](int state) {
        bool checked = state == 0 ? false : true;
        m_canvasComponentJson["scaleWithDistance"] = checked;
        requestBillboardFlagChange();
    });

    connect(m_checkBoxes[1], &QCheckBox::stateChanged,
        this,
        [&](int state) {
        bool checked = state == 0 ? false : true;
        m_canvasComponentJson["faceCamera"] = checked;
        requestBillboardFlagChange();    });

    connect(m_checkBoxes[2], &QCheckBox::stateChanged,
        this,
        [&](int state) {
        bool checked = state == 0 ? false : true;
        m_canvasComponentJson["onTop"] = checked;
        requestBillboardFlagChange();
    });
}

void BillboardFlagsWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_area);
}




CanvasComponentWidget::CanvasComponentWidget(WidgetManager* wm, const json& componentJson, Uint32_t sceneObjectId, QWidget *parent) :
    SceneObjectComponentWidget(wm, componentJson, sceneObjectId, parent)
{
    initialize();
    m_glyphModeMessage.setSceneObjectId(sceneObjectId);
}

CanvasComponentWidget::~CanvasComponentWidget()
{
    delete m_glyphMode;
    WidgetCollector::Instance().deferredDelete(m_glyphs, 1e6/*1sec*/);
}

void CanvasComponentWidget::update()
{
}

void CanvasComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    m_glyphMode = new QComboBox();
    m_glyphMode->addItems({ "Screen-space GUI", "World-space Billboard" });
    m_glyphMode->setCurrentIndex(m_componentJson["mode"].get<Int32_t>());

    m_billboardFlags = new BillboardFlagsWidget(m_widgetManager, m_componentJson, m_sceneOrObjectId, this);

    m_glyphs = new CanvasGlyphWidget(m_widgetManager, m_componentJson, m_sceneOrObjectId);
}

void CanvasComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Glyph mode
    connect(m_glyphMode,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int idx) {

        // Switch between screen-space and billboard modes
        ECanvasGlyphMode mode = ECanvasGlyphMode(idx);

        // Enable or disable glyph flags
        bool enabled;
        if (mode == ECanvasGlyphMode::eGui) {
            enabled = false;
        }
        else {
            enabled = true;
        }

        // TODO: Make glyph controls reflect canvas mode
        m_billboardFlags->m_checkBoxes[0]->setEnabled(enabled);
        m_billboardFlags->m_checkBoxes[1]->setEnabled(enabled);

        // Change the glyph mode in the main application
        m_glyphModeMessage.setGlyphMode(idx);
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_glyphModeMessage);
    });
}

void CanvasComponentWidget::layoutWidgets()
{
    ComponentWidget::layoutWidgets();

    // Glyph mode
    m_mainLayout->addLayout(LabeledLayout("Glyph Mode: ", m_glyphMode));

    // Layout billboard flags
    auto* flagsBox = new QGroupBox("Behavior Flags");
    auto* flagLayout = new QVBoxLayout();
    flagLayout->addWidget(m_billboardFlags);
    flagLayout->setSpacing(0);
    flagsBox->setToolTip("Flags for the behavior of this canvas");
    flagsBox->setLayout(flagLayout);
    m_mainLayout->addWidget(flagsBox);

    // Glyphs widget
    m_mainLayout->addWidget(m_glyphs);

}


} // rev
