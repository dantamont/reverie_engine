#pragma once

// Internal
#include "fortress/types/GLoadable.h"
#include "fortress/containers/GSortingLayer.h"

namespace rev {

class Scene;
class Scenario;
class CoreEngine;
class PythonClassScript;
class ShaderPreset;
class AABB;
class Uuid;

/// @class ScenarioSettings
/// @brief Class for managing scenario settings
/// @todo Don't let sorting layers persist in cameras and scene objects if deleted from here
class ScenarioSettings  {
public:
    /// @name Constructors/Destructor
    /// @{
    ScenarioSettings(Scenario* scenario);
    ~ScenarioSettings() {
    }
    /// @}

    /// @name Properties
    /// @{

    SortingLayers& renderLayers() { return m_renderLayers; }

    /// @brief Map of all shader presets
    const std::vector<std::shared_ptr<ShaderPreset>>& shaderPresets() const { return m_shaderPresets; }


    /// @}

    /// @name Public methods
    /// @{

    /// @brief Remove a given render layer from all scene objects in the scenario
    void onRemoveRenderLayer(uint32_t layerId);

    /// @brief Remove shader material from resource cache
    bool removeShaderPreset(const GString& name);

    /// @brief Whether the resource map has the specified shader preset
    bool hasShaderPreset(const GString& name, int* iterIndex = nullptr) const;

    /// @brief Shader preset
    /// @todo Add preset in a separate method
    std::shared_ptr<ShaderPreset> getShaderPreset(const GString& name, bool& created);

    void addShaderPreset(const std::shared_ptr<ShaderPreset>& preset);

    void onShaderPresetChanged(const Uuid& presetId);

    /// @}

    /// @name LoadableInterface Overrides
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const ScenarioSettings& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, ScenarioSettings& orObject);


    /// @}

protected:
    friend class Scenario;

    /// @name Protected Members
    /// @{

    Scenario* m_scenario{ nullptr };
    SortingLayers m_renderLayers; ///< All of the layers for rendering available for the scene
    std::vector<std::shared_ptr<ShaderPreset>> m_shaderPresets; ///< Map of all shader presets

    /// @}
};


} // End namespaces
