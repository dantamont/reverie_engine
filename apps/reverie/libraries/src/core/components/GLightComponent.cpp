#include "core/components/GLightComponent.h"

#include "core/GCoreEngine.h"
#include "fortress/containers/math/GMatrix.h"
#include "core/components/GTransformComponent.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/buffers/GUniformBufferObject.h"
#include "core/rendering/lighting/GLight.h"
#include "core/rendering/lighting/GShadowMap.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/renderer/GRenderContext.h"

namespace rev{

LightComponent::LightComponent():
    Component(ComponentType::kLight)
{
    m_lightIndex = -1; // dummy construction for Qt metadata system
}

LightComponent::LightComponent(const std::shared_ptr<SceneObject>& object, Light::LightType type):
    Component(object, ComponentType::kLight)
{
    // Add light component to scene object
    if (!sceneObject()) {
        Logger::Throw("Error, light needs a scene object");
    }
    setSceneObject(sceneObject());
    sceneObject()->setComponent(this);

    initializeLight(type);
}

LightComponent::~LightComponent()
{
    clearLight();

    if (m_shadowMap) {
        disableShadowCasting(true /*Doesn't matter for destruction*/);
    }

    if (m_shadowMap) {
        Logger::Throw("Error, failed to delete shadow map");
    }
}

void LightComponent::setDiffuseColor(const Vector4& color)
{
    m_cachedLight.setDiffuseColor(color);
    updateLight();
}

void LightComponent::setAmbientColor(const Vector4& color)
{
    m_cachedLight.setAmbientColor(color);
    updateLight();
}

void LightComponent::setSpecularColor(const Vector4& color)
{
    m_cachedLight.setSpecularColor(color);
    updateLight();
}

void LightComponent::setIntensity(Float32_t i)
{
    m_cachedLight.setIntensity(i);
    updateLight();
}

void LightComponent::setAttributes(const Vector4& attributes)
{
    m_cachedLight.setAttributes(attributes);
    updateLight();
}

void LightComponent::setRange(Float32_t range)
{
    m_cachedLight.setRange(range);
    updateLight();
}

void LightComponent::setLightType(Light::LightType type)
{
    // Set cached type
    Light::LightType prevType = m_cachedLight.getType();
    m_cachedLight.setType(type);

    // Change shadow map camera type
    if (prevType != type) {
        if (m_shadowMap) {
            RenderContext& context = m_scene->engine()->openGlRenderer()->renderContext();
            context.makeCurrent();

            LightingSettings& settings = context.lightingSettings();

            int availableShadowIndex;
            if (!settings.canAddShadow(m_cachedLight.getType(), &availableShadowIndex)) {
                Logger::Throw("Error, cannot add any more shadows to scenario");
                return;
            }
            size_t mapIndex = availableShadowIndex % NUM_SHADOWS_PER_LIGHT_TYPE;
            m_shadowMap->initializeCamera(context, mapIndex);
        }

        // Queue an update for the light's OpenGL buffer
        updateLight();
    }
}

void LightComponent::setLightPosition(const Vector3 & position)
{
    m_cachedLight.setPosition(position);
    updateLight();
}

LightingSettings & LightComponent::lightingSettings() const
{
    return m_scene->engine()->openGlRenderer()->renderContext().lightingSettings();
}

void LightComponent::updateLight()
{
    // Queue update in light buffer
    lightingSettings().lightBuffers().queueUpdate<Light>(m_cachedLight, m_lightIndex);

    // If light can cast shadows
    if (m_shadowMap) {
        // Whenever light is modified, update it's lightSpaceMatrix
        updateShadowMap();
    }

    // Recreate draw commands for the scene object's shadows
    scene()->recreateAllShadowDrawCommands();
}

void LightComponent::updateShadow()
{
    // Queue update in shadow buffer
    lightingSettings().shadowBuffers().queueUpdate<ShadowInfo>(m_cachedShadow, m_lightIndex);

    // Recreate draw commands for the scene object's shadows
    scene()->recreateAllShadowDrawCommands();
}

void LightComponent::setShadowFarClipPlane(Real_t farClip)
{
    m_cachedShadow.setFarClipPlane(farClip);
    updateShadow();
}

void LightComponent::setShadowLightMatrix(const Matrix4x4& lightMatrix)
{
    m_cachedShadow.m_attributesOrLightMatrix = lightMatrix;
    updateShadow();
}

void LightComponent::enableShadowCasting(bool setStatus)
{
    RenderContext& context = m_scene->engine()->openGlRenderer()->renderContext();
    LightingSettings& settings = context.lightingSettings();
    int availableShadowIndex;
    if (!settings.canAddShadow(m_cachedLight.getType(), &availableShadowIndex)) {
        Logger::Throw("Error, cannot add any more shadows to scenario");
        return;
    }

    if (m_shadowMap) {
        Logger::Throw("Shadow map should not exist");
    }

    if (setStatus) {
        m_castShadows = true;
    }

    size_t mapIndex = availableShadowIndex % NUM_SHADOWS_PER_LIGHT_TYPE;
    m_shadowMap = std::make_unique<ShadowMap>(context, this, mapIndex);

    // Mark shadow map as populated in light settings
    settings.addShadow(m_shadowMap.get(), availableShadowIndex, m_lightIndex);

    // Set shadow value in SSBO to cached values
    m_cachedShadow.m_mapIndexBiasesFarClip[0] = mapIndex;
    updateShadow();

    // Update shadow map
    updateShadowMap();

    //Matrix4x4g* mats = settings.pointLightMatrixBuffer().data<Matrix4x4g>();
    //
    //for (size_t i = 0; i < 30; i++) {
    //    logInfo(mats[i]);
    //}
    //Logger::LogWarning("----------------------------------");
    //settings.pointLightMatrixBuffer().unmap(true);
}

void LightComponent::disableShadowCasting(bool setStatus)
{
    if (setStatus) {
        m_castShadows = false;
    }

    LightingSettings& settings = lightingSettings();
    if (!m_shadowMap && !isEnabled()) {
        return;
    }

    // Free up the shadow map's slot in settings array of shadow maps
    int startIndex = NUM_SHADOWS_PER_LIGHT_TYPE * m_cachedLight.getType();
    int mapIndex = m_cachedShadow.m_mapIndexBiasesFarClip[0] + startIndex;
    settings.removeShadow(m_shadowMap.get(), mapIndex);

    // Reset shadow map
    m_shadowMap = nullptr;

    // Set shadow value in SSBO
    m_cachedShadow.m_mapIndexBiasesFarClip[0] = -1;
    updateShadow();
}

void LightComponent::enable()
{
    Component::enable();
    if (m_cachedIntensity < 0) {
        // Uninitialized, so no need to enable
        return;
    }
    m_cachedLight.setIntensity(m_cachedIntensity);
    m_cachedLight.enable();
    updateLight();

    if (m_castShadows) {
        enableShadowCasting(false);
    }
}

void LightComponent::disable()
{
    Component::disable();
    m_cachedIntensity = m_cachedLight.getIntensity();
    m_cachedLight.setIntensity(0);
    m_cachedLight.disable();
    updateLight();

    if (m_castShadows) {
        disableShadowCasting(false);
    }
}

void to_json(json& orJson, const LightComponent& korObject)
{
    ToJson<Component>(orJson, korObject);
    json lightJson;
    korObject.lightAsJson(lightJson);
    orJson["light"] = lightJson;
    orJson["castShadows"] = korObject.m_castShadows;

    // Get map index for light type
    orJson["mapIndex"] = korObject.m_cachedShadow.m_mapIndexBiasesFarClip[0];
    orJson["bias"] = korObject.m_cachedShadow.m_mapIndexBiasesFarClip[1];
    orJson["maxBias"] = korObject.m_cachedShadow.m_mapIndexBiasesFarClip[2];
    if (korObject.getLightType() == Light::kPoint) {
        orJson["farClip"] = korObject.m_cachedShadow.farClipPlane();
        orJson["nearClip"] = korObject.m_cachedShadow.nearClipPlane();
    }
}

void from_json(const json& korJson, LightComponent& orObject)
{
    if (!korJson.contains("light")) {
        orObject.loadLightFromJson(korJson);
    }
    else {
        orObject.loadLightFromJson(korJson["light"]);
    }
    orObject.m_cachedIntensity = orObject.m_cachedLight.getIntensity();

    // Load shadow casting
    if (korJson.contains("castShadows")) {
        orObject.m_castShadows = korJson.value("castShadows", false);

        // Get map index for light type
        // This saves indexing headache if NUM_SHADOWS_PER_LIGHT_TYPE changes
        //int startIndex = NUM_SHADOWS_PER_LIGHT_TYPE * m_cachedLight.getType();
        //int typedIndex = object["mapIndex"].toInt(startIndex);
        orObject.m_cachedShadow.m_mapIndexBiasesFarClip[0] = korJson.value("mapIndex", 0);
        orObject.m_cachedShadow.m_mapIndexBiasesFarClip[1] = korJson.at("bias").get<Float64_t>();
        orObject.m_cachedShadow.m_mapIndexBiasesFarClip[2] = korJson.at("maxBias").get<Float64_t>();

        if (orObject.getLightType() == Light::kPoint) {
            orObject.m_cachedShadow.setFarClipPlane(korJson.value("farClip", 10000.0));
            orObject.m_cachedShadow.setNearClipPlane(korJson.value("nearClip", 1.0));
        }
    }

    /// @todo See if this can be called before light-specific loading
    FromJson<Component>(korJson, orObject);
}

void LightComponent::updateShadowMap()
{
    // TODO: Recreate shadow map if light type changes
    m_shadowMap->updateShadowAttributes(*sceneObject()->scene());
}

void LightComponent::clearLight()
{
    // Was never assigned
    if (m_lightIndex < 0) {
        Logger::Throw("Error, light never assigned");
    }

    // Disable cached light, and then send update to buffer queue
    // ----------------------------------------------------------
    LightingSettings& ls = lightingSettings();

    // Set intensity of light to zero
    if (Ubo::GetLightSettingsBuffer()) {
        m_cachedLight.setIntensity(0);
    }

    // Set as disabled
    m_cachedLight.disable();

    // Flag index for overwrite in static list
    ls.m_deletedIndices.push_back(m_lightIndex);

    // Update light SSBO with disabled light
    lightingSettings().lightBuffers().queueUpdate<Light>(m_cachedLight, m_lightIndex);
}

void LightComponent::initializeLight(Light::LightType type)
{
    RenderContext& context = m_scene->engine()->openGlRenderer()->renderContext();

    // Reserve index for light
    LightingSettings& ls = lightingSettings();
    m_lightIndex = ls.reserveLightIndex();

    // Update cached light
    m_cachedLight = Light();
    m_cachedLight.setIndex(m_lightIndex);
    m_cachedLight.setType(type);
    m_cachedLight.setIntensity(1);
    m_cachedLight.initializeLight(context);

    // Set position of cached light and send update command to buffer queue
    setLightPosition(sceneObject()->transform().getPosition());
}

void LightComponent::lightAsJson(json& orJson) const
{
    const Light& light = m_cachedLight;
    orJson = json::object();
    orJson["intensity"] = light.getIntensity();
    orJson["diffuseColor"] = light.diffuseColor();
    orJson["ambientColor"] = light.ambientColor();
    orJson["specularColor"] = light.specularColor();
    orJson["attributes"] = light.attributes();
    orJson["lightType"] = light.getType();
    orJson["direction"] = light.direction();
    orJson["range"] = light.getRange();
}

void LightComponent::loadLightFromJson(const nlohmann::json& korJson)
{
    // Get Json as object
    Light& light = m_cachedLight;

    // Set type
    if (korJson.contains("lightType")) {
        light.setType(Light::LightType(korJson["lightType"].get<Int32_t>()));
    }
    else {
        light.setType(Light::LightType::kPoint);
    }

    // Set color
    const json& diffuseColorJson = korJson.at("diffuseColor");
    Vector4f diffuseColorVec(diffuseColorJson);
    Color diffuseColor(diffuseColorVec);
    light.setDiffuseColor(diffuseColor);

    const json& ambientColorJson = korJson.at("ambientColor");
    Vector4f ambientColorVec(ambientColorJson);
    Color ambientColor(ambientColorVec);
    light.setAmbientColor(ambientColor);

    const json& specularColorJson = korJson.at("specularColor");
    Vector4f specularColorVec(specularColorJson);
    Color specularColor(specularColorVec);
    light.setSpecularColor(specularColor);

    // Set direction
    // Used for spot and directional lights
    if (korJson.contains("direction")) {
        light.setDirection(Vector4(korJson["direction"]));
    }

    // Set intensity
    light.setIntensity(korJson.at("intensity").get<Float32_t>());

    // Set range
    if (korJson.contains("range")) {
        light.setRange(korJson.at("range").get<Real_t>());
    }
    else {
        light.setRange(50);
    }

    // Set attenuation/generic attributes
    //    For directional lights: empty
    //    For point lights: constant, linear, and quadratic attenuations: attenuation = a1 + a2 * distance + a3 * distance^2
    //        Default (1, 0, 0) is no attenuation (infinite light distance)
    //    For spot lights, direction and cutoff
    const json& attribArray = korJson.at("attributes");
    Vector4 attribs;
    attribArray.get_to(attribs);
    light.setAttributes(attribs);

    updateLight();
}


    
// End namespaces
}