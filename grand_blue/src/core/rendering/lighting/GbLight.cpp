#include "GbLight.h"

#include "../../geometry/GbMatrix.h"
#include "../../components/GbTransformComponent.h"
#include "../../scene/GbSceneObject.h"
#include "../../rendering/shaders/GbShaders.h"
#include "../../rendering/shaders/GbUniformBufferObject.h"
#include "../../rendering/renderer/GbRenderContext.h"

namespace Gb{
//////////////////////////////////////////////////////////////////////////////////////////////////
// Light
//////////////////////////////////////////////////////////////////////////////////////////////////
int Light::CreateLight(RenderContext& context)
{
    // Don't initialize light, is used only for default light component
    Light& light = AddLight(context);
    int idx = light.m_typeIntensityAndIndex[2];
    context.lightingSettings().unbindLightData();
    return idx;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
int Light::CreateLight(RenderContext& context, LightType type)
{
    // Set light attributes
    Light& light = AddLight(context);
    light.m_typeIntensityAndIndex[0] = (int)type;
    light.m_typeIntensityAndIndex[1] = 1;

    // Initialize light in GL
    light.initializeLight(context);
    int idx = light.m_typeIntensityAndIndex[2];
    context.lightingSettings().unbindLightData();
    return idx;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
int Light::CreateLight(RenderContext& context, const Color & color, LightType type)
{
    // Set light attributes
    Light& light = AddLight(context);
    light.m_diffuseColor = color.toVector4g();
    light.m_ambientColor = color.toVector4g();
    light.m_specularColor = color.toVector4g();
    light.m_typeIntensityAndIndex[0] = (int)type;
    light.m_typeIntensityAndIndex[1] = 1;

    // Initialize light in GL
    light.initializeLight(context);
    int idx = light.m_typeIntensityAndIndex[2];
    context.lightingSettings().unbindLightData();
    return idx;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Light::Light():
    m_typeIntensityAndIndex({ (int)kPoint, 1, 0, 0 })
{
    // Don't initialize, this is only for default light component
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Light::~Light()
{
    //clearLight();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Light & Light::AddLight(RenderContext& context)
{
    LightingSettings& ls = context.lightingSettings();

    // Map light buffer
    Light* lights = ls.lightData();

    // Set index and increment light count
    int index;
    if (ls.m_deletedIndices.size() > 0) {
        index = ls.m_deletedIndices.back();
        ls.m_deletedIndices.pop_back();
    }
    else {
        index = ls.m_lightCount;
        ls.m_lightCount++;
    }

    // Add to context
    lights[index] = Light();

    lights[index].m_typeIntensityAndIndex[2] = index;

    return lights[index];
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::clearLight(LightingSettings& ls)
{
    // Was never assigned, so return
    if (m_typeIntensityAndIndex[2] < 0) return;

    // Set intensity of light to zero
    if (UBO::getLightBuffer()) {
        setIntensity(0);
    }

    // Remove from global list of lights
    //s_lights[m_index] = nullptr;

    // Flag index for overwrite in static list
    ls.m_deletedIndices.push_back(m_typeIntensityAndIndex[2]);
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

    // Throw error if there are too many lights
    ls.checkLights();

    // Initialize global UBO uniforms
    UBO::getLightBuffer()->setUniformValue("lightCount", (int)ls.m_lightCount);
    UBO::getLightBuffer()->setUniformValue("lightingModel", (int)ls.m_lightingModel);

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
// Light Settings
//////////////////////////////////////////////////////////////////////////////////////////////////
LightingSettings::LightingSettings(RenderContext& context, size_t maxNumLights) :
    m_context(context),
    m_maxLights(maxNumLights),
    m_lights(context, maxNumLights)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
LightingSettings::~LightingSettings()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightingSettings::clearLights()
{
    m_lightCount = 0;
    //m_lights.clear();
    m_lights.bind();
    m_lights.allocateMemory();
    m_lights.release();
    m_deletedIndices.clear();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Light * LightingSettings::lightData()
{
    return m_lights.map();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightingSettings::unbindLightData()
{
    m_lights.unmap(true);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightingSettings::clearLight(int lightIndex)
{
    Light& light = m_context.lightingSettings().lightData()[lightIndex];
    light.clearLight(*this);
    m_context.lightingSettings().unbindLightData();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightingSettings::checkLights() const
{
    if (m_lightCount > m_maxLights) {
        throw("Error, exceeded max number of allowable lights");
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}