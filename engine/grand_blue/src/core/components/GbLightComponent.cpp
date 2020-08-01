#include "GbLightComponent.h"

#include "../GbCoreEngine.h"
#include "../geometry/GbMatrix.h"
#include "GbTransformComponent.h"
#include "../scene/GbSceneObject.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/shaders/GbUniformBufferObject.h"
#include "../rendering/lighting/GbLight.h"
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

    RenderContext& context = m_engine->mainRenderer()->renderContext();
    m_lightIndex = Light::CreateLight(context, type);
    Light& l = light();
    const Vector3& position = sceneObject()->transform()->getPosition();
    l.setPosition(position.asReal());
    unbindLightBuffer();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
LightComponent::LightComponent(const std::shared_ptr<SceneObject>& object, const Color& color, Light::LightType type):
    Component(object, ComponentType::kLight)
{
    // Add light component to scene object
    if (!sceneObject()) {
        throw("Error, light needs a scene object");
    }
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);

    RenderContext& context = m_engine->mainRenderer()->renderContext();
    m_lightIndex = Light::CreateLight(context, color, type);
    light().setPosition(sceneObject()->transform()->getPosition().asReal());
    unbindLightBuffer();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
LightComponent::~LightComponent()
{
    RenderContext& context = m_engine->mainRenderer()->renderContext();
    context.lightingSettings().clearLight(m_lightIndex);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Light & LightComponent::light()
{
    RenderContext& context = m_engine->mainRenderer()->renderContext();
    Light* lightData = context.lightingSettings().lightData();
    return lightData[m_lightIndex];
}
//////////////////////////////////////////////////////////////////////////////////////////////////
const Light & LightComponent::light() const
{
    RenderContext& context = m_engine->mainRenderer()->renderContext();
    Light* lightData = context.lightingSettings().lightData();
    return lightData[m_lightIndex];
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::unbindLightBuffer() const
{
    RenderContext& context = m_engine->mainRenderer()->renderContext();
    context.lightingSettings().unbindLightData();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::enable()
{
    Component::enable();
    Light& l = light();
    l.setIntensity(m_cachedIntensity);

    unbindLightBuffer();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::disable()
{
    Component::disable();
    Light& l = light();
    m_cachedIntensity = l.getIntensity();
    l.setIntensity(0);
    unbindLightBuffer();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue LightComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    object.insert("light", lightAsJson());

    unbindLightBuffer();
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::loadFromJson(const QJsonValue & json)
{
    Component::loadFromJson(json);

    // Get Json as object
    const QJsonObject& object = json.toObject();

    if (!object.contains("light")) {
        loadLightFromJson(json);
    }
    else {
        loadLightFromJson(object["light"]);
    }

}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue LightComponent::lightAsJson() const
{
    QJsonObject object;
    const Light& light = LightComponent::light();
    object.insert("intensity", light.getIntensity());
    object.insert("diffuseColor", light.getDiffuseColor().asJson());
    object.insert("ambientColor", light.getAmbientColor().asJson());
    object.insert("specularColor", light.getSpecularColor().asJson());
    object.insert("attributes", light.getAttributes().asJson());
    object.insert("lightType", light.getType());
    object.insert("direction", light.getDirection().asJson());

    unbindLightBuffer();
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void LightComponent::loadLightFromJson(const QJsonValue & json)
{
    // Get Json as object
    const QJsonObject& object = json.toObject();

    Light& light = LightComponent::light();

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
        light.setDirection(Vector4g(object["direction"]));
    }

    // Set intensity
    light.setIntensity((float)object.value("intensity").toDouble());

    // Set attenuation/generic attributes
    //    For directional lights: empty
    //    For point lights: constant, linear, and quadratic attenuations: attenuation = a1 + a2 * distance + a3 * distance^2
    //        Default (1, 0, 0) is no attenuation (infinite light distance)
    //    For spot lights, direction and cutoff
    QJsonArray attribArray = object.value("attributes").toArray();
    Vector4g attribs;
    attribs.loadFromJson(attribArray);
    light.setAttributes(attribs);

    unbindLightBuffer();
}


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}