#include "core/GCoreEngine.h"
#include "logging/GLogger.h"

#include "GMainWindow.h"

#include "core/canvas/GLabel.h"
#include "core/events/GEvent.h"

#include "core/scripting/GPythonModules.h"
#include "core/scripting/GPythonAPI.h"
#include "core/scripting/GPythonModules.h"
#include "core/scripting/GPythonScript.h"

#include "fortress/containers/math/GEulerAngles.h"

#include "core/components/GTransformComponent.h"
#include "core/components/GLightComponent.h"
#include "core/components/GCameraComponent.h"
#include "core/components/GShaderComponent.h"
#include "core/components/GAudioSourceComponent.h"
#include "core/components/GAnimationComponent.h"
#include "core/components/GRigidBodyComponent.h"
#include "core/components/GCharControlComponent.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/buffers/GUniformBufferObject.h"

#include "core/loop/GSimLoop.h"
#include "core/scene/GScenario.h"
#include "core/scene/GScene.h"
#include "core/scene/GSceneObject.h"
#include "core/processes/GProcessManager.h"
#include "core/events/GEventManager.h"
#include "core/events/GLogEvent.h"
#include "core/physics/GPhysicsManager.h"
#include "core/sound/GSoundManager.h"
#include "core/animation/GAnimationManager.h"
#include "core/resource/GFileManager.h"

#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/renderer/GRenderContext.h"

#include "core/debugging/GDebugManager.h"

#include "core/layer/gateway/GApplicationGateway.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "core/layer/view/widgets/graphics/GInputHandler.h"

#include "geppetto/qt/actions/GActionManager.h"
#include "geppetto/qt/fonts/GFontManager.h"
#include "geppetto/layer/gateway/GWidgetGateway.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/tree/GSceneTreeWidget.h"
#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "geppetto/qt/widgets/logging/GConsoleTool.h"

#include "core/resource/GResourceCache.h"
#include "fortress/system/memory/GPointerTypes.h"
#include "fortress/system/memory/GMemoryMonitor.h"
#include "fortress/thread/GParallelLoop.h"

#include "core/readers/models/GModelReader.h"

#define NUMBER_OF_HELPER_THREADS std::thread::hardware_concurrency()

namespace rev {


unsigned int CoreEngine::THREAD_COUNT = 0;


ThreadPool CoreEngine::HELPER_THREADPOOL(NUMBER_OF_HELPER_THREADS);


CoreEngine::CoreEngine(int argc, char* argv[]) :
    QObject(nullptr),
    m_isConstructed(false),
    m_fileManager(nullptr),
    m_processManager(nullptr),
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

CoreEngine::~CoreEngine()
{
    clearEngine(true, true); // Need to clear scenario before widget manager (and mainGLWidget(/context?)) is destroyed
    
    // Clear the Python API
    /// @note Needs to be after scenario is cleared, since scripted processes have associated python objects
    PythonAPI::Delete();
    
    Ubo::ClearUBOs(true); // Clear core UBOs before OpenGL context is destroyed
    delete m_debugManager;  // Doesn't behave if deleted after resource cache
    ResourceCache::Delete();
    delete m_simulationLoop;
    delete m_processManager;
    delete m_fontManager;
    GlFontFaceDispatcher::Clear(); // Must delete before widget manager and openGL context disappear
    delete m_widgetManager; 
    delete m_actionManager;
    delete m_eventManager;
    //PhysicsManager::Clear(true); // Must delete default shape prefab before physx shuts down
    delete m_physicsManager;
    delete m_soundManager;
    delete m_animationManager;
    delete m_fileManager;

    Logger::Delete();
}

MainWindow * CoreEngine::mainWindow() const
{
    return static_cast<MainWindow*>(m_widgetManager->mainWindow());
}

std::shared_ptr<OpenGlRenderer> CoreEngine::openGlRenderer()
{
    if (m_widgetManager->mainGLWidget()) {
        return static_cast<GLWidget*>(m_widgetManager->mainGLWidget())->renderer();
    }
    Logger::Throw("No GL widget found containing a renderer");
    return nullptr;
}

SceneTreeWidget* CoreEngine::sceneTreeWidget()
{
    if (m_widgetManager) {
        return m_widgetManager->sceneTreeWidget();
    }
    else {
        return nullptr;
    }
}

ComponentTreeWidget * CoreEngine::componentWidget()
{
    if (m_widgetManager) {
        return m_widgetManager->componentWidget();
    }
    else {
        return nullptr;
    }
}

json CoreEngine::getScenarioJson() const
{
    // Allow access from multiple threads
    static std::mutex s_reentrantMutex;
    std::unique_lock lock(s_reentrantMutex);
    return *m_scenario;
}

void CoreEngine::setScenario(std::shared_ptr<Scenario> scenario)
{
    clearEngine(false, true);
    m_scenario = scenario;

    //static_cast<WidgetGateway*>(m_widgetManager->messageGateway())->scenarioChanged();
}

void CoreEngine::clearEngine(bool clearActionStack, bool clearScenario)
{
    // Clear the undo stack
    if (clearActionStack) {
        m_actionManager->undoStack()->clear();
    }

    // Clear the process manager
    processManager()->clearProcesses();

    // Clear the resource cache
    m_resourceCache->clear();

    // Clear uniforms
    /// @todo Split UBO uniform storage from other storage so that this can be cleared
    //UniformContainer::Instance().clear();

    // Clear the physics manager, except for default shape 
    PhysicsManager::Clear(false);

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

    // Clear the scenario
    if (m_scenario && clearScenario) {
        m_scenario = nullptr;
    }

    // Clear uniform buffer objects that are not core UBOs
    Ubo::ClearUBOs(false);

    // Clear Lights
    openGlRenderer()->renderContext().reset();

    // Clear material count
    Material::clearCount();

}

QOpenGLContext* CoreEngine::getGLWidgetContext()
{
    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (context != widgetManager()->mainGLWidget()->context()) {
        m_widgetManager->mainGLWidget()->makeCurrent();
    }
    return widgetManager()->mainGLWidget()->context();
}

void CoreEngine::setGLContext()
{
    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (context != widgetManager()->mainGLWidget()->context()) {
        m_widgetManager->mainGLWidget()->makeCurrent();
    }
}

void CoreEngine::setNewScenario()
{
    if (!m_simulationLoop) {
        // Need to feed a persistant core engine shared_ptr to a scenario
        Logger::Throw("Error, must initialize simulation loop before creating a scenario");
    }
    setScenario(std::make_shared<Scenario>(this));
}

void CoreEngine::initialize(MainWindow* mainWindow)
{
    // Initialize error logging
    initializeLogger();

    // Throw error if there isn't a main window
    if (!mainWindow) {
        Logger::LogWarning("Warning, no main window in this application");
        Logger::Throw("Warning, no main window in this application");
    }

    // Initialize Managers
    m_fileManager = new FileManager(this);
    m_fontManager = new FontManager(FileManager::GetResourcePath());
    m_widgetManager = new WidgetManager(mainWindow);
    m_actionManager = new ActionManager("Action Manager");
    m_widgetManager->setActionManager(m_actionManager);

    m_processManager = new ProcessManager(this, MultithreadedProcessQueue::s_defaultThreadCount);
    m_simulationLoop = new SimulationLoop(this, 10); // max 100 FPS
    m_physicsManager = new PhysicsManager(this);
    m_debugManager = new DebugManager(this);
    m_soundManager = new SoundManager(this);
    m_animationManager = new AnimationManager(this);

    // Create singleton resource cache
    m_resourceCache = &ResourceCache::Create(this, m_processManager, MemoryMonitor::GetMaxMemoryMb(0.25));

    // Create eventManager
    m_eventManager = new EventManager(this);

    /// Initialize gateways
    Uint32_t applicationPort = GApplicationSettings::Instance().applicationGatewayReceivePort();
    Uint32_t widgetPort = GApplicationSettings::Instance().widgetGatewayReceivePort();

    // Initialize communication server settings for application
    Uint64_t systemBeatUs = 10e3;// 10ms = 100 FPS
    GMessageGateway::ServerSettings applicationServerSettings;
    applicationServerSettings.m_listenerEndpoint = NetworkEndpoint::Any(applicationPort);
    applicationServerSettings.m_destinationEndpoint = NetworkEndpoint::Loopback(widgetPort);
    applicationServerSettings.m_sendTimeMicroseconds = systemBeatUs;
    applicationServerSettings.m_clientConnectionTimeoutMicroSeconds = 5e6;

    m_applicationGateway = std::make_unique<ApplicationGateway>(this);
    m_applicationGateway->initializeConnections();

    // Need to initialize server on a separate thread so that initialization operations are non-blocking
    //applicationGateway->initializeServer(applicationServerSettings);
    //std::chrono::system_clock::time_point tenSecondsElapsed = std::chrono::system_clock::now() + std::chrono::seconds(10);
    ThreadPool threadPool{ 2 };
    std::future<void> completes = threadPool.addTask(
        &ApplicationGateway::initializeServer,
        m_applicationGateway.get(),
        applicationServerSettings);

    // Initialize communication server settings for widgets
    GMessageGateway::ServerSettings widgetServerSettings;
    widgetServerSettings.m_listenerEndpoint = NetworkEndpoint::Any(widgetPort);
    widgetServerSettings.m_destinationEndpoint = NetworkEndpoint::Loopback(applicationPort);
    widgetServerSettings.m_sendTimeMicroseconds = systemBeatUs;
    widgetServerSettings.m_clientConnectionTimeoutMicroSeconds = 5e6;

    auto widgetGateway = new WidgetGateway(m_widgetManager);
    m_widgetManager->setMessageGateway(widgetGateway);

    // Need to initialize server on a separate thread so that initialization operations are non-blocking
    //widgetGateway->initializeServer(widgetServerSettings);
    std::future<void> completesWidgets = threadPool.addTask(
        &WidgetGateway::initializeServer,
        widgetGateway,
        widgetServerSettings);

    // Wait until gateways are initialized, then shut down those threads
    //completes.wait_until(tenSecondsElapsed); // Waits until either done, or 10 seconds pass
    //completesWidgets.wait_until(tenSecondsElapsed); // Waits until either done, or 10 seconds pass
    threadPool.shutdown(); // Similar thread locking to futures
}

void CoreEngine::postConstruction()
{
#ifdef DEBUG_MODE    
    gl::OpenGLFunctions::printGLError("Error before setting context");
#endif

    // Perform post-construction for managers
    m_fontManager->postConstruction();
    m_resourceCache->postConstruction();
    processManager()->postConstruction();
    m_simulationLoop->postConstruction();
    m_animationManager->postConstruction();

    // Load built-in shader presets
    ShaderPreset::InitializeBuiltins(this);

    // Debug manager
    m_debugManager->postConstruction();

#ifdef DEBUG_MODE    
    gl::OpenGLFunctions::printGLError("Error after setting context");
#endif

    // Set default scenario
    GApplicationSettings& settings = GApplicationSettings::Instance();
    bool loaded = false;
    if (!settings.getRecentProject().isEmpty()) {
        GString filepath = settings.getRecentProject();
#ifdef DEBUG_MODE
        loaded = Scenario::LoadFromFile(filepath, this);
#else
        try {
            loaded = Scenario::LoadFromFile(filepath, this);
        }
        catch (std::exception e) {
            Logger::LogWarning("Warning, failed to load file at " + filepath);
        }
#endif

        // Set window title
        mainWindow()->setWindowTitle(mainWindow()->getDefaultTitle() + " " + filepath);
    }

    if(!loaded) {
        // Initialize a blank scenario if load failed
        setNewScenario();
    }

    // Initialize Python console
    // FIXME: Fix memory leak (just delete window after app closes)
    //QWidget* window = new QWidget();
    //PythonQtScriptingConsole* console = new PythonQtScriptingConsole(window, PythonQt::self()->getMainModule());
    //    window->setWindowTitle("Python Debug Console");
    //window->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    //window->show();

    // Once scenario is loaded, ensure that framebuffers are resized correctly
    m_widgetManager->postConstruction();

    // Engine is fully constructed
    m_isConstructed = true;
}

bool CoreEngine::event(QEvent * event)
{
    if (event->type() == LogEvent::EVENT_TYPE)
    {
        // This is a log event
        LogEvent *logEvent = static_cast<LogEvent *>(event);
        Logger::LogMessage(logEvent->getLogLevel(), 
            logEvent->getNamespaceName(),
            logEvent->getLogMessage());
    }

    return QObject::event(event);
}


void CoreEngine::initializeLogger()
{
    // Create logger
    auto& logger = rev::Logger::Instance();
    logger.setLevel(LogLevel::Debug);

    Logger::getFileLogger("reverie", "AppFileLogger");

    Logger::getCustomLogHandler<ConsoleTool>("ConsoleTool", LogLevel::Debug);

    Logger::getStdOutHandler("StandardOut");

}

void CoreEngine::initializeMetaTypes()
{
    // Register types to send over qt signals/slots
    qRegisterMetaType<Uuid>("Uuid");
    qRegisterMetaType<Uuid>("rev::Uuid");
}

void CoreEngine::initializePythonAPI()
{
    // Initialize custom Python API
    rev::PythonAPI::Create("python");

    //py::print("Hello, World!"); // use the Python API
}

void CoreEngine::initializeConnections()
{
    // Perform post-construction on GL widget initialization
    connect(m_widgetManager->mainGLWidget(),
        &GLWidget::initializedContext,
        this,
        &CoreEngine::postConstruction);
}


CoreEngine* CoreEngine::s_coreEngine{ nullptr };

// End namespaces
}