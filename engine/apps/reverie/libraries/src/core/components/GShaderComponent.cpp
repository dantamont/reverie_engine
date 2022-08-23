#include "core/components/GShaderComponent.h"

#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "fortress/json/GJson.h"
#include "core/components/GModelComponent.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/materials/GMaterial.h"
#include "core/rendering/shaders/GShaderPreset.h"
#include "core/scene/GScene.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"

namespace rev {

ShaderComponent::ShaderComponent() :
    Component(ComponentType::kShader)
{
}

ShaderComponent::ShaderComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kShader)
{
    setSceneObject(sceneObject());
    sceneObject()->setComponent(this);
}

rev::ShaderComponent::~ShaderComponent()
{
}

void ShaderComponent::enable()
{
    Component::enable();
}
 
void ShaderComponent::disable()
{
    Component::disable();
}

const std::shared_ptr<const ShaderPreset> ShaderComponent::shaderPreset() const
{
    if (std::shared_ptr<const ShaderPreset> preset = m_shaderPreset.lock())
        return preset;
    else
        return nullptr;
}


void ShaderComponent::setShaderPreset(const std::shared_ptr<ShaderPreset>& preset)
{
    m_shaderPreset = preset;
    onShaderPresetChanged();
}

void ShaderComponent::onShaderPresetChanged()
{
    sceneObject()->createModelDrawCommands();
    sceneObject()->createShadowDrawCommands();
}

void to_json(json& orJson, const ShaderComponent& korObject)
{
    ToJson<Component>(orJson, korObject);
    if (korObject.shaderPreset()) {
        orJson["shaderPreset"] = korObject.shaderPreset()->getName().c_str();
    }
}

void from_json(const json& korJson, ShaderComponent& orObject)
{
    FromJson<Component>(korJson, orObject);
    bool wasCreated;
    if (korJson.contains("shaderPreset")) {
        // This looks ridiculous, but a scene might not have a scenario, so we want the main scenario settings
        GString mtlName;
        korJson.at("shaderPreset").get_to(mtlName);
        orObject.m_shaderPreset = orObject.sceneObject()->scene()->engine()->scenario()->settings().getShaderPreset(mtlName, wasCreated);
#ifdef DEBUG_MODE
        if (wasCreated) Logger::Throw("Error, no shader preset found for the specified name");
#endif
    }
}


} // end namespacing