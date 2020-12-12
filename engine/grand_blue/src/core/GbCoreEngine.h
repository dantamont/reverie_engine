#ifndef GB_CORE_ENGINE_H
#define GB_CORE_ENGINE_H

//////////////////////////////////////////////////////////////////////////////////
// Workaround for VS 2017 bug
// https://developercommunity.visualstudio.com/content/problem/539919/e1097-unknown-attribute-no-init-all-in-winnth-in-w.html
#if (_MSC_VER >= 1915)
#define no_init_all deprecated
#endif
//////////////////////////////////////////////////////////////////////////////////
// Includes  
//////////////////////////////////////////////////////////////////////////////////
// QT
#include <QObject>
#include <QMetatype>

// Internal
#include "GbSettings.h"
#include "GbObject.h"
#include "containers/GbContainerExtensions.h"

namespace Gb { 

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class MainWindow;
namespace View {
class WidgetManager;
class SceneTreeWidget;
class ComponentTreeWidget;
}
class MainRenderer;
class EventManager;
class ActionManager;
class ProcessManager;
class PhysicsManager;
class DebugManager;
class FontManager;
class SoundManager;
class SimulationLoop;
class Scenario;
class ResourceCache;
class ThreadPool;
class AnimationManager;

//////////////////////////////////////////////////////////////////////////////////
// Class Definitions
//////////////////////////////////////////////////////////////////////////////////

///@brief The main engine driving the application
class CoreEngine: public QObject, public Gb::Object {
    Q_OBJECT
public:
    typedef std::shared_ptr<Scenario> ScenarioPtr;
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    static unsigned int THREAD_COUNT;

    static std::unordered_map<QString, CoreEngine*>& engines() { return ENGINES; }

    /// @brief Threadpool for helping with miscellaneous engine tasks
    static ThreadPool HELPER_THREADPOOL;

    /// @brief Get path to base Reverie directory
    static QString GetRootPath();

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
	CoreEngine();
	~CoreEngine();

	/// @}

	//--------------------------------------------------------------------------------------------
	/// @name Properties
	/// @{

    bool isConstructed() const { return m_isConstructed; }

    /// @brief Returns the main application window
    MainWindow* mainWindow() const;

    /// @brief Return the resource cache
    ResourceCache* resourceCache() { return m_resourceCache; }

    /// @brief Event manager class
    EventManager* eventManager() { return m_eventManager; }

    /// @brief Return the renderer
    std::shared_ptr<MainRenderer> mainRenderer();

    /// @brief Return the scene tree widget
    View::SceneTreeWidget* sceneTreeWidget();
	
    /// @brief Return the  component widget
    View::ComponentTreeWidget* componentWidget();

    /// @property Physics Manager
    PhysicsManager* physicsManager() const { return m_physicsManager; }

    /// @property Simulation loop
    SimulationLoop* simulationLoop() const { return m_simulationLoop; }

    /// @property Process manager
    ProcessManager* processManager() const;

    /// @property Font manager
    FontManager* fontManager() const { return m_fontManager; }

    /// @property Debug manager
    DebugManager* debugManager() const { return m_debugManager; }

	/// @property WidgetManager
	View::WidgetManager* widgetManager() const { return m_widgetManager; }

    /// @property UndoManager
    ActionManager* actionManager() const { return m_actionManager; }

    /// @property SoundManager
    SoundManager* soundManager() const { return m_soundManager; }

    /// @property AnimationManager
    AnimationManager* animationManager() const { return m_animationManager; }

    /// @property Scenario
    std::shared_ptr<Scenario> scenario() const { return m_scenario; }
    void setScenario(std::shared_ptr<Scenario> scenario);

	/// @}


    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Clear all daata
    void clearEngine(bool clearActionStack, bool clearScenario = true);

    /// @brief Set the current OpenGL context to the main GL widget context
    /// @details Context must be set before creating a VAO, since they are not shared between contexts
    QOpenGLContext* getGLWidgetContext();
    void setGLContext();

    /// @brief Create a new scenario
    void setNewScenario();

    /// @brief Initialize the engine
    void initialize(MainWindow* mainWindow);

    /// @brief Setup all connections
    void initializeConnections();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "CoreEngine"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::CoreEngine"; }
    /// @}

signals:
    /// @brief Changed the scenario (used for signaling widgets to update display lists)
    // TODO: Remove this, move to event manager, and make specific to scene object changes
    void scenarioChanged();

    /// @brief Loaded in a new scenario
    void scenarioLoaded();

    /// @brief Used to signal that GL widget needs to refresh settings
    void scenarioNeedsRedraw();

public slots:

    /// @brief What to run once all managers and widgets have initialized
    void postConstruction();

protected:
	//--------------------------------------------------------------------------------------------
	/// @name Protected Methods
	/// @{

    /// @brief Override event handling
    virtual bool event(QEvent *event) override;

	/// @brief Initialize the logger
    void initializeLogger();

    /// @brief Set up all custom QT metatypes
    void initializeMetaTypes();

    /// @brief Initialize Python Qt API
    void initializePythonAPI();

	/// @}

	//--------------------------------------------------------------------------------------------
	/// @name Managers
	/// @{

    /// @brief The currently-loaded scenario containing scenes with objects to render
    std::shared_ptr<Scenario> m_scenario;

    /// @brief Controls the simulation loop
    SimulationLoop* m_simulationLoop;

	/// @brief Manages all widgets and the main window
	View::WidgetManager* m_widgetManager;

    /// @brief Manages all actions to undo and redo
    ActionManager* m_actionManager;

    /// @brief Cache of all resources
    ResourceCache* m_resourceCache;

    /// @brief QObject for emitting signals and handling events
    EventManager* m_eventManager;

    /// @brief Physics manager
    PhysicsManager* m_physicsManager;

    /// @brief Manages application fonts
    FontManager* m_fontManager;

    /// @brief Manages debugging functionality of the engine
    DebugManager* m_debugManager;

    /// @brief Manages all audio capabilities for the engine
    SoundManager* m_soundManager;

    /// @brief Manages all animation state machines for the engine
    AnimationManager* m_animationManager;

	/// @}


    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Map of core engines (realistically, will only ever be one)
    static std::unordered_map<QString, CoreEngine*> ENGINES;

    bool m_isConstructed;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespacing
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Typedef
///////////////////////////////////////////////////////////////////////////////////////////////////
typedef Gb::CoreEngine GbEngine;


#endif