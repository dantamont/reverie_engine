/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SCENARIO_H
#define GB_SCENARIO_H

// QT

// Internal
#include "../GbManager.h"
#include "../mixins/GbLoadable.h"
#include "../containers/GbSortingLayer.h"

namespace Gb {

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

    std::shared_ptr<SortingLayer> renderLayer(const GString& label) {
        auto iter = std::find_if(m_renderLayers.begin(), m_renderLayers.end(),
            [&](const std::shared_ptr<SortingLayer>& layer) {
            return layer->getName() == label;
        });
        if (iter != m_renderLayers.end())
            return *iter;
        else
            return nullptr;
    }

    std::shared_ptr<SortingLayer> addRenderLayer();
    std::shared_ptr<SortingLayer> addRenderLayer(const GString& name, int order);
    bool removeRenderLayer(const GString& label);

    /// @brief Sort the vector of render layers to reflect any modifications
    void sortRenderLayers();

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
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    friend class Scenario;

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    static bool SortRenderLayers(const std::shared_ptr<SortingLayer>& l1, 
        const std::shared_ptr<SortingLayer>& l2);

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
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class Scenario
/// @brief class for managing scenes
class Scenario : public QObject, public Object, public Loadable{
    Q_OBJECT
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Load a scenario from a file
    /// @details Returns false if file not found
    static bool LoadFromFile(const GString& filepath, CoreEngine* core);

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Scenario(); // for QMetaType declaration
    Scenario(const Scenario& scenario);
    Scenario(CoreEngine* core);
	~Scenario();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Whether or not the scenario is currently loading
    bool isLoading() const { return m_isLoading; }

    /// @brief Return pointer to core engine 
    /// @details Used by scenes and scene objects
    CoreEngine* engine() const { return m_engine; }

    /// @brief Miscellaneous settings describing the scenario
    ScenarioSettings& settings() { return m_settings; }

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    ///// @brief Get the visible frustum bounds for the entire scenario
    //const AABB& getVisibleFrustumBounds();
    //void updateVisibleFrustumBounds();

        /// @brief Whether the given bounding box is visible in the scenario
    bool isVisible(const AABB& boundingBox);

    /// @brief Save the scenario to a file, overwriting all contents
    bool save();
    bool save(const GString& filepath);

    /// @brief Whether or not the scenario has scenes
    bool isEmpty() const { return m_scenes.size() == 0; }

    /// @brief Create a scene and add to scene map
    std::shared_ptr<Scene> addScene();

    /// @brief Remove a scene from the map
    void removeScene(const std::shared_ptr<Scene>& scene);

    /// @brief Get a scene given the UUID
    const std::shared_ptr<Scene>& getScene(const Uuid& uuid);

    /// @brief Get a scene by name
    std::shared_ptr<Scene> getSceneByName(const GString& name);

    /// @brief Get map of scenes
    const std::vector<std::shared_ptr<Scene>>& getScenes() const { return m_scenes; }
    std::vector<std::shared_ptr<Scene>>& scenes() { return m_scenes; }

    /// @brief Clear the scenario
    void clear();

    /// @brief Clear all scenes of objects
    void clearSceneObjects();

    /// @brief Removes all scenes
    void removeScenes();

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Loadable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb Object Properties
    /// @{
    /** @property className
        @brief The name of this class
    */
    const char* className() const override { return "Scenario"; }

    /** @property namespaceName
        @brief The full namespace for this class
    */
    const char* namespaceName() const override { return "Gb::Scenario"; }
    /// @}

signals:

    void addedScene(std::shared_ptr<Scene> scene);
    void removedScene(std::shared_ptr<Scene> scene);
    void clearedSceneObjects();

public slots:

    /// @brief Resize all framebuffers in the scenario that are associated with scene cameras
    void resizeCameras(size_t width, size_t height);

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class ScenarioSettings;
    friend class Scene;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Add an existing scene to scene map
    void addScene(const std::shared_ptr<Scene>& scene);

    /// @brief Initialize the scenario
    void initialize();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief the core engine
    CoreEngine* m_engine;

    /// @brief Whether or not the scenario is loading
    bool m_isLoading = false;

    /// @brief Aggregated bounds of all view frustums in the scenario
    //AABB m_visibleBounds;

    /// @brief The scenes in this scenario
    std::vector<std::shared_ptr<Scene>> m_scenes;

    /// @brief The settings associated with the scenario
    ScenarioSettings m_settings;

    static unsigned int SCENARIO_COUNT;
    /// @}
};
typedef std::shared_ptr<Scenario> ScenarioPtr;
Q_DECLARE_METATYPE(ScenarioPtr)

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif