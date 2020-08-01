#include "GbLightComponentWidget.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/loop/GbSimLoop.h"
#include "../tree/GbComponentWidget.h"
#include "../../core/resource/GbResourceCache.h"

#include "../../core/scene/GbSceneObject.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/components/GbLightComponent.h"
#include "../../core/components/GbCamera.h"

#include "../../core/rendering/renderer/GbRenderers.h"
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
    ColorWidget(core, light->light().diffuseColor(), parent),
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
    if(m_lightComponent)
        m_lightComponent->light().setDiffuseColor(color());
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
    ColorWidget(core, light->light().ambientColor(), parent),
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
    if(m_lightComponent)
        m_lightComponent->light().setAmbientColor(color());
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// LightSpecularColorWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////
LightSpecularColorWidget::LightSpecularColorWidget(CoreEngine * core,
    LightComponent * light,
    QWidget * parent) :
    ColorWidget(core, light->light().specularColor(), parent),
    m_lightComponent(light)
{
    initialize();
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
        m_lightComponent->light().setSpecularColor(color());
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
        light->light().direction(), 
        parent,
        -1e20,
        1e20,
        10,
        {"x:", "y:", "z:"},
        3),
    m_lightComponent(light)
{
    m_lineEdits[0]->setMaximumWidth(0.5 * Renderer::screenDPIX());
    m_lineEdits[1]->setMaximumWidth(0.5 * Renderer::screenDPIX());
    m_lineEdits[2]->setMaximumWidth(0.5 * Renderer::screenDPIX());
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
    Vector3g vec3 = Vector3g(m_vector);
    if (vec3.size() < 1e-8) vec3[1] = -1;
    m_vector = Vector4g(vec3.normalized());
    if(m_lightComponent)
        m_lightComponent->light().setDirection(m_vector);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// LightAttenuationWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////
LightAttenuationWidget::LightAttenuationWidget(CoreEngine * core,
    LightComponent * light,
    QWidget * parent) :
    VectorWidget<float, 4>(core,
        light->light().attenuations(),
        parent,
        -1e20,
        1e20,
        10,
        { "", "", "" },
        3),
    m_lightComponent(light)
{
    m_lineEdits[0]->setToolTip("Constant term");
    m_lineEdits[0]->setMaximumWidth(0.5 * Renderer::screenDPIX());
    m_lineEdits[1]->setToolTip("Linear coefficient");
    m_lineEdits[1]->setMaximumWidth(0.5 * Renderer::screenDPIX());
    m_lineEdits[2]->setToolTip("Quadratic coefficient");
    m_lineEdits[2]->setMaximumWidth(0.5 * Renderer::screenDPIX());
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
    if(m_lightComponent)
        m_lightComponent->light().setAttributes(m_vector);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// LightCutoffWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////
LightCutoffWidget::LightCutoffWidget(CoreEngine * core,
    LightComponent * light,
    QWidget * parent) :
    VectorWidget<float, 4>(core,
        light->light().cutoffs(),
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
    if(m_lightComponent)
        m_lightComponent->light().setAttributes(m_vector);
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
    Light::LightType currentType = lightComponent()->light().getType();
    m_lightType = new QComboBox();
    m_lightType->addItem(SAIcon("lightbulb"), "Point Light");
    m_lightType->addItem(SAIcon("lightbulb"), "Directional Light");
    m_lightType->addItem(SAIcon("lightbulb"), "Spot Light");
    m_lightType->setCurrentIndex((int)currentType);

    // Diffuse Color
    m_diffuseColorWidget = new LightDiffuseColorWidget(m_engine, lightComponent());

    // Ambient Color
    m_ambientColorWidget = new LightAmbientColorWidget(m_engine, lightComponent());

    // Specular color
    m_specularColorWidget = new LightSpecularColorWidget(m_engine, lightComponent());

    // Intensity
    double intensity = lightComponent()->light().getIntensity();
    m_lightIntensity = new QLineEdit(QString::number(intensity));
    m_lightIntensity->setMaximumWidth(1.0 * Renderer::screenDPIX());
    m_lightIntensity->setValidator(
        new QDoubleValidator(0, 1e20, 5)
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
        lightComponent()->light().setType(Light::LightType(index));
        toggleWidgets();
    });

    // Light intensity
    connect(m_lightIntensity, &QLineEdit::editingFinished,
        this,
        [this]() {
        double intensity = m_lightIntensity->text().toDouble();
        lightComponent()->light().setIntensity(intensity);
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponentWidget::layoutWidgets()
{    
    // Create base widget layout
    ComponentWidget::layoutWidgets();
    m_mainLayout->setSpacing(0);

    auto* typeLayout = LabeledLayout("Type:", m_lightType);
    typeLayout->setSpacing(0);
    m_mainLayout->addLayout(typeLayout);

    auto* intensityLayout = LabeledLayout("Intensity:", m_lightIntensity);
    intensityLayout->setSpacing(0);
    m_mainLayout->addLayout(intensityLayout);

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
    switch (lightComponent()->light().getType()) {
    case Light::kPoint:
        m_lightCutoffs->setDisabled(true);
        m_lightAttenuation->setDisabled(false);
        m_lightDirection->setDisabled(true);
        break;
    case Light::kDirectional:
        m_lightCutoffs->setDisabled(true);
        m_lightAttenuation->setDisabled(true);
        m_lightDirection->setDisabled(false);
        break;
    case Light::kSpot:
        m_lightCutoffs->setDisabled(false);
        m_lightAttenuation->setDisabled(true);
        m_lightDirection->setDisabled(false);
        break;
    }
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // Gb