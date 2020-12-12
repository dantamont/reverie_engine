#include "GbLightComponent.h"

#include "../GbCoreEngine.h"
#include "../geometry/GbMatrix.h"
#include "GbTransformComponent.h"
#include "../scene/GbSceneObject.h"
#include "../scene/GbScene.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/buffers/GbUniformBufferObject.h"
#include "../rendering/lighting/GbLight.h"
#include "../rendering/lighting/GbShadowMap.h"
#include "../rendering/renderer/GbMainRenderer.h"
#include "../rendering/renderer/GbRenderContext.h"

namespace Gb{
//////////////////////////////////////////////////////////////////////////////////////////////////
LightComponent::LightComponent():
    Component(ComponentType::kLight)
{
    m_lightIndex = -1; // dummy construction for Qt metadata system
}
//////////////////////////////////////////////////////////////////////////////////////////////////
LightComponent::LightComponent(const std::shared_ptr<SceneObject>& object, Light::LightType type):
    Component(object, ComponentType::kLight)
{
    // Add light component to scene object
    if (!sceneObject()) {
        throw("Error, light needs a scene object");
    }
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);

    initializeLight(type);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
LightComponent::~LightComponent()
{
    clearLight();

    if (m_shadowMap) {
        disableShadowCasting();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::setLightPosition(const Vector3 & position)
{
    m_cachedLight.setPosition(position);
    updateLight();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
LightingSettings & LightComponent::lightingSettings() const
{
    return m_engine->mainRenderer()->renderContext().lightingSettings();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::updateLight()
{
    // Queue update in light buffer
    lightingSettings().lightBuffers().queueUpdate<Light>(m_cachedLight, m_lightIndex);

    // If light can cast shadows
    if (m_shadowMap) {
        // Whenever light is modified, update it's lightSpaceMatrix
        updateShadowMap();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::updateShadow()
{
    // Queue update in shadow buffer
    lightingSettings().shadowBuffers().queueUpdate<ShadowInfo>(m_cachedShadow, m_lightIndex);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::enableShadowCasting()
{
    RenderContext& context = m_engine->mainRenderer()->renderContext();
    LightingSettings& settings = context.lightingSettings();
    int availableShadowIndex;
    if (!settings.canAddShadow(m_cachedLight.getType(), &availableShadowIndex)) {
        throw("Error, cannot add any more shadows to scenario");
        return;
    }

    if (m_shadowMap) {
        throw("Shadow map should not exist");
    }

    m_castShadows = true;

    size_t mapIndex = availableShadowIndex % NUM_SHADOWS_PER_LIGHT_TYPE;
    m_shadowMap = new ShadowMap(context, this, mapIndex);

    // Mark shadow map as populated in light settings
    settings.addShadow(m_shadowMap, availableShadowIndex, m_lightIndex);

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
    //logWarning("----------------------------------");
    //settings.pointLightMatrixBuffer().unmap(true);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::disableShadowCasting()
{
    m_castShadows = false;

    //RenderContext& context = m_engine->mainRenderer()->renderContext();
    LightingSettings& settings = lightingSettings();

    if (!m_shadowMap && !m_isEnabled) {
        if (m_isEnabled) {
            throw("Error, shadow map should exist");
        }
        else {
            // If there is no shadow map, there is nothing to disable
            return;
        }
    }

    // Free up the shadow map's slot in settings array of shadow maps
    int startIndex = NUM_SHADOWS_PER_LIGHT_TYPE * m_cachedLight.getType();
    int mapIndex = m_cachedShadow.m_mapIndexBiasesFarClip[0] + startIndex;
    settings.removeShadow(m_shadowMap, mapIndex);

    // Delete shadow map
    delete m_shadowMap;
    m_shadowMap = nullptr;

    // Set shadow value in SSBO
    m_cachedShadow.m_mapIndexBiasesFarClip[0] = -1;
    updateShadow();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
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
        enableShadowCasting();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::disable()
{
    Component::disable();
    m_cachedIntensity = m_cachedLight.getIntensity();
    m_cachedLight.setIntensity(0);
    m_cachedLight.disable();
    updateLight();

    //if (m_castShadows) {
    //    disableShadowCasting();
    //}
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue LightComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    object.insert("light", lightAsJson());
    object.insert("castShadows", m_castShadows);

    // Get map index for light type
    object.insert("mapIndex", m_cachedShadow.m_mapIndexBiasesFarClip[0]);
    object.insert("bias", m_cachedShadow.m_mapIndexBiasesFarClip[1]);
    object.insert("maxBias", m_cachedShadow.m_mapIndexBiasesFarClip[2]);
    if (getLightType() == Light::kPoint) {
        object.insert("farClip", m_cachedShadow.farClipPlane());
        object.insert("nearClip", m_cachedShadow.nearClipPlane());
    }

    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    // Get Json as object
    const QJsonObject& object = json.toObject();

    if (!object.contains("light")) {
        loadLightFromJson(json);
    }
    else {
        loadLightFromJson(object["light"]);
    }
    m_cachedIntensity = m_cachedLight.getIntensity();

    // Load shadow casting
    if (object.contains("castShadows")) {
        m_castShadows = object["castShadows"].toBool(false);

        // Get map index for light type
        // This saves indexing headache if NUM_SHADOWS_PER_LIGHT_TYPE changes
        //int startIndex = NUM_SHADOWS_PER_LIGHT_TYPE * m_cachedLight.getType();
        //int typedIndex = object["mapIndex"].toInt(startIndex);
        m_cachedShadow.m_mapIndexBiasesFarClip[0] = object["mapIndex"].toInt(0);
        m_cachedShadow.m_mapIndexBiasesFarClip[1] = object["bias"].toDouble();
        m_cachedShadow.m_mapIndexBiasesFarClip[2] = object["maxBias"].toDouble();

        if (getLightType() == Light::kPoint) {
            m_cachedShadow.setFarClipPlane(object["farClip"].toDouble(10000));
            m_cachedShadow.setNearClipPlane(object["nearClip"].toDouble(1.0));
        }
    }

    Component::loadFromJson(json);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::updateShadowMap()
{
    // TODO: Recreate shadow map if light type changes
    m_shadowMap->updateShadowAttributes(*sceneObject()->scene());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::clearLight()
{
    // Was never assigned
    if (m_lightIndex < 0) {
        throw("Error, light never assigned");
    }

    // Disable cached light, and then send update to buffer queue
    // ----------------------------------------------------------
    LightingSettings& ls = lightingSettings();

    // Set intensity of light to zero
    if (UBO::getLightSettingsBuffer()) {
        m_cachedLight.setIntensity(0);
    }

    // Set as disabled
    m_cachedLight.disable();

    // Flag index for overwrite in static list
    ls.m_deletedIndices.push_back(m_lightIndex);

    // Update light SSBO with disabled light
    lightingSettings().lightBuffers().queueUpdate<Light>(m_cachedLight, m_lightIndex);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::initializeLight(Light::LightType type)
{
    RenderContext& context = m_engine->mainRenderer()->renderContext();

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
    setLightPosition(sceneObject()->transform()->getPosition());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue LightComponent::lightAsJson() const
{
    QJsonObject object;
    const Light& light = m_cachedLight;
    object.insert("intensity", light.getIntensity());
    object.insert("diffuseColor", light.getDiffuseColor().asJson());
    object.insert("ambientColor", light.getAmbientColor().asJson());
    object.insert("specularColor", light.getSpecularColor().asJson());
    object.insert("attributes", light.getAttributes().asJson());
    object.insert("lightType", light.getType());
    object.insert("direction", light.getDirection().asJson());
    object.insert("range", light.getRange());

    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::loadLightFromJson(const QJsonValue & json)
{
    // Get Json as object
    const QJsonObject& object = json.toObject();

    Light& light = m_cachedLight;

    // Set type
    if (object.contains("lightType")) {
        light.setType(Light::LightType(object["lightType"].toInt()));
    }
    else {
        light.setType(Light::LightType::kPoint);
    }

    // Set color
    QJsonValue diffuseColorJson = object.value("diffuseColor");
    Vector4f diffuseColorVec(diffuseColorJson);
    Color diffuseColor(diffuseColorVec);
    light.setDiffuseColor(diffuseColor);

    QJsonValue ambientColorJson = object.value("ambientColor");
    Vector4f ambientColorVec(ambientColorJson);
    Color ambientColor(ambientColorVec);
    light.setAmbientColor(ambientColor);

    QJsonValue specularColorJson = object.value("specularColor");
    Vector4f specularColorVec(specularColorJson);
    Color specularColor(specularColorVec);
    light.setSpecularColor(specularColor);

    // Set direction
    // Used for spot and directional lights
    if (object.contains("direction")) {
        light.setDirection(Vector4(object["direction"]));
    }

    // Set intensity
    light.setIntensity((float)object.value("intensity").toDouble());

    // Set range
    if (object.contains("range")) {
        light.setRange((real_g)object.value("range").toDouble());
    }
    else {
        light.setRange(50);
    }

    // Set attenuation/generic attributes
    //    For directional lights: empty
    //    For point lights: constant, linear, and quadratic attenuations: attenuation = a1 + a2 * distance + a3 * distance^2
    //        Default (1, 0, 0) is no attenuation (infinite light distance)
    //    For spot lights, direction and cutoff
    QJsonArray attribArray = object.value("attributes").toArray();
    Vector4 attribs;
    attribs.loadFromJson(attribArray);
    light.setAttributes(attribs);

    updateLight();
}


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}