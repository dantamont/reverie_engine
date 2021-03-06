#include "GCoreEngine.h"
#include "GLogger.h"

#include "../GMainWindow.h"

#include "events/GEvent.h"

#include "scripting/GPythonModules.h"
#include "scripting/GPythonAPI.h"
#include "scripting/GPythonModules.h"
#include "scripting/GPythonScript.h"

#include "geometry/GEulerAngles.h"

#include "components/GTransformComponent.h"
#include "components/GLightComponent.h"
#include "components/GCameraComponent.h"
#include "components/GShaderComponent.h"
#include "components/GAudioSourceComponent.h"
#include "components/GAnimationComponent.h"
#include "components/GRigidBodyComponent.h"
#include "components/GCharControlComponent.h"
#include "rendering/shaders/GShaderProgram.h"
#include "rendering/buffers/GUniformBufferObject.h"

#include "loop/GSimLoop.h"
#include "scene/GScenario.h"
#include "scene/GScene.h"
#include "scene/GSceneObject.h"
#include "processes/GProcessManager.h"
#include "events/GEventManager.h"
#include "input/GInputHandler.h"
#include "physics/GPhysicsManager.h"
#include "sound/GSoundManager.h"
#include "animation/GAnimationManager.h"
#include "resource/GFileManager.h"

#include "rendering/renderer/GMainRenderer.h"
#include "rendering/renderer/GRenderContext.h"

#include "canvas/GFontManager.h"
#include "debugging/GDebugManager.h"

#include "../model_control/commands/GActionManager.h"
#include "../view/GWidgetManager.h"
#include "../view/tree/GSceneTreeWidget.h"
#include "../view/tree/GComponentWidget.h"
#include "../view/GL/GGLWidget.h"
#include "../view/logging/GConsoleTool.h"

#include "resource/GResourceCache.h"
#include "utils/GMemoryManager.h"
#include "threading/GParallelLoop.h"

#include "readers/models/GModelReader.h"

#define NUMBER_OF_HELPER_THREADS std::thread::hardware_concurrency()

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int CoreEngine::THREAD_COUNT = 0;

/////////////////////////////////////////////////////////////////////////////////////////////
ThreadPool CoreEngine::HELPER_THREADPOOL(NUMBER_OF_HELPER_THREADS);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CoreEngine::CoreEngine(int argc, char *argv[]):
    QObject(nullptr),
    m_isConstructed(false),
    m_fileManager(new FileManager(this, argc, argv)), // Initialize file manager before everything else, to set root path
    m_simulationLoop(nullptr),
    m_widgetManager(nullptr),
    m_actionManager(nullptr),
    m_resourceCache(nullptr),
    m_physicsManager(nullptr),
    m_fontManager(nullptr)
{ 
    // Initialize custom Qt types
    initializeMetaTypes();

    // Initialize Python API
    initializePythonAPI();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CoreEngine::~CoreEngine()
{
    delete m_simulationLoop;
    delete m_widgetManager;
    delete m_actionManager;
    delete m_eventManager;
    delete m_physicsManager;
    delete m_fontManager;
    delete m_debugManager;
    delete m_soundManager;
    delete m_animationManager;
    delete m_fileManager;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MainWindow * CoreEngine::mainWindow() const
{
    return m_widgetManager->mainWindow();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<MainRenderer> CoreEngine::mainRenderer()
{
    if (m_widgetManager->mainGLWidget()) {
        return m_widgetManager->mainGLWidget()->renderer();
    }
    throw("No GL widget found containing a renderer");
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
View::SceneTreeWidget* CoreEngine::sceneTreeWidget()
{
    if (m_widgetManager) {
        return m_widgetManager->sceneTreeWidget();
    }
    else {
        return nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
View::ComponentTreeWidget * CoreEngine::componentWidget()
{
    if (m_widgetManager) {
        return m_widgetManager->componentWidget();
    }
    else {
        return nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ProcessManager* CoreEngine::processManager() const
{
    return m_simulationLoop->processManager().get();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoreEngine::setScenario(std::shared_ptr<Scenario> scenario)
{
    clearEngine(false, true);
    m_scenario = scenario;
    emit scenarioChanged(); // TODO: Make these one signal
    emit scenarioLoaded();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoreEngine::clearEngine(bool clearActionStack, bool clearScenario)
{
    // Clear the undo stack
    if (clearActionStack) {
        m_actionManager->undoStack()->clear();
    }

    // Clear the process manager
    processManager()->clearAllProcesses();

    // Clear the resource cache
    m_resourceCache->clear();

    // Clear the physics manager
    physicsManager()->clear();

    // Clear the debug manager
    debugManager()->clear();

    // Clear the widget manager
    widgetManager()->clear();

    // Clear the sound manager
    m_soundManager->clear();

    // Clear the animation manager
    m_animationManager->clear();

    // Clear the file manager
    m_fileManager->clear();

    // Clear the Python API
    PythonAPI::Clear();

    // Clear the scenario
    if (m_scenario && clearScenario) {
        m_scenario = nullptr;
    }

    // Clear uniform buffer objects
    UBO::clearUBOs();

    // Clear Lights
    mainRenderer()->renderContext().reset();

    // Clear material count
    Material::clearCount();

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QOpenGLContext* CoreEngine::getGLWidgetContext()
{
    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (context != widgetManager()->mainGLWidget()->context()) {
        m_widgetManager->mainGLWidget()->makeCurrent();
    }
    return widgetManager()->mainGLWidget()->context();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoreEngine::setGLContext()
{
    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (context != widgetManager()->mainGLWidget()->context()) {
        m_widgetManager->mainGLWidget()->makeCurrent();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoreEngine::setNewScenario()
{
    if (!m_simulationLoop) {
        // Need to feed a persistant core engine shared_ptr to a scenario
        throw("Error, must initialize simulation loop before creating a scenario");
    }
    setScenario(std::make_shared<Scenario>(this));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoreEngine::initialize(MainWindow* mainWindow)
{
    // Initialize error logging
    initializeLogger();

    // Throw error if there isn't a main window
    if (!mainWindow) {
        auto& logger = rev::Logger::getInstance();
        logger.logMessage(LogLevel::Warning, "CoreEngine", "Warning, no main window in this application");
        throw("CoreEngine", "Warning, no main window in this application");
    }

    // Initialize Managers
    m_fontManager = new FontManager(this);
    m_widgetManager = new View::WidgetManager(this, mainWindow);
    m_actionManager = new ActionManager(this);
    m_simulationLoop = new SimulationLoop(this, 10); // max 100 FPS
    m_physicsManager = new PhysicsManager(this);
    m_debugManager = new DebugManager(this);
    m_soundManager = new SoundManager(this);
    m_animationManager = new AnimationManager(this);

    // Create resource cache
    m_resourceCache = new ResourceCache(this,
        m_simulationLoop->m_processManager.get(),
        MemoryManager::GET_MAX_MEMORY_MB(0.25));

    // Create eventManager
    m_eventManager = new EventManager(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoreEngine::postConstruction()
{
#ifdef DEBUG_MODE    
    GL::OpenGLFunctions::printGLError("Error before setting context");
#endif

    // Perform post-construction for managers
    m_fontManager->postConstruction();
    m_resourceCache->postConstruction();

    // Load built-in shader presets
    ShaderPreset::InitializeBuiltins(this);

    // Debug manager
    m_debugManager->postConstruction();

#ifdef DEBUG_MODE    
    GL::OpenGLFunctions::printGLError("Error after setting context");
#endif

    // Set default scenario
    auto* settings = new rev::Settings::INISettings();
    bool loaded = false;
    if (!settings->getRecentProject().isEmpty()) {
        QString filepath = settings->getRecentProject();
#ifdef DEBUG_MODE
        loaded = Scenario::LoadFromFile(filepath, this);
#else
        try {
            loaded = Scenario::LoadFromFile(filepath, this);
        }
        catch (std::exception e) {
            logWarning("Warning, failed to load file at " + filepath);
        }
#endif

        // Set window title
        mainWindow()->setWindowTitle(mainWindow()->getDefaultTitle() + " " + filepath);
    }

    if(!loaded) {
        // Initialize a blank scenario if load failed
        setNewScenario();
    }
    delete settings;

    // Initialize Python console
    // FIXME: Fix memory leak (just delete window after app closes)
    //QWidget* window = new QWidget();
    //PythonQtScriptingConsole* console = new PythonQtScriptingConsole(window, PythonQt::self()->getMainModule());
    //Q_UNUSED(console)
    //    window->setWindowTitle("Python Debug Console");
    //window->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    //window->show();

    // Once scenario is loaded, ensure that framebuffers are resized correctly
    m_widgetManager->postConstruction();

    // Engine is fully constructed
    m_isConstructed = true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CoreEngine::event(QEvent * event)
{
    if (event->type() == LogEvent::EVENT_TYPE)
    {
        // This is a log event
        LogEvent *logEvent = static_cast<LogEvent *>(event);
        logMessage(logEvent->getLogLevel(), 
            logEvent->getNamespaceName(),
            logEvent->getLogMessage());
    }

    return QObject::event(event);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoreEngine::initializeLogger()
{
    // Create logger
    auto& logger = rev::Logger::getInstance();
    logger.setLevel(LogLevel::Debug);

    Logger::getFileLogger("reverie", "AppFileLogger");

    Logger::getVSLogger("VisualStudio");

    Logger::getConsoleTool("ConsoleTool");

    Logger::getStdOutHandler("StandardOut");

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoreEngine::initializeMetaTypes()
{
    // Register types to send over qt signals/slots
    qRegisterMetaType<Uuid>("Uuid");
    qRegisterMetaType<Uuid>("rev::Uuid");

    qRegisterMetaType<std::shared_ptr<Object>>();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoreEngine::initializePythonAPI()
{
    // Initialize custom Python API
    rev::PythonAPI* api = rev::PythonAPI::get();
    Q_UNUSED(api);

    //py::print("Hello, World!"); // use the Python API
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoreEngine::initializeConnections()
{
    // Perform post-construction on GL widget initialization
    connect(m_widgetManager->mainGLWidget(),
        &View::GLWidget::initializedContext,
        this,
        &CoreEngine::postConstruction);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}