#include "GbCoreEngine.h"
#include "GbLogger.h"

#include "../GbMainWindow.h"

#include "events/GbEvent.h"

#include "../third_party/pythonqt/gui/PythonQtScriptingConsole.h"
#include "scripting/GbPythonAPI.h"
#include "scripting/GbPythonScript.h"
#include "scripting/GbPyWrappers.h"
#include "geometry/GbEulerAngles.h"

#include "components/GbTransformComponent.h"
#include "components/GbLightComponent.h"
#include "components/GbCamera.h"
#include "components/GbShaderComponent.h"
#include "rendering/renderer/GbRenderers.h"
#include "rendering/shaders/GbShaders.h"
#include "rendering/shaders/GbUniformBufferObject.h"
#include "components/GbPhysicsComponents.h"

#include "loop/GbSimLoop.h"
#include "scene/GbScenario.h"
#include "scene/GbScene.h"
#include "scene/GbSceneObject.h"
#include "processes/GbProcessManager.h"
#include "events/GbEventManager.h"
#include "input/GbInputHandler.h"
#include "physics/GbPhysicsManager.h"

#include "rendering/renderer/GbMainRenderer.h"
#include "rendering/renderer/GbRenderContext.h"

#include "canvas/GbFonts.h"
#include "debugging/GbDebugManager.h"

#include "../model_control/commands/GbActionManager.h"
#include "../view/GbWidgetManager.h"
#include "../view/tree/GbSceneTreeWidget.h"
#include "../view/tree/GbComponentWidget.h"
#include "../view/GL/GbGLWidget.h"
#include "../view/logging/GbConsoleTool.h"

#include "resource/GbResourceCache.h"
#include "utils/GbMemoryManager.h"
#include "utils/GbParallelization.h"

#include "readers/models/GbModelReader.h"

#define NUMBER_OF_HELPER_THREADS std::thread::hardware_concurrency()

namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int CoreEngine::THREAD_COUNT = 0;

/////////////////////////////////////////////////////////////////////////////////////////////
ThreadPool CoreEngine::HELPER_THREADPOOL(NUMBER_OF_HELPER_THREADS);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CoreEngine::CoreEngine():
    QObject(nullptr),
    Object("Engine_" + QString::number(ENGINES.size())),
    m_isConstructed(false),
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

    ENGINES[m_name] = this;
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
    ENGINES.erase(m_name);
    PythonQt::cleanup();
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
    if(clearActionStack)
        m_actionManager->undoStack()->clear();

    // Clear the resource cache
    m_resourceCache->clear();

    // Clear the process manager
    processManager()->clearAllProcesses();

    // Clear the physics manager
    physicsManager()->clear();

    // Clear the debug manager
    debugManager()->clear();

    // Clear the widget manager
    widgetManager()->clear();

    // Clear the Python API
    PythonAPI::get()->clear();

    // Clear the scenario
    if (m_scenario && clearScenario) {
        m_scenario = nullptr;
    }

    // Clear uniform buffer objects
    UBO::clearUBOs();

    // Clear Lights
    // FIXME: Lights don't toggle on when switching scenarios
    mainRenderer()->renderContext().lightingSettings().clearLights();

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
        auto& logger = Gb::Logger::getInstance();
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
    GL::OpenGLFunctions functions;
    functions.printGLError("Error before setting context");
#endif

    // Perform post-construction for managers
    m_fontManager->postConstruction();
    m_resourceCache->postConstruction();
    m_debugManager->postConstruction();

#ifdef DEBUG_MODE    
    functions.printGLError("Error after setting context");
#endif

    m_isConstructed = true;

    // Set default scenario
    auto* settings = new Gb::Settings::INISettings();
    if (!settings->getRecentProject().isEmpty()) {
        QString filepath = settings->getRecentProject();
#ifdef DEBUG_MODE
        Scenario::loadFromFile(filepath, this);

        // Set window title
        mainWindow()->setWindowTitle(mainWindow()->getDefaultTitle() + " " + filepath);
#else
        try {
            Scenario::loadFromFile(filepath, this);
        }
        catch (std::exception e) {
            logWarning("Warning, failed to load file at " + filepath);
        }
#endif
    }
    else {
        // Initialize a blank scenario
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
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CoreEngine::event(QEvent * event)
{
    if (event->type() == LogEvent::EVENT_TYPE)
    {
        // This is a log event
        LogEvent *logEvent = static_cast<LogEvent *>(event);
        logMessage(logEvent->getLogLevel(), 
            logEvent->getNamespaceName().toUtf8().constData(),
            logEvent->getLogMessage().toUtf8().constData());
    }

    return QObject::event(event);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoreEngine::initializeLogger()
{
    // Create logger
    auto& logger = Gb::Logger::getInstance();
    logger.setLevel(LogLevel::Debug);

    Logger::getFileLogger("grand_blue", "AppFileLogger");

    Logger::getVSLogger("VisualStudio");

    Logger::getConsoleTool("ConsoleTool");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoreEngine::initializeMetaTypes()
{
    // Register types to send over qt
    bool succeeded = true;
    qRegisterMetaType<Uuid>("Uuid");
    qRegisterMetaType<Uuid>("Gb::Uuid");

    qRegisterMetaType<std::shared_ptr<Object>>();

    // Python Types
    // TODO: Encapsulate initialization into a template
    qRegisterMetaType<PythonBehavior>("ResourceCache");
    qRegisterMetaType<PythonBehavior>("Gb::ResourceCache");

    qRegisterMetaType<PythonBehavior>("PythonBehavior");
    qRegisterMetaType<PythonBehavior>("Gb::PythonBehavior");

    qRegisterMetaType<SceneObject>("SceneObject");
    qRegisterMetaType<SceneObject>("Gb::SceneObject");

    qRegisterMetaType<CustomEvent>("CustomEvent");
    qRegisterMetaType<CustomEvent>("Gb::CustomEvent");

    qRegisterMetaType<Vector2>("Vector2");
    qRegisterMetaType<Vector2>("Gb::Vector2");
    qRegisterMetaType<Vector3>("Vector3");
    qRegisterMetaType<Vector3>("Gb::Vector3");
    qRegisterMetaType<Vector4>("Vector4");
    qRegisterMetaType<Vector4>("Gb::Vector4");

    qRegisterMetaType<Matrix2x2>("Matrix2x2");
    qRegisterMetaType<Matrix2x2>("Gb::Matrix2x2");
    qRegisterMetaType<Matrix3x3>("Matrix3x3");
    qRegisterMetaType<Matrix3x3>("Gb::Matrix3x3");
    qRegisterMetaType<Matrix4x4>("Matrix4x4");
    qRegisterMetaType<Matrix4x4>("Gb::Matrix4x4");

    qRegisterMetaType<Scene>("Scene");
    qRegisterMetaType<Scene>("Gb::Scene");

    qRegisterMetaType<Scenario>("Scenario");
    qRegisterMetaType<Scenario>("Gb::Scenario");

    qRegisterMetaType<LightComponent>("LightComponent");
    qRegisterMetaType<LightComponent>("Gb::LightComponent");

    qRegisterMetaType<ShaderComponent>("ShaderComponent");
    qRegisterMetaType<ShaderComponent>("Gb::ShaderComponent");

    qRegisterMetaType<ShaderProgram>("ShaderProgram");
    qRegisterMetaType<ShaderProgram>("Gb::ShaderProgram");

    qRegisterMetaType<CameraComponent>("CameraComponent");
    qRegisterMetaType<CameraComponent>("Gb::CameraComponent");

    qRegisterMetaType<TransformComponent>("TransformComponent");
    qRegisterMetaType<TransformComponent>("Gb::TransformComponent");

    qRegisterMetaType<TranslationComponent>("TranslationComponent");
    qRegisterMetaType<TranslationComponent>("Gb::TranslationComponent");

    qRegisterMetaType<RotationComponent>("RotationComponent");
    qRegisterMetaType<RotationComponent>("Gb::RotationComponent");

    qRegisterMetaType<ScaleComponent>("ScaleComponent");
    qRegisterMetaType<ScaleComponent>("Gb::ScaleComponent");

    qRegisterMetaType<CharControlComponent>("CharControlComponent");
    qRegisterMetaType<CharControlComponent>("Gb::CharControlComponent");

    qRegisterMetaType<EulerAngles>("EulerAngles");
    qRegisterMetaType<EulerAngles>("Gb::EulerAngles");

    qRegisterMetaType<Quaternion>("Quaternion");
    qRegisterMetaType<Quaternion>("Gb::Quaternion");

    qRegisterMetaType<InputHandler>("InputHandler");
    qRegisterMetaType<InputHandler>("Gb::InputHandler");

    qRegisterMetaType<KeyHandler>("KeyHandler");
    qRegisterMetaType<KeyHandler>("Gb::KeyHandler");

    qRegisterMetaType<MouseHandler>("MouseHandler");
    qRegisterMetaType<MouseHandler>("Gb::MouseHandler");

    if (!succeeded) {
        throw("Error, failed to register metatypes");
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoreEngine::initializePythonAPI()
{
    // Initialize PythonQt
    // Flags: IgnoreSiteModule ~ cannot load modules from site packages
    PythonQt::init();

    // Set importer
    // Removed, was breaking package imports
    //PythonQt::self()->installDefaultImporter();

    // Initialize custom Python API
    Gb::PythonAPI* api = Gb::PythonAPI::get();
    Q_UNUSED(api)

    // Register types that are python-compatible
    // NOTE: Type strings must match class name
    // TODO: Creata a helper class to wrap up these registrations

    int typeFlag = int(
        PythonQt::Type_Add |
        PythonQt::Type_Subtract |
        PythonQt::Type_Multiply |
        PythonQt::Type_InplaceAdd |
        PythonQt::Type_InplaceSubtract |
        PythonQt::Type_InplaceMultiply |
        PythonQt::Type_RichCompare);

    PythonQt::self()->registerCPPClass("PythonBehavior",
        "Object",
        "scriptedBehaviors",
        PythonQtCreateObject<PythonBehaviorWrapper>);

    PythonQt::self()->registerCPPClass("CoreEngine",
        "Object",
        "core",
        PythonQtCreateObject<PyEngine>);

    PythonQt::self()->registerCPPClass("ResourceCache",
        "Manager",
        "core",
        PythonQtCreateObject<PyResourceCache>);

    PythonQt::self()->registerCPPClass("CustomEvent",
        "Serializable",
        "events",
        PythonQtCreateObject<PyCustomEvent>);


    //QString className = GbEngine::staticMetaObject.className();
    //PythonQt::self()->registerClass(&CoreEngine::staticMetaObject,
    //    "core",
    //    PythonQtCreateObject<PyEngine>);

    PythonQt::self()->priv()->registerCPPClass("Vector2",
        "",
        "alg",
        PythonQtCreateObject<PyVector2>,
        NULL,
        NULL,
        typeFlag);

    PythonQt::self()->priv()->registerCPPClass("Vector3",
        "",
        "alg",
        PythonQtCreateObject<PyVector3>,
        NULL,
        NULL,
        typeFlag);

    PythonQt::self()->priv()->registerCPPClass("Vector4",
        "",
        "alg",
        PythonQtCreateObject<PyVector4>,
        NULL,
        NULL,
        typeFlag);

    PythonQt::self()->priv()->registerCPPClass("Matrix2x2",
        "",
        "alg",
        PythonQtCreateObject<PyMatrix2x2>,
        NULL,
        NULL,
        typeFlag);

    PythonQt::self()->priv()->registerCPPClass("Matrix3x3",
        "",
        "alg",
        PythonQtCreateObject<PyMatrix3x3>,
        NULL,
        NULL,
        typeFlag);

    PythonQt::self()->priv()->registerCPPClass("Matrix4x4",
        "",
        "alg",
        PythonQtCreateObject<PyMatrix4x4>,
        NULL,
        NULL,
        typeFlag);

    PythonQt::self()->registerCPPClass("SceneObject",
        "",
        "scene",
        PythonQtCreateObject<PySceneObject>);

    PythonQt::self()->registerCPPClass("Scene",
        "",
        "scene",
        PythonQtCreateObject<PyScene>);

    PythonQt::self()->registerCPPClass("Scenario",
        "",
        "scene",
        PythonQtCreateObject<PyScenario>);

    PythonQt::self()->registerCPPClass("LightComponent",
        "Component",
        "components",
        PythonQtCreateObject<PyLight>);

    PythonQt::self()->registerCPPClass("CameraComponent",
        "Component",
        "components",
        PythonQtCreateObject<PyCamera>);

    PythonQt::self()->registerCPPClass("ShaderComponent",
        "Component",
        "components",
        PythonQtCreateObject<PyMaterial>);

    PythonQt::self()->registerCPPClass("CharControlComponent",
        "Component",
        "components",
        PythonQtCreateObject<PyCharacterController>);

    PythonQt::self()->registerCPPClass("ShaderProgram",
        "",
        "resources",
        PythonQtCreateObject<PyShaderProgram>);

    PythonQt::self()->registerCPPClass("TransformComponent",
        "Object",
        "components",
        PythonQtCreateObject<PyTransformComponent>);

    PythonQt::self()->registerCPPClass("TranslationComponent",
        "AffineComponent",
        "components",
        PythonQtCreateObject<PyTranslationComponent>);

    PythonQt::self()->registerCPPClass("RotationComponent",
        "AffineComponent",
        "components",
        PythonQtCreateObject<PyRotationComponent>);

    PythonQt::self()->registerCPPClass("ScaleComponent",
        "AffineComponent",
        "components",
        PythonQtCreateObject<PyScaleComponent>);

    PythonQt::self()->registerCPPClass("EulerAngles",
        "",
        "alg",
        PythonQtCreateObject<PyEulerAngles>);

    PythonQt::self()->priv()->registerCPPClass("Quaternion",
        "Serializable",
        "alg",
        PythonQtCreateObject<PyQuaternion>,
        NULL,
        NULL,
        typeFlag);

    PythonQt::self()->registerCPPClass("InputHandler",
        "",
        "input",
        PythonQtCreateObject<PyInputHandler>);

    PythonQt::self()->registerCPPClass("KeyHandler",
        "",
        "input",
        PythonQtCreateObject<PyKeyHandler>);

    PythonQt::self()->registerCPPClass("MouseHandler",
        "",
        "input",
        PythonQtCreateObject<PyMouseHandler>);
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
std::unordered_map<QString, CoreEngine*> CoreEngine::ENGINES = {};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}