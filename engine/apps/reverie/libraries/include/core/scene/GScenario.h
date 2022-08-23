#pragma once

// Internal
#include <core/GManager.h>
#include "GScenarioSettings.h"
#include "GScene.h"
#include "GBlueprint.h"

namespace rev {

class AABB;

/// @class Scenario
/// @brief class for managing scenes
class Scenario : public QObject, public NameableInterface, public LoadableInterface{
    Q_OBJECT
public:
    /// @name Static
    /// @{

    /// @brief Load a scenario from a file
    /// @details Returns false if file not found
    static bool LoadFromFile(const GString& filepath, CoreEngine* core);

    /// @}

    /// @name Constructors/Destructor
	/// @{
    Scenario(); // for QMetaType declaration
    Scenario(const Scenario& scenario) = delete;
    Scenario(CoreEngine* core);
	~Scenario();
	/// @}

    /// @name Properties
    /// @{

    /// @brief Whether or not the scenario is currently loading
    bool isLoading() const { return m_isLoading; }

    /// @brief Return pointer to core engine 
    /// @details Used by scenes and scene objects
    CoreEngine* engine() const { return m_engine; }

    /// @brief Miscellaneous settings describing the scenario
    ScenarioSettings& settings() { return m_settings; }

    /// @brief Blueprints for scene objects
    const std::vector<Blueprint> & blueprints() const { return m_blueprints; }
    std::vector<Blueprint> & blueprints() { return m_blueprints; }

    /// @}

    /// @name Public Methods
	/// @{

    /// @brief Whether the given bounding box is visible in the scenario
    bool isVisible(const AABB& boundingBox);

    /// @brief Save the scenario to a file, overwriting all contents
    bool save();
    bool save(const GString& filepath);

    /// @brief Get map of scenes
    const Scene& scene() const { return m_scene; }
    Scene& scene() { return m_scene; }

    /// @brief Clear the scenario
    void clear();

	/// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Scenario& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Scenario& orObject);

    /// @}

signals:

    void addedScene(std::shared_ptr<Scene> scene);
    void removedScene(std::shared_ptr<Scene> scene);
    void clearedSceneObjects();

public slots:

    /// @brief Resize all framebuffers in the scenario that are associated with scene cameras
    void resizeCameras(uint32_t width, uint32_t height);

    /// @brief Update on resource load
    void onResourceLoaded(const Uuid& resourceId);

protected:

    /// @name Friends
    /// @{

    friend class ScenarioSettings;
    friend class Scene;

    /// @}

    /// @name Protected methods
    /// @{

    /// @brief Initialize the scenario
    void initialize();

    /// @}

    /// @name Protected members
    /// @{

    /// @brief the core engine
    CoreEngine* m_engine;

    /// @brief Whether or not the scenario is loading
    bool m_isLoading = false;

    /// @brief The scenes in this scenario
    Scene m_scene;

    /// @brief The settings associated with the scenario
    ScenarioSettings m_settings;

    /// @brief Blueprints for scene objects
    std::vector<Blueprint> m_blueprints;

    /// @todo Remove this connection
    Int32_t m_signalIndex; ///< Index of connected signal CoreEngine::s_scenario_loaded. Should just not connect this to signal

    static unsigned int SCENARIO_COUNT;
    /// @}
};
typedef std::shared_ptr<Scenario> ScenarioPtr;
Q_DECLARE_METATYPE(ScenarioPtr)


} // End namespaces
