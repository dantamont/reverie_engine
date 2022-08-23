#include "core/scene/GScenarioSettings.h"
#include "core/scene/GScenario.h"
#include "core/scene/GScene.h"
#include "core/scene/GSceneObject.h"

#include "core/components/GModelComponent.h"
#include "core/GCoreEngine.h"
#include "fortress/json/GJson.h"
#include <core/rendering/shaders/GShaderPreset.h>

namespace rev{

ScenarioSettings::ScenarioSettings(Scenario * scenario) :
    m_scenario(scenario) {

    // Add core render layers
    m_renderLayers.addLayer("skybox", -10);
    m_renderLayers.addLayer("world", 0);
    m_renderLayers.addLayer("effects", 5);
    m_renderLayers.addLayer("ui", 10);
}

void to_json(json& orJson, const ScenarioSettings& korObject)
{
    orJson["renderLayers"] = korObject.m_renderLayers;

    json shaderPresets = json::array();
    for (const std::shared_ptr<ShaderPreset>& preset : korObject.m_shaderPresets) {
        shaderPresets.push_back(*preset);
    }
    orJson["shaderPresets"] = shaderPresets;
}

void from_json(const json& korJson, ScenarioSettings& orObject)
{
    orObject.m_renderLayers.clear();
    orObject.m_shaderPresets.clear();

    
    if (korJson.contains("renderLayers")) {
        korJson["renderLayers"].get_to(orObject.m_renderLayers);
    }

    //if (!m_renderLayers.size()) {
    //    // Ensure that there is always the default render layer
    //    m_renderLayers.push_back(std::make_shared<SortingLayer>());
    //}

    if (korJson.contains("shaderPresets")) { // legacy check
        const json& shaderPresets = korJson.at("shaderPresets");
        for (const auto& shaderJson : shaderPresets) {
            auto shaderPreset = std::make_shared<ShaderPreset>(orObject.m_scenario->engine(), shaderJson);
            orObject.m_shaderPresets.push_back(shaderPreset);
        }
    }
}

void ScenarioSettings::onRemoveRenderLayer(uint32_t layerId)
{
    // Iterate through all scene objects to remove the now invalid render layer
    std::vector<std::shared_ptr<SceneObject>>& topLevelObjects = m_scenario->scene().topLevelSceneObjects();
    for (const auto& so : topLevelObjects) {
        so->removeRenderLayer(layerId);
    }
}

bool ScenarioSettings::removeShaderPreset(const GString & name)
{
    int idx;
    if (hasShaderPreset(name, &idx)) {
        m_shaderPresets.erase(idx + m_shaderPresets.begin());
        return true;
    }

    Logger::Throw("Error, shader material with the specified name not found");
    return false;
}

bool ScenarioSettings::hasShaderPreset(const GString & name, int* iterIndex) const
{
    auto iter = std::find_if(m_shaderPresets.begin(), m_shaderPresets.end(),
        [&](const std::shared_ptr<ShaderPreset>& preset) {
        return preset->getName() == name;
    });

    bool hasPreset = iter != m_shaderPresets.end();
    if (hasPreset) {
        *iterIndex = iter - m_shaderPresets.begin();
    }

    return hasPreset;
}

std::shared_ptr<ShaderPreset> ScenarioSettings::getShaderPreset(const GString & name, bool & created)
{
    // Check if shader material is in the map 
    int idx;
    if (hasShaderPreset(name, &idx)) {
        created = false;
        return m_shaderPresets[idx];
    }
    else {
        // Check for built-in presets
        const std::shared_ptr<ShaderPreset> preset = ShaderPreset::GetBuiltin(name);
        if (preset) {
            created = false;
            return preset;
        }
    }

    // Create new shader preset if not in map
    created = true;
    auto preset = std::make_shared<ShaderPreset>(m_scenario->engine(), name);
    addShaderPreset(preset);
    return preset;
}

void ScenarioSettings::addShaderPreset(const std::shared_ptr<ShaderPreset>& preset)
{
    m_shaderPresets.push_back(preset);
    onShaderPresetChanged(preset->getUuid());
}

void ScenarioSettings::onShaderPresetChanged(const Uuid& presetId)
{
    Scene& scene = m_scenario->scene();
    for (ModelComponent* m : scene.models()) {
        m->sceneObject()->createModelDrawCommands();
        m->sceneObject()->createShadowDrawCommands();
    }
}

//std::shared_ptr<ShaderPreset> ScenarioSettings::getShaderPreset(const Uuid & uuid)
//{
//    return m_shaderPresets[uuid];
//}


// End namespaces        
}