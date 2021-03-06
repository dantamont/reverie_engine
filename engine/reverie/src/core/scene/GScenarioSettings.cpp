#include "GScenarioSettings.h"
#include "GScenario.h"
#include "GScene.h"
#include "GSceneObject.h"

#include "../GCoreEngine.h"
#include "../readers/GJsonReader.h"
#include <core/rendering/shaders/GShaderPreset.h>

namespace rev{
///////////////////////////////////////////////////////////////////////////////////////////////////
ScenarioSettings::ScenarioSettings(Scenario * scenario) :
    m_scenario(scenario) {

    // Add core render layers
    m_renderLayers.addLayer("skybox", -10);
    m_renderLayers.addLayer("world", 0);
    m_renderLayers.addLayer("effects", 5);
    m_renderLayers.addLayer("ui", 10);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ScenarioSettings::asJson(const SerializationContext& context) const
{
    QJsonObject object = QJsonObject();
    
    QJsonArray renderLayers;
    for (const auto& renderLayer : m_renderLayers.m_layers) {
        renderLayers.append(renderLayer->asJson());
    }
    object.insert("renderLayers", renderLayers);

    QJsonArray shaderPresets;
    for (const auto& preset : m_shaderPresets) {
        shaderPresets.append(preset->asJson());
    }
    object.insert("shaderPresets", shaderPresets);

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScenarioSettings::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context);

    m_renderLayers.m_layers.clear();
    m_shaderPresets.clear();

    QJsonObject object = json.toObject();
    if (object.contains("renderLayers")) {
        QJsonArray renderLayers = object["renderLayers"].toArray();
        for (const auto& layerJson : renderLayers) {
            m_renderLayers.m_layers.push_back(std::make_unique<SortingLayer>(layerJson));
        }
    }
    m_renderLayers.sort();

    //if (!m_renderLayers.size()) {
    //    // Ensure that there is always the default render layer
    //    m_renderLayers.push_back(std::make_shared<SortingLayer>());
    //}

    if (object.contains("shaderPresets")) { // legacy check
        const QJsonArray& shaderPresets = object.value("shaderPresets").toArray();
        for (const auto& shaderJson : shaderPresets) {
            auto shaderPreset = std::make_shared<ShaderPreset>(m_scenario->engine(), shaderJson);
            m_shaderPresets.push_back(shaderPreset);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ScenarioSettings::onRemoveRenderLayer(size_t layerId)
{
    // Iterate through all scene objects to remove the now invalid render layer
    std::vector<std::shared_ptr<SceneObject>>& topLevelObjects = m_scenario->scene().topLevelSceneObjects();
    for (const auto& so : topLevelObjects) {
        so->removeRenderLayer(layerId);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////s
bool ScenarioSettings::removeShaderPreset(const GString & name)
{
    int idx;
    if (hasShaderPreset(name, &idx)) {
        m_shaderPresets.erase(idx + m_shaderPresets.begin());
        return true;
    }

    throw("Error, shader material with the specified name not found");
    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
    m_shaderPresets.push_back(preset);
    return preset;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//std::shared_ptr<ShaderPreset> ScenarioSettings::getShaderPreset(const Uuid & uuid)
//{
//    return m_shaderPresets[uuid];
//}

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}