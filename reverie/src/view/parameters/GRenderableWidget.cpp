#include "GRenderableWidget.h"
#include "../style/GFontIcon.h"
#include "../../core/readers/GJsonReader.h"
#include "../../core/components/GModelComponent.h"
#include "../../core/rendering/renderer/GMainRenderer.h"
#include "../../core/utils/GMemoryManager.h"
#include "../../core/scene/GScenario.h"
#include "../tree/GRenderLayerWidget.h"

namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// UniformWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
UniformWidget::UniformWidget(CoreEngine* core,
    Uniform& uniform,
    RenderUniformsWidget* parent) :
    ParameterWidget(core, nullptr),
    m_uniform(uniform),
    m_parent(parent)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void UniformWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void UniformWidget::mousePressEvent(QMouseEvent * event)
{
    if (m_parent) {
        m_parent->setCurrentUniformWidget(this);
    }
    QWidget::mousePressEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void UniformWidget::initializeWidgets()
{
    m_uniformName = new QLineEdit(m_uniform.getName());
    m_uniformName->setToolTip("Uniform name");
    m_uniformValue = new QLineEdit(QString(m_uniform));
    m_uniformValue->setToolTip("Uniform value");
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void UniformWidget::initializeConnections()
{
    // Update uniform name and value
    connect(m_uniformName, &QLineEdit::editingFinished, this,
        [this]() {
        pauseSimulation();

        updateUniform();

        resumeSimulation();
    });
    connect(m_uniformValue, &QLineEdit::editingFinished, this,
        [this]() {
        pauseSimulation();

        updateUniform();

        resumeSimulation();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void UniformWidget::layoutWidgets()
{
    setMaximumHeight(500);
    m_mainLayout = new QHBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_uniformName);
    m_mainLayout->addSpacing(15);
    m_mainLayout->addWidget(m_uniformValue);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void UniformWidget::updateUniform()
{
    m_uniform = Uniform(GString(m_uniformName->text()));

    QString valueStr = m_uniformValue->text();
    QRegularExpression intRegex("(\\d+)");
    QRegularExpression floatRegex("(\\d+\\.\\d+)");
    QRegularExpressionMatch intMatch = intRegex.match(valueStr);
    QRegularExpressionMatch floatMatch = floatRegex.match(valueStr);
    if (intMatch.capturedLength(0) == valueStr.length()) {
        // The entire string is digits
        m_uniform.set<int>(valueStr.toInt());
    }
    else if (floatMatch.capturedLength(0) == valueStr.length()) {
        m_uniform.set<float>(valueStr.toDouble());
    }
    else {
        QJsonValue valueJson = JsonReader::ToJsonObject(valueStr);
        m_uniform.loadFromJson(valueJson);
    }
}




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderUniformsWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderUniformsWidget::RenderUniformsWidget(CoreEngine * core,
    Shadable & renderable,
    QWidget * parent) :
    ParameterWidget(core, parent),
    m_renderable(renderable)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderUniformsWidget::~RenderUniformsWidget()
{
    clearUniforms();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderUniformsWidget::update() {
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderUniformsWidget::initializeWidgets()
{
    // Initialize scroll area
    m_areaWidget = new QWidget();

    m_area = new QScrollArea();
    m_area->setWidget(m_areaWidget);
    m_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // Initialize uniform widgets
    repopulateUniforms();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderUniformsWidget::initializeConnections()
{
    m_addUniform = new QAction(tr("&Add a new uniform"), this);
    m_addUniform->setStatusTip("Create a new uniform to describe this model");
    connect(m_addUniform,
        &QAction::triggered,
        this,
        [this] {
        addUniform();
    });

    m_removeUniform = new QAction(tr("&Delete uniform"), this);
    m_removeUniform->setStatusTip("Delete the selected uniform");
    connect(m_removeUniform,
        &QAction::triggered,
        this,
        [this] {
        removeCurrentUniform();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderUniformsWidget::layoutWidgets()
{
    setMaximumHeight(500);
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_area);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderUniformsWidget::clearUniforms()
{
    for (UniformWidget* w : m_uniformWidgets) {
        delete w;
    }
    m_uniformWidgets.clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderUniformsWidget::repopulateUniforms()
{
    clearUniforms();

    m_uniformWidgets.reserve(m_renderable.uniforms().size());
    for (Uniform& uniform : m_renderable.uniforms()) {
        m_uniformWidgets.push_back(new UniformWidget(m_engine,
            uniform,
            this));
    }

    QVBoxLayout* areaLayout = new QVBoxLayout;
    for (const auto& uniformWidget : m_uniformWidgets) {
        areaLayout->addWidget(uniformWidget);
    }

    // Note cannot add layout later, no matter what, so need to recreate widget
    m_areaWidget = new QWidget();
    m_areaWidget->setLayout(areaLayout);
    m_area->setWidget(m_areaWidget);
    // m_areaWidget->show();
    // m_area->show();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderUniformsWidget::addUniform()
{
    const char* tempName = "temp_name";
    m_renderable.addUniform(Uniform(tempName, 0));
    repopulateUniforms();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderUniformsWidget::removeCurrentUniform()
{
    auto iter = std::find_if(m_renderable.uniforms().begin(), m_renderable.uniforms().end(),
        [&](const Uniform& uniform) {
        return uniform.getName() == m_currentUniformWidget->uniform().getName();
    });

    if (iter == m_renderable.uniforms().end()) {
        throw("Error, uniform not found");
    }

    m_renderable.uniforms().erase(iter);
    repopulateUniforms();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderUniformsWidget::contextMenuEvent(QContextMenuEvent * event)
{
    // Create menu
    QMenu menu(this);

    // If a scene object is selected
    if (!m_currentUniformWidget)
        menu.addAction(m_addUniform);
    else
        menu.addAction(m_removeUniform);

    // Display menu at click location
    menu.exec(event->globalPos());

}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderSettingsWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
RenderSettingsWidget::RenderSettingsWidget(CoreEngine* core,
    RenderSettings& renderSettings,
    QWidget* parent) :
    ParameterWidget(core, parent),
    m_renderSettings(renderSettings)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettingsWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettingsWidget::initializeWidgets()
{
    // Not implemented
    m_blendMode = new QComboBox();
    //m_blendMode->setDisabled(true);

    m_depthMode = new QComboBox();
    m_depthMode->addItems({
        "Always fail depth test",
        "Render if less deep",
        "Render if equal",
        "Render if less deep or equal",
        "Render if more deep",
        "Render if not equal",
        "Render if more deep or equal",
        "Always pass depth test"
        });

    RenderSettings& settings = m_engine->mainRenderer()->renderContext().renderSettings();
    const RenderSetting* depthSetting = m_renderSettings.setting(RenderSettingType::kDepth);
    if (!depthSetting) {
        m_renderSettings.addSetting<DepthSetting>();
        depthSetting = m_renderSettings.setting(RenderSettingType::kDepth);
    }
    int index = (int)static_cast<const DepthSetting*>(depthSetting)->depthPassMode() - GL_NEVER;
    m_depthMode->setCurrentIndex(index);


    m_depthTest = new QCheckBox("Test");
    m_depthTest->setToolTip("Enable/disable depth testing");
    bool depthTestValue = static_cast<const DepthSetting*>(depthSetting)->isTestEnabled();
    m_depthTest->setChecked(depthTestValue);

    m_culledFace = new QComboBox();
    m_culledFace->addItems({
        "Front",
        "Back",
        "Front and Back"
        });
    const RenderSetting* cullSetting = m_renderSettings.setting(RenderSettingType::kCullFace);
    if (!cullSetting) {
        cullSetting = settings.setting(RenderSettingType::kCullFace);
    }
    int culledFace = (int)static_cast<const CullFaceSetting*>(cullSetting)->culledFace();
    int faceIndex;
    if (culledFace == GL_FRONT)
        faceIndex = 0;
    else if (culledFace == GL_BACK)
        faceIndex = 1;
    else
        faceIndex = 2;
    m_culledFace->setCurrentIndex(faceIndex);

    m_cullFace = new QCheckBox(QStringLiteral("Cull Face"));
    m_cullFace->setChecked(static_cast<const CullFaceSetting*>(cullSetting)->cullFace());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettingsWidget::initializeConnections()
{
    connect(m_depthMode, qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int index) {

        int depthMode = getDepthMode(index);

        // TODO: Split testing from depth masking
        m_renderSettings.addSetting<DepthSetting>(
            (bool)m_depthTest->isChecked(),
            (DepthPassMode)depthMode,
            (bool)m_depthTest->isChecked());
    });

    connect(m_depthTest, &QCheckBox::stateChanged,
        this,
        [this](int state) {
        Q_UNUSED(state);
        //QMutexLocker lock(&m_engine->mainRenderer()->drawMutex());
        int depthMode = getDepthMode(m_depthMode->currentIndex());
        m_renderSettings.addSetting<DepthSetting>(
            (bool)m_depthTest->isChecked(),
            (DepthPassMode)depthMode,
            (bool)m_depthTest->isChecked());
    });


    connect(m_culledFace, qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int index) {
        int culledFace = getCulledFace(index);

        m_renderSettings.addSetting<CullFaceSetting>(
            (bool)m_cullFace->isChecked(),
            (CulledFace)culledFace);
    });

    connect(m_cullFace, &QCheckBox::stateChanged,
        this,
        [this](int state) {
        Q_UNUSED(state);
        int culledFace = getCulledFace(m_culledFace->currentIndex());
        m_renderSettings.addSetting<CullFaceSetting>(
            (bool)m_cullFace->isChecked(),
            (CulledFace)culledFace);
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettingsWidget::layoutWidgets()
{
    setMaximumHeight(500);
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);
    //m_mainLayout->addLayout(LabeledLayout("Blend Mode", m_blendMode));

    QHBoxLayout* depthLayout = static_cast<QHBoxLayout*>(LabeledLayout("", m_depthMode));
    depthLayout->addSpacing(15);
    depthLayout->addWidget(m_depthTest);
    m_mainLayout->addLayout(depthLayout);

    QHBoxLayout* cullLayout = static_cast<QHBoxLayout*>(LabeledLayout("", m_culledFace));
    cullLayout->addSpacing(15);
    cullLayout->addWidget(m_cullFace);
    m_mainLayout->addLayout(cullLayout);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
int RenderSettingsWidget::getDepthMode(int modeWidgetIndex) const
{
    int depthMode = GL_NEVER;
    depthMode += modeWidgetIndex;
    return depthMode;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
int RenderSettingsWidget::getCulledFace(int faceWidgetIndex) const
{
    int face;
    if (faceWidgetIndex == 0) {
        face = GL_FRONT;
    }
    else if (faceWidgetIndex == 1) {
        face = GL_BACK;
    }
    else if (faceWidgetIndex == 2) {
        face = GL_FRONT_AND_BACK;
    }
    else {
        throw("Invalid index");
    }
    return face;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettingsWidget::contextMenuEvent(QContextMenuEvent * event)
{
    Q_UNUSED(event);
    logInfo("Context menu event");
}




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// TransparencyTypeWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
TransparencyTypeWidget::TransparencyTypeWidget(CoreEngine* core,
    TransparencyRenderMode& transparencyType,
    QWidget* parent) :
    ParameterWidget(core, parent),
    m_transparencyType(transparencyType)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
TransparencyTypeWidget::~TransparencyTypeWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TransparencyTypeWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TransparencyTypeWidget::initializeWidgets()
{
    m_transparencyTypeWidget = new QComboBox();
    m_transparencyTypeWidget->addItem("Opaque", 0);
    m_transparencyTypeWidget->addItem("Transparent (Normal)", 0);
    m_transparencyTypeWidget->addItem("Transparent (Additive)", 0);
    m_transparencyTypeWidget->addItem("Transparent (Subtractive)", 0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TransparencyTypeWidget::initializeConnections()
{
    connect(m_transparencyTypeWidget, 
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [&](int idx) {

        m_transparencyType = TransparencyRenderMode(idx);

    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TransparencyTypeWidget::layoutWidgets()
{
    setMaximumHeight(500);
    m_mainLayout = new QHBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_transparencyTypeWidget);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // rev