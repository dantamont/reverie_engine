#include "GShaderComponent.h"

#include "../GCoreEngine.h"
#include "../resource/GResourceCache.h"
#include "../readers/GJsonReader.h"
#include "../rendering/shaders/GShaderProgram.h"
#include "../rendering/materials/GMaterial.h"
#include "../rendering/shaders/GShaderPreset.h"
#include "../scene/GScene.h"
#include "../scene/GScenario.h"
#include "../scene/GSceneObject.h"

namespace rev {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ShaderComponent::ShaderComponent() :
    Component(ComponentType::kShader)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ShaderComponent::ShaderComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kShader)
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::ShaderComponent::~ShaderComponent()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderComponent::enable()
{
    Component::enable();
}
//////////////// ///////////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderComponent::disable()
{
    Component::disable();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ShaderComponent::asJson(const SerializationContext& context) const
{
    QJsonObject object = Component::asJson(context).toObject();
    if (shaderPreset()) {
        object.insert("shaderPreset", shaderPreset()->getName().c_str());
    }

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ShaderComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)
    Component::loadFromJson(json);
    const QJsonObject& object = json.toObject();
    bool wasCreated;
    if (object.contains("shaderPreset")) {
        // This looks ridiculous, but a scene might not have a scenario, so we want the main scenario settings
        QString mtlName = object["shaderPreset"].toString();
        m_shaderPreset = sceneObject()->scene()->engine()->scenario()->settings().getShaderPreset(mtlName, wasCreated);
#ifdef DEBUG_MODE
        if (wasCreated) throw("Error, no shader preset found for the specified name");
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing