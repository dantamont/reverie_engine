#include "geppetto/qt/widgets/components/GLightComponentWidget.h"

#include <QMutexLocker>
#include <QMutex>

#include "fortress/constants/GConstants.h"
#include "fortress/json/GJson.h"
#include "fortress/containers/math/GEulerAngles.h"

#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "geppetto/qt/widgets/graphics/GGLWidgetInterface.h"

namespace rev {

LightWidgetBaseInterface::LightWidgetBaseInterface(Uint32_t sceneObjectId)
{
    m_updateLightComponentMessage.setSceneObjectId(sceneObjectId);
}


LightSubWidgetInterface::LightSubWidgetInterface(Uint32_t sceneObjectId, json& lightComponentJson):
    LightWidgetBaseInterface(sceneObjectId),
    m_lightComponentJson(lightComponentJson)
{
    m_updateLightComponentMessage.setSceneObjectId(sceneObjectId);
}

void LightSubWidgetInterface::sendUpdateLightMessage(WidgetManager* wm)
{
    m_updateLightComponentMessage.setJsonBytes(GJson::ToBytes(m_lightComponentJson));
    wm->messageGateway()->copyAndQueueMessageForSend(m_updateLightComponentMessage);
}


LightWidgetInterface::LightWidgetInterface(Uint32_t sceneObjectId, const json& lightComponentJson) :
    LightWidgetBaseInterface(sceneObjectId),
    m_lightComponentJson(lightComponentJson)
{
    m_updateLightComponentMessage.setSceneObjectId(sceneObjectId);
}

void LightWidgetInterface::sendUpdateLightMessage(WidgetManager* wm)
{
    m_updateLightComponentMessage.setJsonBytes(GJson::ToBytes(m_lightComponentJson));
    wm->messageGateway()->copyAndQueueMessageForSend(m_updateLightComponentMessage);
}


LightDiffuseColorWidget::LightDiffuseColorWidget(WidgetManager* wm, 
    json& lightComponentJson,
    Uint32_t sceneObjectId,
    QWidget * parent) :
    ColorWidget(wm, m_cachedColor, parent),
    LightSubWidgetInterface(sceneObjectId, lightComponentJson),
    m_lightComponentJson(lightComponentJson)
{
    m_cachedColor = lightComponentJson["light"]["diffuseColor"];
    setToolTip("Diffuse Color");
}

void LightDiffuseColorWidget::update()
{
    ColorWidget::update();
}

void LightDiffuseColorWidget::updateColor()
{
    ColorWidget::updateColor();
    m_lightComponentJson["light"]["diffuseColor"] = m_cachedColor;
    sendUpdateLightMessage(m_widgetManager);
}



LightAmbientColorWidget::LightAmbientColorWidget(WidgetManager* wm,
    json& lightComponentJson,
    Uint32_t sceneObjectId,
    QWidget * parent) :
    ColorWidget(wm, m_cachedColor, parent),
    LightSubWidgetInterface(sceneObjectId, lightComponentJson),
    m_lightComponentJson(lightComponentJson)
{
    m_cachedColor = lightComponentJson["light"]["ambientColor"];
    setToolTip("Ambient Color");
}

void LightAmbientColorWidget::update()
{
    ColorWidget::update();
}


void LightAmbientColorWidget::updateColor()
{
    ColorWidget::updateColor();
    m_lightComponentJson["light"]["ambientColor"] = m_cachedColor;
    sendUpdateLightMessage(m_widgetManager);
}


LightSpecularColorWidget::LightSpecularColorWidget(WidgetManager* wm,
    json& lightComponentJson,
    Uint32_t sceneObjectId,
    QWidget * parent) :
    ColorWidget(wm, m_cachedColor, parent),
    LightSubWidgetInterface(sceneObjectId, lightComponentJson),
    m_lightComponentJson(lightComponentJson)
{
    m_cachedColor = lightComponentJson["light"]["specularColor"];
    setToolTip("Specular Color");
}

void LightSpecularColorWidget::update()
{
    ColorWidget::update();
}

void LightSpecularColorWidget::updateColor()
{
    ColorWidget::updateColor();
    m_lightComponentJson["light"]["specularColor"] = m_cachedColor;
    sendUpdateLightMessage(m_widgetManager);
}


LightDirectionWidget::LightDirectionWidget(WidgetManager* wm,
    json& lightComponentJson,
    Uint32_t sceneObjectId,
    QWidget * parent) :
    VectorWidget<float, 4>(wm, 
        m_cachedVector,
        parent,
        -1e20,
        1e20,
        10,
        {"x:", "y:", "z:"},
        3),
    LightSubWidgetInterface(sceneObjectId, lightComponentJson),
    m_lightComponentJson(lightComponentJson)
{
    QScreen* screen = QGuiApplication::primaryScreen();
    Float32_t dpiX = screen->logicalDotsPerInchX();

    m_cachedVector = lightComponentJson["light"]["direction"];
    m_lineEdits[0]->setMaximumWidth(0.5 * dpiX);
    m_lineEdits[1]->setMaximumWidth(0.5 * dpiX);
    m_lineEdits[2]->setMaximumWidth(0.5 * dpiX);
}

void LightDirectionWidget::update()
{
    VectorWidget<float, 4>::update();
}

void LightDirectionWidget::updateVectorFromWidget()
{
    VectorWidget::updateVectorFromWidget();
    Vector3 vec3 = Vector3(m_vector);
    if (vec3.size() < 1e-8) vec3[1] = -1;
    m_vector = Vector4(vec3.normalized());

    m_lightComponentJson["light"]["direction"] = m_cachedVector;
    sendUpdateLightMessage(m_widgetManager);
}



LightAttenuationWidget::LightAttenuationWidget(WidgetManager* wm,
    json& lightComponentJson,
    Uint32_t sceneObjectId,
    QWidget * parent) :
    VectorWidget<float, 4>(wm,
        m_cachedVector,
        parent,
        -1e20,
        1e20,
        10,
        { "", "", "" },
        3),
    LightSubWidgetInterface(sceneObjectId, lightComponentJson),
    m_lightComponentJson(lightComponentJson)
{
    /// @todo Replace with Viewport::ScreenDPI calls
    QScreen* screen = QGuiApplication::primaryScreen();
    Float32_t screenDpiX = screen->logicalDotsPerInchX();

    m_lineEdits[0]->setToolTip("Constant term");
    m_lineEdits[0]->setMaximumWidth(0.5 * screenDpiX);
    m_lineEdits[1]->setToolTip("Linear coefficient");
    m_lineEdits[1]->setMaximumWidth(0.5 * screenDpiX);
    m_lineEdits[2]->setToolTip("Quadratic coefficient");
    m_lineEdits[2]->setMaximumWidth(0.5 * screenDpiX);

    m_cachedVector = lightComponentJson["light"]["attributes"];

}

void LightAttenuationWidget::update()
{
    VectorWidget<float, 4>::update();
}

void LightAttenuationWidget::updateVectorFromWidget()
{
    VectorWidget::updateVectorFromWidget();
    m_lightComponentJson["light"]["attributes"] = m_cachedVector;
    sendUpdateLightMessage(m_widgetManager);
}




LightCutoffWidget::LightCutoffWidget(WidgetManager* wm,
    json& lightComponentJson,
    Uint32_t sceneObjectId,
    QWidget * parent) :
    VectorWidget<float, 4>(wm,
        m_cachedVector,
        parent,
        -1e20,
        1e20,
        10,
        { "In:", "Out:"},
        2),
    LightSubWidgetInterface(sceneObjectId, lightComponentJson),
    m_lightComponentJson(lightComponentJson)
{
    m_lineEdits[0]->setToolTip("Inner cutoff half-angle (degrees)");
    m_lineEdits[1]->setToolTip("Outer cutoff half-angle (degrees)");
    m_lightComponentJson["light"]["attributes"] = m_cachedVector;
}

void LightCutoffWidget::update()
{
    for (size_t i = 0; i < m_numToShow; i++) {
        QLineEdit* lineEdit = m_lineEdits[i];
        if (!lineEdit->hasFocus()) {
            lineEdit->setText(QString::number(acos(m_vector[i]) * Constants::RadToDeg));
        }
    }
}

void LightCutoffWidget::updateVectorFromWidget()
{
    for (size_t i = 0; i < m_numToShow; i++) {
        QLineEdit* lineEdit = m_lineEdits[i];
        m_vector[i] = (float)cos(Constants::DegToRad * 
            lineEdit->text().toDouble());
    }

    m_lightComponentJson["light"]["attributes"] = m_cachedVector;
    sendUpdateLightMessage(m_widgetManager);
}



LightComponentWidget::LightComponentWidget(WidgetManager* wm,
    const json& lightComponentJson, 
    Uint32_t sceneObjectId,
    QWidget *parent) :
    ComponentWidget(wm, lightComponentJson, sceneObjectId, parent),
    LightWidgetInterface(sceneObjectId, lightComponentJson)
{
    m_setLightcomponentTypeMessage.setSceneObjectId(sceneObjectId);
    m_setLightcomponentTypeMessage.setComponentIndex(0);

    m_toggleLightcomponentShadowsMessage.setSceneObjectId(sceneObjectId);
    m_toggleLightcomponentShadowsMessage.setComponentIndex(0);

    initialize();
}

LightComponentWidget::~LightComponentWidget()
{
}

void LightComponentWidget::update(const GRenderSettingsInfoMessage& message)
{
    m_renderSettingsInfoMessage = message;

    bool canAddShadows = canAddShadowsToLight();

    if (!canAddShadows) {
        if (!m_componentJson["castShadows"].get<bool>()) {
            m_castShadows->setDisabled(true);
            m_castShadows->setToolTip("Cannot add more shadows for this light type");
        }
    }
}

bool LightComponentWidget::canAddShadowsToLight() const
{
    /// @todo Don't duplicate this from Light class
    enum LightType : Int32_t {
        kPoint = 0,
        kDirectional,
        kSpot
    };

    bool canAddShadows = true;
    LightType lightType = (LightType)m_componentJson["light"]["lightType"];
    switch (lightType) {
    case LightType::kPoint:
        canAddShadows = m_renderSettingsInfoMessage.getCanAddPointLightShadow();
        break;
    case LightType::kDirectional:
        canAddShadows = m_renderSettingsInfoMessage.getCanAddDirectionalLightShadow();
        break;
    case LightType::kSpot:
        canAddShadows = m_renderSettingsInfoMessage.getCanAddSpotLightShadow();
        break;
    default:
        assert(false && "Invalid light type");
    }

    return canAddShadows;
}

void LightComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    // Light Type
    Int32_t currentLightType = m_lightComponentJson["light"]["lightType"];
    m_lightType = new QComboBox();
    m_lightType->addItem(SAIcon("lightbulb"), "Point Light");
    m_lightType->addItem(SAIcon("lightbulb"), "Directional Light");
    m_lightType->addItem(SAIcon("lightbulb"), "Spot Light");
    m_lightType->setCurrentIndex((int)currentLightType);

    // Whether or not to cast shadows
    m_castShadows = new QCheckBox("Cast Shadows");
    m_castShadows->setToolTip("Whether or not the light casts shadows");
    m_castShadows->setChecked(m_lightComponentJson["castShadows"].get<bool>());

    QScreen* screen = QGuiApplication::primaryScreen();
    Float32_t dpiX = screen->logicalDotsPerInchX();

    // Minimum shadow bias
    double minBias = m_lightComponentJson["bias"];
    m_minBias = new QLineEdit(QString::number(minBias));
    m_minBias->setMaximumWidth(0.75 * dpiX);
    m_minBias->setValidator(new QDoubleValidator(0, 1e20, 9));
    m_minBias->setToolTip("Bias used to reduce moire pattern in shadow casting");

    // Maximum shadow bias
    double maxBias = m_lightComponentJson["maxBias"];
    m_maxBias = new QLineEdit(QString::number(maxBias));
    m_maxBias->setMaximumWidth(0.75 * dpiX);
    m_maxBias->setValidator(new QDoubleValidator(0, 1e20, 9));
    m_maxBias->setToolTip("Max bias used to reduce moire pattern in shadow casting");

    // Near clip plane
    //double nearClip = 0.01f; // Stand-in value
    //if (lightComponent()->getLightType() == Light::kPoint) {
    //    // Retrieve actual near clip value if set
    //    nearClip = lightComponent()->cachedShadow().m_attributesOrLightMatrix(0, 0);
    //}
    //m_nearPlane = new QLineEdit(QString::number(nearClip));
    //m_nearPlane->setMaximumWidth(0.75 * Viewport::ScreenDPIX());
    //m_nearPlane->setValidator(new QDoubleValidator(0, 1e8, 9));
    //m_nearPlane->setToolTip("Near clip plane used for point light shadow casting");

    // Diffuse Color
    m_diffuseColorWidget = new LightDiffuseColorWidget(m_widgetManager, m_lightComponentJson, m_sceneOrObjectId);

    // Ambient Color
    m_ambientColorWidget = new LightAmbientColorWidget(m_widgetManager, m_lightComponentJson, m_sceneOrObjectId);

    // Specular color
    m_specularColorWidget = new LightSpecularColorWidget(m_widgetManager, m_lightComponentJson, m_sceneOrObjectId);

    // Intensity
    double intensity = m_componentJson["light"]["intensity"];
    m_lightIntensity = new QLineEdit(QString::number(intensity));
    m_lightIntensity->setMaximumWidth(0.75 * dpiX);
    m_lightIntensity->setValidator(
        new QDoubleValidator(0, 1e20, 5)
    );

    // Range
    double range = m_componentJson["light"]["range"];
    m_lightRange = new QLineEdit(QString::number(range));
    m_lightRange->setMaximumWidth(0.75 * dpiX);
    m_lightRange->setValidator(
        new QDoubleValidator(0, 1e30, 8)
    );

    // Direction (for directional and spot lights)
    m_lightDirection = new LightDirectionWidget(m_widgetManager, m_lightComponentJson, m_sceneOrObjectId);

    // Attenuations (for point light)
    m_lightAttenuation = new LightAttenuationWidget(m_widgetManager, m_lightComponentJson, m_sceneOrObjectId);

    // Cutoffs (for spot-light)
    m_lightCutoffs = new LightCutoffWidget(m_widgetManager, m_lightComponentJson, m_sceneOrObjectId);

    toggleWidgets();

    // Request render settings to update info in widget
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_requestRenderSettingsInfoMessage);
}

void LightComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Light type
    connect(m_lightType, qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int index) {
        m_lightComponentJson["light"]["lightType"] = index;
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_setLightcomponentTypeMessage);
        toggleWidgets();
    });

    // Light intensity
    connect(m_lightIntensity, &QLineEdit::editingFinished,
        this,
        [this]() {
            double intensity = m_lightIntensity->text().toDouble();
            m_componentJson["light"]["intensity"] = intensity;
            m_updateLightComponentMessage.setUpdateShadows(false);
            sendUpdateLightMessage(m_widgetManager);
        });

    // Light range
    connect(m_lightRange, &QLineEdit::editingFinished,
        this,
        [this]() {
        double range = m_lightRange->text().toDouble();
        m_componentJson["light"]["range"] = range;
        m_updateLightComponentMessage.setUpdateShadows(false);
        sendUpdateLightMessage(m_widgetManager);
    });

    // Whether or not the light casts shadows
    connect(m_castShadows, qOverload<int>(&QCheckBox::stateChanged),
        this,
        [this](int index) {

        bool enabled = bool(index);
        // If can't add a shadow, leave unchecked
        if (!canAddShadowsToLight()) {
            m_castShadows->blockSignals(true);
            m_castShadows->setChecked(false);
            m_castShadows->blockSignals(false);
        }

        m_toggleLightcomponentShadowsMessage.setEnable(enabled);
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_toggleLightcomponentShadowsMessage);
    });

    // Minimum shadow bias
    connect(m_minBias, &QLineEdit::editingFinished,
        this,
        [this]() {
        double bias = m_minBias->text().toDouble();
        m_componentJson["light"]["bias"] = bias;
        m_updateLightComponentMessage.setUpdateShadows(true);
        sendUpdateLightMessage(m_widgetManager);
    });

    // Maximum shadow bias
    connect(m_maxBias, &QLineEdit::editingFinished,
        this,
        [this]() {
        double bias = m_maxBias->text().toDouble();
        m_componentJson["light"]["maxBias"] = bias;
        m_updateLightComponentMessage.setUpdateShadows(true);
        sendUpdateLightMessage(m_widgetManager);
    });

    // Near clip plane
    //connect(m_nearPlane, &QLineEdit::editingFinished,
    //    this,
    //    [this]() {
    //    double nearPlane = m_nearPlane->text().toDouble();
    //    lightComponent()->cachedShadow().setNearClipPlane(nearPlane);
    //    lightComponent()->updateShadowMap(); // Need to update shadow map camera frustum
    //});
}

void LightComponentWidget::layoutWidgets()
{    
    // Create base widget layout
    ComponentWidget::layoutWidgets();
    m_mainLayout->setSpacing(0);

    // Type of light and shadow casting
    auto* typeLayout = LabeledLayout("Type:", m_lightType);
    typeLayout->setSpacing(0);
    typeLayout->addSpacing(10);
    typeLayout->addWidget(m_castShadows);
    m_mainLayout->addLayout(typeLayout);

    // Min/max bias widgets
    auto* biasLayout = LabeledLayout("Bias:", m_minBias);
    biasLayout->setSpacing(0);
    biasLayout->addSpacing(10);
    AddLabel("Max Bias:", biasLayout);
    biasLayout->addWidget(m_maxBias);
    m_mainLayout->addLayout(biasLayout);

    //// Near clip plane
    //auto* nearPlaneLayout = LabeledLayout("Near Clip:", m_nearPlane);
    //nearPlaneLayout->setSpacing(0);
    //m_mainLayout->addLayout(nearPlaneLayout);

    // Intensity/range widgets
    auto* intensityRangeLayout = LabeledLayout("Intensity:", m_lightIntensity);
    intensityRangeLayout->setSpacing(0);
    intensityRangeLayout->addSpacing(10);
    AddLabel("Range:", intensityRangeLayout);
    intensityRangeLayout->addWidget(m_lightRange);
    m_mainLayout->addLayout(intensityRangeLayout);

    // Color widgets
    QBoxLayout* colorLayout = LabeledLayout("Diffuse:", m_diffuseColorWidget);
    AddLabel("Ambient:", colorLayout);
    colorLayout->addWidget(m_ambientColorWidget);
    AddLabel("Specular:", colorLayout);
    colorLayout->addWidget(m_specularColorWidget);
    colorLayout->setSpacing(0);
    m_mainLayout->addLayout(colorLayout);

   
    m_mainLayout->addLayout(LabeledLayout("Direction:", m_lightDirection));
    m_mainLayout->addLayout(LabeledLayout("Attenuation:", m_lightAttenuation));
    m_mainLayout->addLayout(LabeledLayout("Cutoffs:", m_lightCutoffs));

}

void LightComponentWidget::toggleWidgets()
{
    /// @todo Don't duplicate this from Light class
    enum LightType: Int32_t {
        kPoint = 0,
        kDirectional,
        kSpot
    };

    LightType lightType = (LightType)m_lightComponentJson["light"]["lightType"].get<Int32_t>();
    switch (lightType) {
    case LightType::kPoint:
        m_lightCutoffs->setDisabled(true);
        m_lightAttenuation->setDisabled(false);
        m_lightDirection->setDisabled(true);
        break;
    case LightType::kDirectional:
        m_lightCutoffs->setDisabled(true);
        m_lightAttenuation->setDisabled(true);
        m_lightDirection->setDisabled(false);
        break;
    case LightType::kSpot:
        m_lightCutoffs->setDisabled(false);
        m_lightAttenuation->setDisabled(true);
        m_lightDirection->setDisabled(false);
        break;
    }
}


} // rev