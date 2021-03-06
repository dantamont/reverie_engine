/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SCENARIO_H
#define GB_SCENARIO_H

// QT

// Internal
#include <core/GManager.h>
#include "GScenarioSettings.h"
#include "GScene.h"
#include "GBlueprint.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class AABB;

/////////////////////////////////////////////////////////////////////////////////////////////
// Type definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class Scenario
/// @brief class for managing scenes
class Scenario : public QObject, public Object, public Nameable, public Loadable{
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
    Scenario(const Scenario& scenario) = delete;
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

    /// @brief Blueprints for scene objects
    const std::vector<Blueprint> & blueprints() const { return m_blueprints; }
    std::vector<Blueprint> & blueprints() { return m_blueprints; }

    /// @}
	//--------------------------------------------------------------------------------------------
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

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Object Properties
    /// @{
    /** @property className
        @brief The name of this class
    */
    const char* className() const override { return "Scenario"; }

    /** @property namespaceName
        @brief The full namespace for this class
    */
    const char* namespaceName() const override { return "rev::Scenario"; }
    /// @}

signals:

    void addedScene(std::shared_ptr<Scene> scene);
    void removedScene(std::shared_ptr<Scene> scene);
    void clearedSceneObjects();

public slots:

    /// @brief Resize all framebuffers in the scenario that are associated with scene cameras
    void resizeCameras(size_t width, size_t height);

    /// @brief Update on resource load
    void onResourceLoaded(const Uuid& resourceId);

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
    Scene m_scene;

    /// @brief The settings associated with the scenario
    ScenarioSettings m_settings;

    /// @brief Blueprints for scene objects
    std::vector<Blueprint> m_blueprints;

    static unsigned int SCENARIO_COUNT;
    /// @}
};
typedef std::shared_ptr<Scenario> ScenarioPtr;
Q_DECLARE_METATYPE(ScenarioPtr)

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif