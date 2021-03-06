#include "GScenario.h"

#include <QJsonDocument>
#include <QMessageBox>

#include "../GCoreEngine.h"
//#include "GScene.h"
#include "GSceneObject.h"

#include "../readers/GFileReader.h"
#include "../readers/GJsonReader.h"

#include "../resource/GFileManager.h"
#include "../resource/GResourceCache.h"
#include "../processes/GProcessManager.h"
#include "../physics/GPhysicsManager.h"
#include "../../view/GWidgetManager.h"
#include "../sound/GSoundManager.h"
#include "../../view/GL/GGLWidget.h"
#include "../../GMainWindow.h"
#include "../scripting/GPythonScript.h"
#include "../animation/GAnimationManager.h"

#include "../debugging/GDebugManager.h"
#include "../rendering/shaders/GShaderPreset.h"
#include "../rendering/renderer/GMainRenderer.h"
#include "../components/GCameraComponent.h"
#include "../loop/GSimLoop.h"

namespace rev{

///////////////////////////////////////////////////////////////////////////////////////////////////
// Scenario
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Scenario::LoadFromFile(const GString& filepath, CoreEngine* core) {
    JsonReader reader(filepath.c_str());
    bool exists = reader.FileExists();
    if (exists) {
        QJsonObject json = reader.getContentsAsJsonObject();
        if (!json.isEmpty()) {
            // Don't want any updates while scenario is loading?
            //QMutexLocker lock(&core->simulationLoop()->updateMutex());
            auto scenario = std::make_shared<Scenario>(core);
            core->setScenario(scenario);
            scenario->m_isLoading = true;
            scenario->loadFromJson(json);
            scenario->setPath(filepath);
            scenario->m_isLoading = false;
        }
        else {
            core->logError("Failed to load scenario from file, invalid JSON");
        }
    }

    return exists;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
Scenario::Scenario() :
    QObject(nullptr),
    m_engine(nullptr),
    m_settings(this)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Scenario::Scenario(const Scenario & scenario):
//    QObject(nullptr),
//    m_settings(scenario.m_settings),
//    m_engine(scenario.m_engine)
//{
//    m_settings.m_scenario = this;
//}
///////////////////////////////////////////////////////////////////////////////////////////////////
Scenario::Scenario(CoreEngine* core):
    QObject(nullptr),
    m_engine(core),
    m_settings(this)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
Scenario::~Scenario()
{
    clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//const AABB & Scenario::getVisibleFrustumBounds()
//{
//    return m_visibleBounds;
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//void Scenario::updateVisibleFrustumBounds()
//{
//    m_visibleBounds = AABB();
//    for (const std::shared_ptr<Scene>& scene : s_scenes) {
//        scene->updateVisibleFrustumBounds();
//        const AABBData& sceneViewBounds = scene->getVisibleFrustumBounds().boxData();
//        m_visibleBounds.resize(std::vector<Vector4>{ sceneViewBounds.m_max, sceneViewBounds.m_min });
//    }
//}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Scenario::isVisible(const AABB& boundingBox)
{
    if (m_engine->simulationLoop()->getPlayMode() == SimulationLoop::kDebug) {
        CameraComponent* debugCamComp = m_engine->debugManager()->camera();
        if (!debugCamComp) {
            // On scenario load, debug camera might not yet be initialized
            return false;
        }
        else {
            return debugCamComp->camera().canSee(boundingBox);
        }
    }
    else {
        for (CameraComponent* camera : m_scene.cameras()) {
            if (camera->camera().canSee(boundingBox)) {
                return true;
            }
        }
    }
    return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Scenario::save()
{
    return save(m_path);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Scenario::save(const GString & filepath)
{
    QJsonObject jsonScenario = asJson().toObject();
    QFile scenarioFile(filepath.c_str());
    scenarioFile.open(QIODevice::WriteOnly);

    QByteArray bytes = QJsonDocument(jsonScenario).toJson();

    if (!scenarioFile.isOpen())
    {
        QMessageBox::about(m_engine->widgetManager()->mainWindow(),
            tr("Error"),
            tr("Error, failed to open scenario file"));
        logWarning("Warning, failed to save to scenario file");
        return false;
    }

    setPath(filepath);

    QTextStream outStream(&scenarioFile);
    outStream.setCodec("UTF-8"); // Absolutely necessary, especially for font-awesome characters
    outStream << bytes;
    scenarioFile.flush();
    scenarioFile.close();

    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Scenario::clear()
{
    m_scene.clear();

    SceneObject::Clear();

    emit clearedSceneObjects();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Scenario::asJson(const SerializationContext& context) const
{
    // Create JSON object
    //QJsonObject jsonObject = Loadable::asJson(context).toObject(); // don't need filepath
    QJsonObject jsonObject;
    jsonObject.insert("name", m_name.c_str());

    // Add process sorting layers
    QJsonObject processManager = m_engine->processManager()->asJson().toObject();
    jsonObject.insert("processManager", processManager);

    // Add physics manager and static physics resources
    QJsonValue physicsManager = m_engine->physicsManager()->asJson();
    jsonObject.insert("physicsManager", physicsManager);

    // Add scene
    jsonObject.insert("scene", m_scene.asJson());

    // Add blueprints
    QJsonArray blueprints;
    for (const Blueprint& bp : m_blueprints) {
        blueprints.append(bp.asJson());
    }
    jsonObject.insert("blueprints", blueprints);

    // Add resources from resource cache
    QJsonObject resourceCache = m_engine->resourceCache()->asJson().toObject();
    jsonObject.insert("resourceCache", resourceCache);

    // Settings
    jsonObject.insert("settings", m_settings.asJson());

    // Debug manager
    jsonObject.insert("debug", m_engine->debugManager()->asJson());

    // Sound manager
    jsonObject.insert("sound", m_engine->soundManager()->asJson());

    // Animation manager
    jsonObject.insert("animation", m_engine->animationManager()->asJson());

    // Save file manager
    jsonObject.insert("fileManager", m_engine->fileManager()->asJson());

    return jsonObject;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Scenario::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    // Get JSON as object
    const QJsonObject& jsonObject = json.toObject();

    // Don't load loadable class attributes, filepath is already known
#ifdef DEBUG_MODE
    QString jsonStr = JsonReader::ToString<QString>(jsonObject);
#endif

    int progressSize = 6;
    QProgressDialog progress("Loading Scenario...",
        "Abort Load", 0, progressSize, nullptr);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    // Set scenario name
    int count = 0;
    if (jsonObject.contains("name")) {
        m_name = jsonObject.value("name").toString();
        progress.setValue(count++);
    }

    // Load settings
    if (jsonObject.contains("settings")) {
        m_settings.loadFromJson(jsonObject.value("settings").toObject());
        progress.setValue(count++);
    }

    // Load search paths
    if (jsonObject.contains("fileManager")) {
        m_engine->fileManager()->loadFromJson(jsonObject["fileManager"]);
    }

    // Load process order
    if (jsonObject.contains("processManager")) {
        m_engine->processManager()->loadFromJson(jsonObject.value("processManager"));
        progress.setValue(count++);
    }

    // Load resources
    if (jsonObject.contains("resourceCache")) {
        m_engine->resourceCache()->loadFromJson(jsonObject.value("resourceCache"));
        progress.setValue(count++);
    }

    // Load physics resources
    if (jsonObject.contains("physicsManager")) {
        m_engine->physicsManager()->loadFromJson(jsonObject.value("physicsManager"));
        progress.setValue(count++);
    }

    // Load animation manager
    if (jsonObject.contains("animation")) {
        m_engine->animationManager()->loadFromJson(jsonObject["animation"]);
    }

    // Load blueprints
    if (jsonObject.contains("blueprints")) {
        QJsonArray& bps = jsonObject["blueprints"].toArray();
        for (const auto& bp : bps) {
            m_blueprints.emplace_back();
            m_blueprints.back().loadFromJson(bp);
        }
    }

    // Load scenes
    if (jsonObject.contains("scenes")) {
        // Deprecated 2/9/2021, now there is only one scene per scenario
        // Add scenes to scenario
        m_scene.m_scenario = this;
        m_scene.m_engine = m_engine;
        m_scene.loadFromJson(jsonObject.value("scenes").toArray()[0]);
    }
    else if(jsonObject.contains("scene")){
        m_scene.m_scenario = this;
        m_scene.m_engine = m_engine;
        m_scene.loadFromJson(jsonObject.value("scene"));
    }
    else {
        throw("no scenes found");
    }
    progress.setValue(count++);

    // Load Debug manager
    if (jsonObject.contains("debug")) {
        m_engine->debugManager()->loadFromJson(jsonObject.value("debug"));
    }

    // Load sound manager
    if (jsonObject.contains("sound")) {
        m_engine->soundManager()->loadFromJson(jsonObject["sound"]);
    }
    
    // Emit signal that scenario changed
    emit m_engine->scenarioChanged();
	emit m_engine->scenarioLoaded();
    emit m_engine->scenarioNeedsRedraw();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Scenario::resizeCameras(size_t width, size_t height) {
    std::vector<CameraComponent*>& cameras = m_scene.cameras();
    size_t numCameras = cameras.size();
    for (size_t i = 0; i < numCameras; i++) {
        // Update camera's framebuffer and light cluster grid sizes
        SceneCamera& cam = cameras[i]->camera();
        cam.resizeFrame(width, height);
        cam.lightClusterGrid().onResize();
    }

    SceneCamera& debugCam = m_engine->debugManager()->camera()->camera();
    debugCam.resizeFrame(width, height);
    debugCam.lightClusterGrid().onResize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Scenario::initialize() {
    // Initialize path to scenario and name
    m_path = "";
    m_name = QStringLiteral("New Scenario");
    if (SCENARIO_COUNT > 0) {
        m_name += " " + QString::number(SCENARIO_COUNT);
    }

    // Make sure that scene has the pointers it needs
    m_scene.m_engine = m_engine;
    m_scene.m_scenario = this;

    // Connect to widgets
    connect(m_engine->mainRenderer()->widget(), &View::GLWidget::resized,
        this, &Scenario::resizeCameras);

    connect(m_engine, &CoreEngine::scenarioLoaded,
        this, [this]() {
        View::GLWidget* widget = m_engine->mainRenderer()->widget();
        resizeCameras(widget->width(), widget->height());
    });

    // Update scene object as necessary when resources are loaded
    connect(m_engine->resourceCache(), &ResourceCache::doneLoadingResource, this,
        &Scenario::onResourceLoaded);

    // Increment scenario count
    SCENARIO_COUNT++;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Scenario::onResourceLoaded(const Uuid & resourceId)
{
    std::shared_ptr<ResourceHandle> handle = m_engine->resourceCache()->getHandle(resourceId);
    if (!handle) {
        throw("Resource not found");
    }

    switch (handle->getResourceType()) {
    case ResourceType::kModel:
    {
        for (const std::shared_ptr<SceneObject>& so : m_scene.topLevelSceneObjects()) {
            so->onModelLoaded(handle);
        }
        break;
    }
    default:
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int Scenario::SCENARIO_COUNT = 0;


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}