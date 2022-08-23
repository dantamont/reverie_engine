#include "core/rendering/lighting/GLight.h"
#include "core/rendering/lighting/GLightSettings.h"

#include "fortress/containers/math/GMatrix.h"
#include "core/components/GTransformComponent.h"
#include "core/scene/GSceneObject.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/buffers/GUniformBufferObject.h"
#include "core/rendering/renderer/GRenderContext.h"

namespace rev{

// Light

Light::Light()
{
    // Don't initialize, this is only for default light component
}

Light::~Light()
{
    //clearLight();
}

void Light::setPosition(const Vector3& position)
{
    m_position = Vector4(position, 1.0F);
}

void Light::setDirection(const Vector3& dir)
{
    m_direction = dir;
}

void Light::setIndex(int idx)
{
    m_typeIndexFlags[1] = idx;
}

void Light::setColor(const Color& color)
{
    setDiffuseColor(color);
    setAmbientColor(color);
    setSpecularColor(color);
}

void Light::setDiffuseColor(const Color& color)
{
    m_diffuseColor = color.toVector<Real_t, 4>();
}

void Light::setAmbientColor(const Color& color)
{
    m_ambientColor = color.toVector<Real_t, 4>();
}

void Light::setSpecularColor(const Color& color)
{
    m_specularColor = color.toVector<Real_t, 4>();
}

void Light::initializeLight(RenderContext& context)
{
    LightingSettings& ls = context.lightingSettings();

    // Set attributes based on type of light
    switch (getType()) {
    case kPoint:
        // Attenuation
        setAttributes({ 1.0f, 0.0f, 0.0f, 0.0f });
        break;
    case kDirectional:
        // No attributes for directional
        break;
    case kSpot:
        // Cutoffs
        // Cutoff is cosine of the half angle of the light (ranges from 0 to 1.0f, aka 0deg to 90deg)
        // e.g. (float)cos(Constants::DEG_TO_RAD * 25.0) 
        setAttributes({1.0f, 0.0f, 0.0f, 0.0f});
        break;
    }

    // Set a default light range for directional and spot lights
    if (getRange() < 0) {
        setRange(100);
    }

    // Throw error if there are too many lights
    ls.checkLights();

    // Initialize global UBO uniforms

    // Update values in UBO
    const std::shared_ptr<LightUbo>& lsBuffer = Ubo::GetLightSettingsBuffer();
    lsBuffer->setBufferUniformValue<ELightBufferUniformName::eLightCount>(ls.lightBufferUniforms().m_lightCount.getData());
    lsBuffer->setBufferUniformValue<ELightBufferUniformName::eLightingModel>(ls.lightBufferUniforms().m_lightingModel.getData());
}

void Light::setRange(Real_t range)
{
    m_moreAttributes[1] = range;
}

void Light::setRangeFromCutoff(Real_t brightnessCutoff)
{
    // Solving qx^2 + L*x + (c - 1/b) = 0
    Real_t b = brightnessCutoff;
    switch (getType()) {
    case LightType::kPoint:
    {
        // Use attenuations to fine range
        Real_t c = m_attributes.z();
        Real_t lin = m_attributes.y();
        Real_t quad = m_attributes.z();
        Real_t range;
        if (quad == 0) {
            if (lin == 0) {
                range = 1e30f;
            }
            else {
                range = (1 - b * c) / (b * lin);
            }
        }
        else {
            if (lin == 0) {
                range = sqrt((1 - b * c) / (b * quad));
            }
            else {
                Real_t sqt = std::sqrt(lin*lin - 4.0 * quad * (c - (1.0 / b)));
                range = (-lin + sqt) / (2.0 * quad);
            }
        }
        setRange(range);
        break;
    }
    // TODO: Implement manual range for directional and spot lights
    case LightType::kDirectional:
    case LightType::kSpot:
        break;
    }
}

void Light::setAttributes(const Vector4& attr) {
    m_attributes = attr; 

    // The brightness cutoff marking the end of a light's range
    if (getType() == LightType::kPoint && getRange() < 0) {
        constexpr Real_t brightnessCutoff = 0.01f;
        setRangeFromCutoff(brightnessCutoff);
    }
}

void Light::setType(LightType type)
{
    m_typeIndexFlags[0] = (int)type;
}

void Light::setIntensity(Real_t brightness)
{
    m_moreAttributes[0] = brightness;
}

void Light::enable() 
{
    flags() |= (size_t)kEnabled;
}

void Light::disable()
{
    flags() &= ~(size_t)kEnabled; // Logical not, then and
}

//void Light::updatePosition(const Vector3g& position) const
//{
//    //Vector4g pos(position);
//    //pos[3] = 1.0f;
//    //QString str = QStringLiteral("lightPositions");
//    //UBO::getLightBuffer()->setUniformSubValue(str, m_index, pos);
//
//}



    
// End namespaces
}