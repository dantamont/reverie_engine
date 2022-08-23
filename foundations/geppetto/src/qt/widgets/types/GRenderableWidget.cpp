#include "geppetto/qt/widgets/types/GRenderableWidget.h"
#include "geppetto/qt/widgets/tree/GRenderLayerWidget.h"
#include "geppetto/qt/style/GFontIcon.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "ripple/network/gateway/GMessageGateway.h"

#include "fortress/json/GJson.h"
#include "fortress/system/memory/GPointerTypes.h"

namespace rev {

//
//
//UniformWidget::UniformWidget(WidgetManager* wm, const json& uniformJson, Uint32_t sceneObjectId, RenderUniformsWidget* parent) :
//    ParameterWidget(wm, nullptr),
//    m_uniformJson(uniformJson),
//    m_parent(parent),
//    m_sceneObjectId(sceneObjectId)
//{
//    initialize();
//}
//
//void UniformWidget::update()
//{
//}
//
//void UniformWidget::update(const GUniformValueMessage& message)
//{
//    /// @todo Leverage uniform QString method, but need to separate out a rendering library for that
//    json uniformJson = GJson::FromBytes(message.getJsonBytes());
//    //Uniform uniform;
//    //from_json(uniform_json, uniform);
//    //m_uniformValue->setText(QString(uniform));
//}
//
//void UniformWidget::mousePressEvent(QMouseEvent * event)
//{
//    if (m_parent) {
//        m_parent->setCurrentUniformWidget(this);
//    }
//    QWidget::mousePressEvent(event);
//}
//
//void UniformWidget::initializeWidgets()
//{
//    m_uniformName = new QLineEdit(m_uniformJson["name"].get_ref<const std::string&>().c_str());
//    m_uniformName->setToolTip("Uniform name");
//    m_uniformValue = new JsonWidget(m_widgetManager, m_uniformJson, { {"sceneObjectId", m_sceneObjectId }, {"isUniformWidget", true } });
//    m_uniformValue->setToolTip("Uniform value");
//}
//
//void UniformWidget::initializeConnections()
//{
//    // Update uniform name and value
//    connect(m_uniformName, &QLineEdit::editingFinished, this, &UniformWidget::requestUpdateUniform);
//}
//
//void UniformWidget::layoutWidgets()
//{
//    setMaximumHeight(500);
//    m_mainLayout = new QHBoxLayout();
//    m_mainLayout->setSpacing(0);
//    m_mainLayout->addWidget(m_uniformName);
//    m_mainLayout->addSpacing(15);
//    m_mainLayout->addWidget(m_uniformValue);
//}
//
//void UniformWidget::requestUpdateUniform()
//{
//    QString valueStr = m_uniformValue->text();
//    QRegularExpression intRegex("(\\d+)");
//    QRegularExpression floatRegex("(\\d+\\.\\d+)");
//    QRegularExpressionMatch intMatch = intRegex.match(valueStr);
//    QRegularExpressionMatch floatMatch = floatRegex.match(valueStr);
//    if (intMatch.capturedLength(0) == valueStr.length()) {
//        // The entire string is digits
//        m_uniform.set<int>(valueStr.toInt());
//    }
//    else if (floatMatch.capturedLength(0) == valueStr.length()) {
//        m_uniform.set<float>(valueStr.toDouble());
//    }
//    else {
//        const json& valueJson = json::parse(valueStr.toStdString());
//        valueJson.get_to(m_uniform);
//    }
//}
//
//
//
//
//
//RenderUniformsWidget::RenderUniformsWidget(CoreEngine * core,
//    Shadable & renderable,
//    QWidget * parent) :
//    ParameterWidget(core, parent),
//    m_renderable(renderable)
//{
//    initialize();
//}
//
//RenderUniformsWidget::~RenderUniformsWidget()
//{
//    clearUniforms();
//}
//
//void RenderUniformsWidget::update() {
//}
//
//void RenderUniformsWidget::initializeWidgets()
//{
//    // Initialize scroll area
//    m_areaWidget = new QWidget();
//
//    m_area = new QScrollArea();
//    m_area->setWidget(m_areaWidget);
//    m_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
//
//    // Initialize uniform widgets
//    repopulateUniforms();
//}
//
//void RenderUniformsWidget::initializeConnections()
//{
//    m_addUniform = new QAction(tr("&Add a new uniform"), this);
//    m_addUniform->setStatusTip("Create a new uniform to describe this model");
//    connect(m_addUniform,
//        &QAction::triggered,
//        this,
//        [this] {
//        addUniform();
//    });
//
//    m_removeUniform = new QAction(tr("&Delete uniform"), this);
//    m_removeUniform->setStatusTip("Delete the selected uniform");
//    connect(m_removeUniform,
//        &QAction::triggered,
//        this,
//        [this] {
//        removeCurrentUniform();
//    });
//}
//
//void RenderUniformsWidget::layoutWidgets()
//{
//    setMaximumHeight(500);
//    m_mainLayout = new QVBoxLayout();
//    m_mainLayout->setSpacing(0);
//    m_mainLayout->addWidget(m_area);
//}
//
//void RenderUniformsWidget::clearUniforms()
//{
//    for (UniformWidget* w : m_uniformWidgets) {
//        delete w;
//    }
//    m_uniformWidgets.clear();
//}
//
//void RenderUniformsWidget::repopulateUniforms()
//{
//    clearUniforms();
//
//    m_uniformWidgets.reserve(m_renderable.uniforms().size());
//    for (Uniform& uniform : m_renderable.uniforms()) {
//        m_uniformWidgets.push_back(new UniformWidget(m_engine,
//            uniform,
//            this));
//    }
//
//    QVBoxLayout* areaLayout = new QVBoxLayout;
//    for (const auto& uniformWidget : m_uniformWidgets) {
//        areaLayout->addWidget(uniformWidget);
//    }
//
//    // Note cannot add layout later, no matter what, so need to recreate widget
//    m_areaWidget = new QWidget();
//    m_areaWidget->setLayout(areaLayout);
//    m_area->setWidget(m_areaWidget);
//    // m_areaWidget->show();
//    // m_area->show();
//}
//
//void RenderUniformsWidget::addUniform()
//{
//    const char* tempName = "temp_name";
//    m_renderable.addUniform(Uniform(tempName, 0));
//    repopulateUniforms();
//}
//
//void RenderUniformsWidget::removeCurrentUniform()
//{
//    auto iter = std::find_if(m_renderable.uniforms().begin(), m_renderable.uniforms().end(),
//        [&](const Uniform& uniform) {
//        return uniform.getName() == m_currentUniformWidget->uniform().getName();
//    });
//
//    if (iter == m_renderable.uniforms().end()) {
//        Logger::Throw("Error, uniform not found");
//    }
//
//    m_renderable.uniforms().erase(iter);
//    repopulateUniforms();
//}
//
//void RenderUniformsWidget::contextMenuEvent(QContextMenuEvent * event)
//{
//    // Create menu
//    QMenu menu(this);
//
//    // If a scene object is selected
//    if (!m_currentUniformWidget)
//        menu.addAction(m_addUniform);
//    else
//        menu.addAction(m_removeUniform);
//
//    // Display menu at click location
//    menu.exec(event->globalPos());
//
//}



TransparencyTypeWidget::TransparencyTypeWidget(WidgetManager* wm, Uint32_t transparencyType, QWidget* parent) :
    ParameterWidget(wm, parent),
    m_transparencyType(transparencyType)
{
    initialize();
}

TransparencyTypeWidget::~TransparencyTypeWidget()
{
}

void TransparencyTypeWidget::update()
{
}

void TransparencyTypeWidget::requestUpdateRenderSettings()
{
    RenderSettingsWidget* parentWidget = dynamic_cast<RenderSettingsWidget*>(parent());
    parentWidget->getTransparencyModeJson() = m_transparencyType;
    parentWidget->requestUpdateRenderSettings();
}

void TransparencyTypeWidget::initializeWidgets()
{
    m_transparencyTypeWidget = new QComboBox();
    m_transparencyTypeWidget->addItem("Opaque", 0);
    m_transparencyTypeWidget->addItem("Transparent (Normal)", 0);
    m_transparencyTypeWidget->addItem("Transparent (Additive)", 0);
    m_transparencyTypeWidget->addItem("Transparent (Subtractive)", 0);
}

void TransparencyTypeWidget::initializeConnections()
{
    connect(m_transparencyTypeWidget,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [&](int idx) {
            m_transparencyType = idx;
            requestUpdateRenderSettings();
        });
}

void TransparencyTypeWidget::layoutWidgets()
{
    setMaximumHeight(500);
    m_mainLayout = new QHBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_transparencyTypeWidget);
}



RenderSettingsWidget::RenderSettingsWidget(WidgetManager* wm, json& settings, RenderSettingsOwnerType ownerType, Uint32_t sceneObjectId, QWidget* parent) :
    ParameterWidget(wm, parent),
    m_renderSettingsJson(settings)
{
    initialize();
    m_updateMessage.setSceneObjectId(sceneObjectId);
    m_updateMessage.setUpdatingModel(true);
}

void RenderSettingsWidget::update()
{
}

void RenderSettingsWidget::requestUpdateRenderSettings()
{
    m_updateMessage.setJsonBytes(GJson::ToBytes(m_renderSettingsJson));
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_updateMessage);
}

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

    if (getDepthModeJson().empty()) {
        /// @todo Add depth setting on click of depth widget
        m_depthMode->setDisabled(true);
    }
    else {
        Int32_t index = getDepthModeJson().get<Int32_t>() - GL_NEVER;
        m_depthMode->setCurrentIndex(index);
    }

    m_depthTest = new QCheckBox("Test");
    m_depthTest->setToolTip("Enable/disable depth testing");
    if (getDepthTestEnabledJson().empty()) {
        /// @todo Add depth setting on click of depth widget
        m_depthTest->setDisabled(true);
    }
    else {
        bool depthTestValue = getDepthTestEnabledJson().get<bool>();
        m_depthTest->setChecked(depthTestValue);
    }

    m_culledFace = new QComboBox();
    m_culledFace->addItems({
        "Front",
        "Back",
        "Front and Back"
        });

    if (getCulledFaceJson().empty()) {
        /// @todo Add depth setting on click of culled face widget
        m_culledFace->setDisabled(true);
    }
    else {
        Int32_t culledFace = getCulledFaceJson().get<Int32_t>();
        int faceIndex;
        if (culledFace == GL_FRONT) {
            faceIndex = 0;
        }
        else if (culledFace == GL_BACK) {
            faceIndex = 1;
        }
        else {
            faceIndex = 2;
        }
        m_culledFace->setCurrentIndex(faceIndex);
    }

    m_cullFace = new QCheckBox(QStringLiteral("Cull Face"));
    if (getCullFaceJson().empty()) {
        /// @todo Add depth setting on click of cull face widget
        m_cullFace->setDisabled(true);
    }
    else {
        m_cullFace->setChecked(getCullFaceJson().get<bool>());
    }
}

void RenderSettingsWidget::initializeConnections()
{
    connect(m_depthMode, qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int index) {

        int depthMode = getDepthMode(index);
        getDepthModeJson() = depthMode;
        requestUpdateRenderSettings();
    });

    connect(m_depthTest, &QCheckBox::stateChanged,
        this,
        [this](int state) {
        Q_UNUSED(state);
        //QMutexLocker lock(&m_engine->openGlRenderer()->drawMutex());
        int depthMode = getDepthMode(m_depthMode->currentIndex());
        getDepthModeJson() = depthMode;
        getDepthTestEnabledJson() = m_depthTest->isChecked();
        requestUpdateRenderSettings();

    });

    connect(m_culledFace, qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int index) {
        int culledFace = getCulledFace(index);
        getCulledFaceJson() = culledFace;
        requestUpdateRenderSettings();
    });

    connect(m_cullFace, &QCheckBox::stateChanged,
        this,
        [this](int state) {
        int culledFace = getCulledFace(m_culledFace->currentIndex());
        getCulledFaceJson() = culledFace;
        bool cullFace = state == 0 ? false : true;
        getCulledFaceJson() = cullFace;
        requestUpdateRenderSettings();
    });
}

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

int RenderSettingsWidget::getDepthMode(int modeWidgetIndex) const
{
    int depthMode = GL_NEVER;
    depthMode += modeWidgetIndex;
    return depthMode;
}

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
        assert(false && "Invalid index");
    }
    return face;
}

json& RenderSettingsWidget::getTransparencyModeJson()
{
    return m_renderSettingsJson["transparencyMode"];
}

json& RenderSettingsWidget::getDepthModeJson()
{
    static json nullSettingJson;
    for (json& setting : m_renderSettingsJson["settings"]) {
        if (setting["type"].get<int>() == 2) {
            // Depth setting type
            return setting["mode"];
        }
    }
    return nullSettingJson;
}

json& RenderSettingsWidget::getDepthTestEnabledJson()
{
    static json nullSettingJson;
    for (json& setting : m_renderSettingsJson["settings"]) {
        if (setting["type"].get<int>() == 2) {
            // Depth setting type
            return setting["test"];
        }
    }
    return nullSettingJson;
}

json& RenderSettingsWidget::getCullFaceJson()
{
    static json nullSettingJson;
    for (json& setting : m_renderSettingsJson["settings"]) {
        if (setting["type"].get<int>() == 0) {
            // Cull face setting type
            return setting["cullFace"];
        }
    }
    return nullSettingJson;
}

json& RenderSettingsWidget::getCulledFaceJson()
{
    static json nullSettingJson;
    for (json& setting : m_renderSettingsJson["settings"]) {
        if (setting["type"].get<int>() == 0) {
            // Cull face setting type
            return setting["culledFace"];
        }
    }
    return nullSettingJson;
}

void RenderSettingsWidget::contextMenuEvent(QContextMenuEvent *)
{
    std::cout << "Context menu event";
}



} // rev