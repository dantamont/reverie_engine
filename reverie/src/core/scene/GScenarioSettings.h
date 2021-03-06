/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SCENARIO_SETTINGS_H
#define GB_SCENARIO_SETTINGS_H

// QT

// Internal
#include "../mixins/GLoadable.h"
#include "../containers/GSortingLayer.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class Scene;
class Scenario;
class CoreEngine;
class PythonClassScript;
class ShaderPreset;
class AABB;

/////////////////////////////////////////////////////////////////////////////////////////////
// Type definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

// TODO: Don't let sorting layers persist in cameras and scene objects if deleted from here
/// @class ScenarioSettings
/// @brief Class for managing scenario settings
class ScenarioSettings : public Serializable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ScenarioSettings(Scenario* scenario);
    ~ScenarioSettings() {
    }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    SortingLayers& renderLayers() { return m_renderLayers; }

    /// @brief Map of all shader presets
    std::vector<std::shared_ptr<ShaderPreset>>& shaderPresets() { return m_shaderPresets; }


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Remove a given render layer from all scene objects in the scenario
    void onRemoveRenderLayer(size_t layerId);

    /// @brief Remove shader material from resource cache
    bool removeShaderPreset(const GString& name);

    /// @brief Whether the resource map has the specified shader preset
    bool hasShaderPreset(const GString& name, int* iterIndex = nullptr) const;

    /// @brief Shader preset
    std::shared_ptr<ShaderPreset> getShaderPreset(const GString& name, bool& created);
    //std::shared_ptr<ShaderPreset> getShaderPreset(const Uuid& uuid);

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Loadable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    friend class Scenario;

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    Scenario* m_scenario;

    /// @brief All of the layers for rendering available for the scene
    SortingLayers m_renderLayers;

    /// @brief The default render order for the scenario
    // TODO: Implement

    /// @brief Map of all shader presets
    std::vector<std::shared_ptr<ShaderPreset>> m_shaderPresets;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif