/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SCENARIO_H
#define GB_SCENARIO_H

// QT

// Internal
#include "../GbManager.h"
#include "../mixins/GbLoadable.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class Scene;
class Scenario;
class CoreEngine;
class PythonClassScript;

/////////////////////////////////////////////////////////////////////////////////////////////
// Type definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class ScenarioSettings
/// @brief Class for managing scenario settings
class ScenarioSettings: public Serializable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ScenarioSettings(Scenario* scenario);
    ~ScenarioSettings() {
    }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Loadable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}
private:
    friend class Scenario;
    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    Scenario* m_scenario;

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
    /// @name static
    /// @{

    /// @brief Load a scenario from a file
    static void loadFromFile(const QString& filepath, CoreEngine* core);

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

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Save the scenario to a file, overwriting all contents
    bool save();
    bool save(const QString& filepath);

    /// @brief Whether or not the scenario has scenes
    bool isEmpty() const { return m_scenes.size() == 0; }

    /// @brief Create a scene and add to scene map
    std::shared_ptr<Scene> addScene();

    /// @brief Remove a scene from the map
    void removeScene(std::shared_ptr<Scene> scene);

    /// @brief Get a scene given the UUID
    std::shared_ptr<Scene> getScene(const Uuid& uuid);

    /// @brief Get a scene by name
    std::shared_ptr<Scene> getSceneByName(const QString& name);

    /// @brief Get map of scenes
    const std::unordered_map<Uuid, std::shared_ptr<Scene>>& getScenes() const { return m_scenes; }
    std::unordered_map<Uuid, std::shared_ptr<Scene>>& scenes() { return m_scenes; }

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
    virtual void loadFromJson(const QJsonValue& json) override;

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
    void addScene(std::shared_ptr<Scene> scene);
    //std::shared_ptr<Scene> addScene(const Uuid& uuid); // Scene gets the given UUID

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

    /// @brief The scenes in this scenario
    std::unordered_map<Uuid, std::shared_ptr<Scene>> m_scenes;

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