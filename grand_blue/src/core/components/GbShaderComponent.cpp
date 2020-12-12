#include "GbShaderComponent.h"

#include "../GbCoreEngine.h"
#include "../resource/GbResourceCache.h"
#include "../readers/GbJsonReader.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/materials/GbMaterial.h"
#include "../rendering/shaders/GbShaderPreset.h"
#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"

namespace Gb {
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
Gb::ShaderComponent::~ShaderComponent()
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
QJsonValue ShaderComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
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
        QString mtlName = object["shaderPreset"].toString();
        m_shaderPreset = m_engine->scenario()->settings().getShaderPreset(mtlName, wasCreated);
#ifdef DEBUG_MODE
        if (wasCreated) throw("Error, no shader preset found for the specified name");
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing