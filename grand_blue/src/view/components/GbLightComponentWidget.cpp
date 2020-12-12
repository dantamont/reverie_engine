#include "GbLightComponentWidget.h"

#include <QMutexLocker>
#include <QMutex>

#include "../../core/GbCoreEngine.h"
#include "../../core/loop/GbSimLoop.h"
#include "../tree/GbComponentWidget.h"
#include "../../core/resource/GbResourceCache.h"

#include "../../core/scene/GbSceneObject.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/components/GbLightComponent.h"
#include "../../core/components/GbCameraComponent.h"
#include "../../core/mixins/GbRenderable.h"

#include "../../core/rendering/renderer/GbMainRenderer.h"

#include "../style/GbFontIcon.h"
#include "../../core/geometry/GbEulerAngles.h"

namespace Gb {
namespace View {

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// LightDiffuseColorWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////
LightDiffuseColorWidget::LightDiffuseColorWidget(CoreEngine * core, 
    LightComponent * light,
    QWidget * parent) :
    ColorWidget(core, light->cachedLight().diffuseColor(), parent),
    m_lightComponent(light)
{
    setToolTip("Diffuse Color");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightDiffuseColorWidget::update()
{
    //QMutexLocker lock(&m_engine->simulationLoop()->userInterfaceMutex());
    ColorWidget::update();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightDiffuseColorWidget::updateColor()
{
    ColorWidget::updateColor();
    if (m_lightComponent) {
        m_lightComponent->cachedLight().setDiffuseColor(color());
        m_lightComponent->updateLight();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightDiffuseColorWidget::clear()
{
    m_lightComponent = nullptr;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// LightAmbientColorWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////
LightAmbientColorWidget::LightAmbientColorWidget(CoreEngine * core,
    LightComponent * light,
    QWidget * parent) :
    ColorWidget(core, light->cachedLight().ambientColor(), parent),
    m_lightComponent(light)
{
    setToolTip("Ambient Color");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightAmbientColorWidget::update()
{
    //QMutexLocker lock(&m_engine->simulationLoop()->userInterfaceMutex());
    ColorWidget::update();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightAmbientColorWidget::clear()
{
    m_lightComponent = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightAmbientColorWidget::updateColor()
{
    ColorWidget::updateColor();
    if (m_lightComponent) {
        m_lightComponent->cachedLight().setAmbientColor(color());
        m_lightComponent->updateLight();
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// LightSpecularColorWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////
LightSpecularColorWidget::LightSpecularColorWidget(CoreEngine * core,
    LightComponent * light,
    QWidget * parent) :
    ColorWidget(core, light->cachedLight().specularColor(), parent),
    m_lightComponent(light)
{
    //initialize();
    setToolTip("Specular Color");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightSpecularColorWidget::update()
{
    //QMutexLocker lock(&m_engine->simulationLoop()->userInterfaceMutex());
    ColorWidget::update();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightSpecularColorWidget::clear()
{
    m_lightComponent = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightSpecularColorWidget::updateColor()
{
    ColorWidget::updateColor();
    if (m_lightComponent) {
        m_lightComponent->cachedLight().setSpecularColor(color());
        m_lightComponent->updateLight();
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// LightDirectionWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////
LightDirectionWidget::LightDirectionWidget(CoreEngine * core,
    LightComponent * light,
    QWidget * parent) :
    VectorWidget<float, 4>(core, 
        light->cachedLight().direction(),
        parent,
        -1e20,
        1e20,
        10,
        {"x:", "y:", "z:"},
        3),
    m_lightComponent(light)
{
    m_lineEdits[0]->setMaximumWidth(0.5 * Renderable::screenDPIX());
    m_lineEdits[1]->setMaximumWidth(0.5 * Renderable::screenDPIX());
    m_lineEdits[2]->setMaximumWidth(0.5 * Renderable::screenDPIX());
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightDirectionWidget::update()
{
    //QMutexLocker lock(&m_engine->simulationLoop()->userInterfaceMutex());
    VectorWidget<float, 4>::update();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightDirectionWidget::clear()
{
    m_lightComponent = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightDirectionWidget::updateVector()
{
    VectorWidget::updateVector();
    Vector3 vec3 = Vector3(m_vector);
    if (vec3.size() < 1e-8) vec3[1] = -1;
    m_vector = Vector4(vec3.normalized());
    if (m_lightComponent) {
        m_lightComponent->cachedLight().setDirection(m_vector);
        m_lightComponent->updateLight();
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// LightAttenuationWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////
LightAttenuationWidget::LightAttenuationWidget(CoreEngine * core,
    LightComponent * light,
    QWidget * parent) :
    VectorWidget<float, 4>(core,
        light->cachedLight().attenuations(),
        parent,
        -1e20,
        1e20,
        10,
        { "", "", "" },
        3),
    m_lightComponent(light)
{
    m_lineEdits[0]->setToolTip("Constant term");
    m_lineEdits[0]->setMaximumWidth(0.5 * Renderable::screenDPIX());
    m_lineEdits[1]->setToolTip("Linear coefficient");
    m_lineEdits[1]->setMaximumWidth(0.5 * Renderable::screenDPIX());
    m_lineEdits[2]->setToolTip("Quadratic coefficient");
    m_lineEdits[2]->setMaximumWidth(0.5 * Renderable::screenDPIX());
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightAttenuationWidget::update()
{
    //QMutexLocker lock(&m_engine->simulationLoop()->userInterfaceMutex());
    VectorWidget<float, 4>::update();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightAttenuationWidget::clear()
{
    m_lightComponent = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightAttenuationWidget::updateVector()
{
    //QMutexLocker lock(&m_engine->simulationLoop()->userInterfaceMutex());
    VectorWidget::updateVector();
    if (m_lightComponent) {
        m_lightComponent->cachedLight().setAttributes(m_vector);
        m_lightComponent->updateLight();
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// LightCutoffWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////
LightCutoffWidget::LightCutoffWidget(CoreEngine * core,
    LightComponent * light,
    QWidget * parent) :
    VectorWidget<float, 4>(core,
        light->cachedLight().cutoffs(),
        parent,
        -1e20,
        1e20,
        10,
        { "In:", "Out:"},
        2),
    m_lightComponent(light)
{
    m_lineEdits[0]->setToolTip("Inner cutoff half-angle (degrees)");
    m_lineEdits[1]->setToolTip("Outer cutoff half-angle (degrees)");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightCutoffWidget::update()
{
    //QMutexLocker lock(&m_engine->simulationLoop()->userInterfaceMutex());
    for (size_t i = 0; i < m_numToShow; i++) {
        QLineEdit* lineEdit = m_lineEdits[i];
        if (!lineEdit->hasFocus()) {
            lineEdit->setText(QString::number(acos(m_vector[i]) * Constants::RAD_TO_DEG));
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightCutoffWidget::clear()
{
    m_lightComponent = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LightCutoffWidget::updateVector()
{
    for (size_t i = 0; i < m_numToShow; i++) {
        QLineEdit* lineEdit = m_lineEdits[i];
        m_vector[i] = (float)cos(Constants::DEG_TO_RAD * 
            lineEdit->text().toDouble());
    }
    if (m_lightComponent) {
        m_lightComponent->cachedLight().setAttributes(m_vector);
        m_lightComponent->updateLight();
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// LightWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////
LightComponentWidget::LightComponentWidget(CoreEngine* core,
    Component* component,
    QWidget *parent) :
    ComponentWidget(core, component, parent)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
LightComponentWidget::~LightComponentWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
LightComponent* LightComponentWidget::lightComponent() const
{
    return static_cast<LightComponent*>(m_component);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    // Light Type
    Light::LightType currentType = lightComponent()->cachedLight().getType();
    m_lightType = new QComboBox();
    m_lightType->addItem(SAIcon("lightbulb"), "Point Light");
    m_lightType->addItem(SAIcon("lightbulb"), "Directional Light");
    m_lightType->addItem(SAIcon("lightbulb"), "Spot Light");
    m_lightType->setCurrentIndex((int)currentType);

    // Whether or not to cast shadows
    m_castShadows = new QCheckBox("Cast Shadows");
    m_castShadows->setToolTip("Whether or not the light casts shadows");
    m_castShadows->setChecked(lightComponent()->castsShadows());
    if (!lightComponent()->lightingSettings().canAddShadow(currentType)) {
        if (!lightComponent()->castsShadows()) {
            m_castShadows->setDisabled(true);
            m_castShadows->setToolTip("Cannot add more shadows for this light type");
        }
    }

    // Minimum shadow bias
    double minBias = lightComponent()->cachedShadow().m_mapIndexBiasesFarClip[1];
    m_minBias = new QLineEdit(QString::number(minBias));
    m_minBias->setMaximumWidth(0.75 * Renderable::screenDPIX());
    m_minBias->setValidator(new QDoubleValidator(0, 1e20, 9));
    m_minBias->setToolTip("Bias used to reduce moire pattern in shadow casting");

    // Maximum shadow bias
    double maxBias = lightComponent()->cachedShadow().m_mapIndexBiasesFarClip[2];
    m_maxBias = new QLineEdit(QString::number(maxBias));
    m_maxBias->setMaximumWidth(0.75 * Renderable::screenDPIX());
    m_maxBias->setValidator(new QDoubleValidator(0, 1e20, 9));
    m_maxBias->setToolTip("Max bias used to reduce moire pattern in shadow casting");

    // Near clip plane
    //double nearClip = 0.01f; // Stand-in value
    //if (lightComponent()->getLightType() == Light::kPoint) {
    //    // Retrieve actual near clip value if set
    //    nearClip = lightComponent()->cachedShadow().m_attributesOrLightMatrix(0, 0);
    //}
    //m_nearPlane = new QLineEdit(QString::number(nearClip));
    //m_nearPlane->setMaximumWidth(0.75 * Renderable::screenDPIX());
    //m_nearPlane->setValidator(new QDoubleValidator(0, 1e8, 9));
    //m_nearPlane->setToolTip("Near clip plane used for point light shadow casting");

    // Diffuse Color
    m_diffuseColorWidget = new LightDiffuseColorWidget(m_engine, lightComponent());

    // Ambient Color
    m_ambientColorWidget = new LightAmbientColorWidget(m_engine, lightComponent());

    // Specular color
    m_specularColorWidget = new LightSpecularColorWidget(m_engine, lightComponent());

    // Intensity
    double intensity = lightComponent()->cachedLight().getIntensity();
    m_lightIntensity = new QLineEdit(QString::number(intensity));
    m_lightIntensity->setMaximumWidth(0.75 * Renderable::screenDPIX());
    m_lightIntensity->setValidator(
        new QDoubleValidator(0, 1e20, 5)
    );

    // Range
    double range = lightComponent()->cachedLight().getRange();
    m_lightRange = new QLineEdit(QString::number(range));
    m_lightRange->setMaximumWidth(0.75 * Renderable::screenDPIX());
    m_lightRange->setValidator(
        new QDoubleValidator(0, 1e30, 8)
    );

    // Direction (for directional and spot lights)
    m_lightDirection = new LightDirectionWidget(m_engine, lightComponent());

    // Attenuations (for point light)
    m_lightAttenuation = new LightAttenuationWidget(m_engine, lightComponent());

    // Cutoffs (for spot-light)
    m_lightCutoffs = new LightCutoffWidget(m_engine, lightComponent());

    toggleWidgets();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Light type
    connect(m_lightType, qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int index) {
        lightComponent()->cachedLight().setType(Light::LightType(index));
        lightComponent()->updateLight();
        toggleWidgets();
    });

    // Light intensity
    connect(m_lightIntensity, &QLineEdit::editingFinished,
        this,
        [this]() {
        double intensity = m_lightIntensity->text().toDouble();
        lightComponent()->cachedLight().setIntensity((real_g)intensity);
        lightComponent()->updateLight();
    });

    // Light range
    connect(m_lightRange, &QLineEdit::editingFinished,
        this,
        [this]() {
        double range = m_lightRange->text().toDouble();
        lightComponent()->cachedLight().setRange((real_g)range);
        lightComponent()->updateLight();
    });

    // Whether or not the light casts shadows
    connect(m_castShadows, qOverload<int>(&QCheckBox::stateChanged),
        this,
        [this](int index) {
        QMutexLocker lock(&m_engine->mainRenderer()->drawMutex());

        if (!m_engine->mainRenderer()->renderContext().isCurrent()) {
            m_engine->mainRenderer()->renderContext().makeCurrent();
        }

        if (index) {
            LightingSettings& settings = m_engine->mainRenderer()->renderContext().lightingSettings();
            if (!settings.canAddShadow(lightComponent()->getLightType())) {
                // If can't add a shadow, leave unchecked
                m_castShadows->blockSignals(true);
                m_castShadows->setChecked(false);
                m_castShadows->blockSignals(false);
            }
            else {
                lightComponent()->enableShadowCasting();
            }
        }
        else {
            // TODO: Configure so that shadow map is preserved on disable
            lightComponent()->disableShadowCasting();
        }
    });

    // Minimum shadow bias
    connect(m_minBias, &QLineEdit::editingFinished,
        this,
        [this]() {
        double bias = m_minBias->text().toDouble();
        lightComponent()->cachedShadow().m_mapIndexBiasesFarClip[1] = bias;
        lightComponent()->updateShadow();
    });

    // Maximum shadow bias
    connect(m_maxBias, &QLineEdit::editingFinished,
        this,
        [this]() {
        double bias = m_maxBias->text().toDouble();
        lightComponent()->cachedShadow().m_mapIndexBiasesFarClip[2] = bias;
        lightComponent()->updateShadow();
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
///////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponentWidget::toggleWidgets()
{
    switch (lightComponent()->cachedLight().getType()) {
    case Light::kPoint:
        m_lightCutoffs->setDisabled(true);
        m_lightAttenuation->setDisabled(false);
        m_lightDirection->setDisabled(true);
        //m_nearPlane->setDisabled(false);
        break;
    case Light::kDirectional:
        m_lightCutoffs->setDisabled(true);
        m_lightAttenuation->setDisabled(true);
        m_lightDirection->setDisabled(false);
        //m_nearPlane->setDisabled(true);
        break;
    case Light::kSpot:
        m_lightCutoffs->setDisabled(false);
        m_lightAttenuation->setDisabled(true);
        m_lightDirection->setDisabled(false);
        //m_nearPlane->setDisabled(true);
        break;
    }
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // Gb