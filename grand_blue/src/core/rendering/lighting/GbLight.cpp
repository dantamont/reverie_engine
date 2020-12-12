#include "GbLight.h"
#include "GbLightSettings.h"

#include "../../geometry/GbMatrix.h"
#include "../../components/GbTransformComponent.h"
#include "../../scene/GbSceneObject.h"
#include "../../rendering/shaders/GbShaders.h"
#include "../../rendering/buffers/GbUniformBufferObject.h"
#include "../../rendering/renderer/GbRenderContext.h"

namespace Gb{
//////////////////////////////////////////////////////////////////////////////////////////////////
// Light
//////////////////////////////////////////////////////////////////////////////////////////////////
Light::Light()
{
    // Don't initialize, this is only for default light component
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Light::~Light()
{
    //clearLight();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
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
    UBO::getLightSettingsBuffer()->setUniformValue("lightCount", (int)ls.m_lightCount);
    UBO::getLightSettingsBuffer()->setUniformValue("lightingModel", (int)ls.m_lightingModel);

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void Light::setRangeFromCutoff(real_g brightnessCutoff)
{
    // Solving qx^2 + L*x + (c - 1/b) = 0
    real_g b = brightnessCutoff;
    switch (getType()) {
    case LightType::kPoint:
    {
        // Use attenuations to fine range
        real_g c = m_attributes.z();
        real_g lin = m_attributes.y();
        real_g quad = m_attributes.z();
        real_g range;
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
                real_g sqt = std::sqrt(lin*lin - 4.0 * quad * (c - (1.0 / b)));
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
////////////////////////////////////////////////////////////////////////////////////////////////////
void Light::setAttributes(const Vector4& attr) {
    m_attributes = attr; 

    // The brightness cutoff marking the end of a light's range
    if (getType() == LightType::kPoint && getRange() < 0) {
        static real_g brightnessCutoff = 0.01f;
        setRangeFromCutoff(brightnessCutoff);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void Light::enable() 
{
    flags() |= (size_t)kEnabled;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void Light::disable()
{
    flags() &= ~(size_t)kEnabled; // Logical not, then and
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//void Light::updatePosition(const Vector3g& position) const
//{
//    //Vector4g pos(position);
//    //pos[3] = 1.0f;
//    //QString str = QStringLiteral("lightPositions");
//    //UBO::getLightBuffer()->setUniformSubValue(str, m_index, pos);
//
//}



//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}