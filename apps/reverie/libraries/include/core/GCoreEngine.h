#pragma once

/// @todo Remove,  Workaround for VS 2017 bug
/// @see  https://developercommunity.visualstudio.com/content/problem/539919/e1097-unknown-attribute-no-init-all-in-winnth-in-w.html
#if (_MSC_VER >= 1915)
#define no_init_all deprecated
#endif

// QT
#include <QObject>
#include <QMetatype>
#include <memory>

// Internal
#include "GSettings.h"
#include "fortress/layer/framework/GSignalSlot.h"
#include "fortress/containers/GContainerExtensions.h"
#include "fortress/types/GIdentifiable.h"

namespace rev { 

class MainWindow;
class WidgetManager;
class SceneTreeWidget;
class ComponentTreeWidget;
class ApplicationGateway;
class OpenGlRenderer;
class EventManager;
class ActionManager;
class ProcessManager;
class PhysicsManager;
class DebugManager;
class FontManager;
class SoundManager;
class SimulationLoop;
class Scenario;
class ThreadPool;
class AnimationManager;
class FileManager;
class ResourceCache;


/// @class CoreEngine
/// @brief The singleton main engine driving the application
class CoreEngine: public QObject, public IdentifiableInterface {
    Q_OBJECT
public:
    typedef std::shared_ptr<Scenario> ScenarioPtr;

    /// @name Static
    /// @{

    /// @brief Signal that the scenario was laoded
    Signal<> s_scenarioLoaded;

    /// @todo No longer needs to be static, since engine is a singleton
    static unsigned int THREAD_COUNT;

    /// @todo No longer needs to be static, since engine is a singleton
    /// @brief Threadpool for helping with miscellaneous engine tasks
    static ThreadPool HELPER_THREADPOOL;

    /// @}

    /// @name Destructor
	/// @{

    CoreEngine(int argc = 0, char* argv[] = nullptr);
	~CoreEngine();

	/// @}

	/// @name Properties
	/// @{

    bool isConstructed() const { return m_isConstructed; }

    /// @brief Returns the main application window
    MainWindow* mainWindow() const;

    /// @brief Event manager class
    EventManager* eventManager() { return m_eventManager; }

    /// @brief Return the renderer
    std::shared_ptr<OpenGlRenderer> openGlRenderer();

    /// @brief Return the scene tree widget
    SceneTreeWidget* sceneTreeWidget();
	
    /// @brief Return the  component widget
    ComponentTreeWidget* componentWidget();

    /// @property Application gateway
    ApplicationGateway* applicationGateway() { return m_applicationGateway.get(); }

    /// @property Physics Manager
    PhysicsManager* physicsManager() const { return m_physicsManager; }

    /// @property Simulation loop
    SimulationLoop* simulationLoop() const { return m_simulationLoop; }

    /// @property Process manager
    ProcessManager* processManager() const { return m_processManager; }

    /// @property Font manager
    FontManager* fontManager() const { return m_fontManager; }

    /// @property Debug manager
    DebugManager* debugManager() const { return m_debugManager; }

	/// @property WidgetManager
	WidgetManager* widgetManager() const { return m_widgetManager; }

    /// @property UndoManager
    ActionManager* actionManager() const { return m_actionManager; }

    /// @property SoundManager
    SoundManager* soundManager() const { return m_soundManager; }

    /// @property AnimationManager
    AnimationManager* animationManager() const { return m_animationManager; }

    /// @property FileManager
    FileManager* fileManager() const { return m_fileManager; }

    /// @property Scenario
    json getScenarioJson() const;
    std::shared_ptr<Scenario> scenario() const { return m_scenario; }
    void setScenario(std::shared_ptr<Scenario> scenario);

	/// @}


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

signals:
    /// @brief Used to signal that GL widget needs to refresh settings
    void scenarioNeedsRedraw();

public slots:

    /// @brief What to run once all managers and widgets have initialized
    void postConstruction();

protected:
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

	/// @name Managers
	/// @{

    /// @brief The currently-loaded scenario containing scenes with objects to render
    std::shared_ptr<Scenario> m_scenario;

    /// @brief Controls the simulation loop
    SimulationLoop* m_simulationLoop;

	/// @brief Manages all widgets and the main window
	WidgetManager* m_widgetManager;

    /// @brief Manages all actions to undo and redo
    ActionManager* m_actionManager;

    /// @brief Cache of all resources
    ResourceCache* m_resourceCache;

    /// @brief QObject for emitting signals and handling events
    EventManager* m_eventManager;

    /// @brief Physics manager
    PhysicsManager* m_physicsManager;

    /// @brief Manager for handling processes
    ProcessManager* m_processManager;

    /// @brief Manages application fonts
    FontManager* m_fontManager;

    /// @brief Manages debugging functionality of the engine
    DebugManager* m_debugManager;

    /// @brief Manages all audio capabilities for the engine
    SoundManager* m_soundManager;

    /// @brief Manages all animation state machines for the engine
    AnimationManager* m_animationManager;

    /// @brief Manages search and working paths
    FileManager* m_fileManager;

	/// @}

    /// @name Protected members
    /// @{

    std::unique_ptr<ApplicationGateway> m_applicationGateway{ nullptr }; ///< Transmits all messages to/from the application

    bool m_isConstructed;

    /// @}

private:

    /// @todo Move signals and slots to another entity so this doesn't need to be a Q_OBJECT
    static CoreEngine* s_coreEngine; ///< Singleton instance of the core engine
};


// End namespacing
}
