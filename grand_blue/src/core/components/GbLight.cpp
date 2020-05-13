#include "GbLight.h"

#include "../geometry/GbMatrix.h"
#include "GbTransformComponent.h"
#include "../scene/GbSceneObject.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/shaders/GbUniformBufferObject.h"

namespace Gb{
//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::clearLights()
{
    LIGHT_COUNT = 0;
    LIGHTS.clear();
    DELETED_INDICES.clear();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Light::Light():
    Component(kLight),
    m_lightType(kPoint)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Light::Light(const std::shared_ptr<SceneObject>& object, LightType type):
    Component(object, kLight),
    m_intensity(1),
    m_lightType(type)
{
    initializeLight();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Light::Light(const std::shared_ptr<SceneObject>& object, const Color& color, LightType type):
    Component(object, kLight),
    m_diffuseColor(color),
    m_ambientColor(color),
    m_specularColor(color),
    m_intensity(1),
    m_lightType(type)
{
    initializeLight();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Light::~Light()
{
    clearLight();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::enable()
{
    Component::enable();
    setIntensity(m_cachedIntensity, false);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::disable()
{
    Component::disable();
    m_cachedIntensity = m_intensity;
    setIntensity(0, false);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Light::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    object.insert("intensity", m_intensity);
    object.insert("diffuseColor", m_diffuseColor.toVector4g().asJson());
    object.insert("ambientColor", m_ambientColor.toVector4g().asJson());
    object.insert("specularColor", m_specularColor.toVector4g().asJson());
    object.insert("attributes", m_attributes.asJson());
    object.insert("lightType", m_lightType);
    object.insert("direction", m_direction.asJson());

    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::loadFromJson(const QJsonValue & json)
{
    Component::loadFromJson(json);

    // Get Json as object
    const QJsonObject& object = json.toObject();

    // Set type
    if (object.contains("lightType")) {
        setType(LightType(object["lightType"].toInt()));
    }
    else {
        setType(kPoint);
    }

    // Set color
    if (object.contains("color")) {
        // DEPRECATED
        QJsonValue colorJson = object.value("color");
        Vector4f colorVec(colorJson);
        Color color(colorVec);
        setDiffuseColor(color);
        setAmbientColor(color);
        setSpecularColor(color);
    }
    else {
        QJsonValue diffuseColorJson = object.value("diffuseColor");
        Vector4f diffuseColorVec(diffuseColorJson);
        Color diffuseColor(diffuseColorVec);
        setDiffuseColor(diffuseColor);

        QJsonValue ambientColorJson = object.value("ambientColor");
        Vector4f ambientColorVec(ambientColorJson);
        Color ambientColor(ambientColorVec);
        setAmbientColor(ambientColor);

        QJsonValue specularColorJson = object.value("specularColor");
        Vector4f specularColorVec(specularColorJson);
        Color specularColor(specularColorVec);
        setSpecularColor(specularColor);
    }

    // Set direction
    if (object.contains("direction")) {
        setDirection(Vector4g(object["direction"]));
    }

    // Set intensity
    setIntensity((float)object.value("intensity").toDouble());

    // Set attenuation
    QJsonArray attribArray;
    if (object.contains("attenuation")) {
        // Deprecated, attenuation is a generic attribs list now
        attribArray = object.value("attenuation").toArray();
    }
    else {
        attribArray = object.value("attributes").toArray();
    }
    Vector4g attribs;
    attribs.loadFromJson(attribArray);
    setAttributes(attribs);

}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::clearLight()
{
    // Set intensity of light to zero
    setIntensity(0);

    // Remove from global list of lights
    LIGHTS[m_index] = nullptr;

    // Flag index for overwrite in static list
    DELETED_INDICES.push_back(m_index);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::initializeLight()
{
    // Add light to scene object
    if (!sceneObject()) {
        throw("Error, light needs a scene object");
    }
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);

    // Set attributes based on type of light
    switch (m_lightType) {
    case kPoint:
        m_attributes = {1.0f, 0.0f, 0.0f, 0.0f};
        break;
    case kDirectional:
        // No attributes for directional
        break;
    case kSpot:        
        // Cutoff is cosine of the half angle of the light (ranges from 0 to 1.0f, aka 0deg to 90deg)
        m_attributes = { 1.0f, 0.0f, 0.0f, (float)cos(Constants::DEG_TO_RAD * 25.0) };
        break;
    }


    // Set index and increment light count
    if (DELETED_INDICES.size() > 0) {
        m_index = DELETED_INDICES.back();
        DELETED_INDICES.pop_back();

        // Add to static list
        LIGHTS[m_index] = this;
    }
    else {
        m_index = LIGHT_COUNT;
        LIGHT_COUNT++;

        // Add to static lists
        Vec::EmplaceBack(LIGHTS, this);
    }
    // Throw error if there are too many lights
    if (LIGHT_COUNT > MAX_LIGHTS) {
        throw("Error, exceeded max number of allowable lights");
    }

    // Initialize global UBO uniforms
    UBO::getLightBuffer()->setUniformValue("lightCount", (int)LIGHT_COUNT);
    UBO::getLightBuffer()->setUniformValue("lightingModel", (int)LIGHTING_MODEL);

    // Initialize light position
    setPosition(sceneObject()->transform()->getPosition().asReal());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::updatePosition(const Vector3g& position) const
{
    Vector4g pos(position);
    pos[3] = 1.0f;
    QString str = QStringLiteral("lightPositions");
    UBO::getLightBuffer()->setUniformSubValue(str, m_index, pos);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::updateDirection() const
{
    QString str = QStringLiteral("lightDirections");
    UBO::getLightBuffer()->setUniformSubValue(str, m_index, m_direction);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::updateDiffuseColor() const
{
    QString str = QStringLiteral("lightDiffuseColors");
    UBO::getLightBuffer()->setUniformSubValue(str, m_index, m_diffuseColor.toVector4g());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::updateAmbientColor() const
{
    QString str = QStringLiteral("lightAmbientColors");
    UBO::getLightBuffer()->setUniformSubValue(str, m_index, m_ambientColor.toVector4g());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::updateSpecularColor() const
{
    QString str = QStringLiteral("lightSpecularColors");
    UBO::getLightBuffer()->setUniformSubValue(str, m_index, m_specularColor.toVector4g());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::updateAttributes() const
{
    QString str = QStringLiteral("lightAttributes");
    UBO::getLightBuffer()->setUniformSubValue(str, m_index, m_attributes);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::updateIntensity() const
{
    QString str = QStringLiteral("lightAttributes1");
    UBO::getLightBuffer()->setUniformSubValue(str, m_index, std::move(getTypeAndIntensity()));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Light::updateType() const
{
    QString str = QStringLiteral("lightAttributes1");
    UBO::getLightBuffer()->setUniformSubValue(str, m_index, std::move(getTypeAndIntensity()));
}

//////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int Light::LIGHT_COUNT = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<Light*> Light::LIGHTS = {};

//////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<int> Light::DELETED_INDICES = {};

//////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int Light::MAX_LIGHTS = 20;

//////////////////////////////////////////////////////////////////////////////////////////////////
Light::LightingModel Light::LIGHTING_MODEL = Light::kBlinnPhong;

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}