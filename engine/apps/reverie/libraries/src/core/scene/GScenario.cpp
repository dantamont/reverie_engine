#include "core/scene/GScenario.h"

#include <QJsonDocument>
#include <QMainWindow>
#include <QMessageBox>

#include "fortress/json/GJson.h"
#include "fortress/system/path/GPath.h"

#include "core/GCoreEngine.h"
#include "core/scene/GSceneObject.h"

#include "core/readers/GFileReader.h"

#include "core/resource/GFileManager.h"
#include "core/resource/GResourceCache.h"
#include "core/processes/GProcessManager.h"
#include "core/physics/GPhysicsManager.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "core/sound/GSoundManager.h"
#include "core/scripting/GPythonScript.h"
#include "core/animation/GAnimationManager.h"

#include "core/debugging/GDebugManager.h"
#include "core/rendering/shaders/GShaderPreset.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/components/GCameraComponent.h"
#include "core/loop/GSimLoop.h"

#include "core/layer/view/widgets/graphics/GGLWidget.h"

#include "logging/GLogger.h"
#include "ripple/network/messages/GScenarioJsonMessage.h"


namespace rev{


// Scenario

bool Scenario::LoadFromFile(const GString& filepath, CoreEngine* core) {
    bool exists = GPath::Exists(filepath);
    if (exists) {
        json myJson;
        GJson::FromFile(filepath, myJson);
        if (!myJson.empty()) {
            // Don't want any updates while scenario is loading?
            auto scenario = std::make_shared<Scenario>(core);
            core->setScenario(scenario);
            scenario->m_isLoading = true;
            myJson.get_to(*scenario);
            scenario->setPath(filepath);
            scenario->m_isLoading = false;
        }
        else {
            Logger::LogError("Failed to load scenario from file, invalid JSON");
        }
    }

    return exists;
}

Scenario::Scenario() :
    QObject(nullptr),
    m_engine(nullptr),
    m_settings(this)
{
}

Scenario::Scenario(CoreEngine* core):
    QObject(nullptr),
    m_engine(core),
    m_settings(this)
{
    initialize();
}

Scenario::~Scenario()
{
    clear();
    m_engine->s_scenarioLoaded.disconnect(m_signalIndex);
}

bool Scenario::isVisible(const AABB& boundingBox)
{
    if (m_engine->simulationLoop()->getPlayMode() == ESimulationPlayMode::eDebug) {
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

bool Scenario::save()
{
    return save(m_path);
}

bool Scenario::save(const GString & filepath)
{
    // Check if file exists
    json jsonScenario = *this;
    QFile scenarioFile(filepath.c_str());
    scenarioFile.open(QIODevice::WriteOnly);

    if (!scenarioFile.isOpen())
    {
        QMessageBox::about(m_engine->widgetManager()->mainWindow(),
            tr("Error"),
            tr("Error, failed to open scenario file"));
        Logger::LogWarning("Warning, failed to save to scenario file");
        return false;
    }
    scenarioFile.flush();
    scenarioFile.close();

    setPath(filepath);

    //QTextStream outStream(&scenarioFile);
    //outStream.setCodec("UTF-8"); // Absolutely necessary, especially for font-awesome characters
    //outStream << bytes;
    //scenarioFile.flush();
    //scenarioFile.close();

    // Actually write to file
    std::ofstream file(filepath.c_str());
    file << std::setw(4) << jsonScenario << std::endl;

    return true;
}

void Scenario::clear()
{
    m_scene.clear();

    SceneObject::Clear();

    emit clearedSceneObjects();
}


void to_json(json& orJson, const Scenario& korObject)
{
    // Create JSON object
    //ToJson<LoadableInterface>(orJson, korObject); // don't need filepath
    orJson["name"] = korObject.m_name.c_str();

    // Add process sorting layers
    orJson["processManager"] = *korObject.m_engine->processManager();

    // Add physics manager and static physics resources
    orJson["physicsManager"] = *korObject.m_engine->physicsManager();

    // Add scene
    orJson["scene"] = korObject.m_scene;

    // Add blueprints
    json blueprints = json::array();
    for (const Blueprint& bp : korObject.m_blueprints) {
        blueprints.push_back(bp);
    }
    orJson["blueprints"] = blueprints;

    // Add resources from resource cache
    orJson["resourceCache"] = ResourceCache::Instance();

    // Settings
    orJson["settings"] = korObject.m_settings;

    // Debug manager
    orJson["debug"] = *korObject.m_engine->debugManager();

    // Sound manager
    orJson["sound"] = *korObject.m_engine->soundManager();

    // Animation manager
    orJson["animation"] = *korObject.m_engine->animationManager();

    // Save file manager
    orJson["fileManager"] = *korObject.m_engine->fileManager();
}

void from_json(const json& korJson, Scenario& orObject)
{
    // Don't load loadable class attributes, filepath is already known
#ifdef DEBUG_MODE
    GString jsonStr = GJson::ToString<GString>(korJson);
#endif

    int progressSize = 6;
    QProgressDialog progress("Loading Scenario...",
        "Abort Load", 0, progressSize, nullptr);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
	progress.raise();

    // Set scenario name
    int count = 0;
    if (korJson.contains("name")) {
        orObject.m_name = korJson.at("name").get_ref<const std::string&>().c_str();
        progress.setValue(count++);
    }

    // Load settings
    if (korJson.contains("settings")) {
        korJson.at("settings").get_to(orObject.m_settings);
        progress.setValue(count++);
    }

    // Load search paths
    if (korJson.contains("fileManager")) {
        korJson["fileManager"].get_to(*orObject.m_engine->fileManager());
    }

    // Load process order
    if (korJson.contains("processManager")) {
        korJson["processManager"].get_to(*orObject.m_engine->processManager());
        progress.setValue(count++);
    }

    // Load resources
    if (korJson.contains("resourceCache")) {
        korJson["resourceCache"].get_to(ResourceCache::Instance());
        progress.setValue(count++);
    }

    // Load physics resources
    if (korJson.contains("physicsManager")) {
        korJson["physicsManager"].get_to(*orObject.m_engine->physicsManager());
        progress.setValue(count++);
    }

    // Load animation manager
    if (korJson.contains("animation")) {
        korJson["animation"].get_to(*orObject.m_engine->animationManager());
    }

    // Load blueprints
    if (korJson.contains("blueprints")) {
        const json& bps = korJson["blueprints"];
        for (const json& bp : bps) {
            orObject.m_blueprints.emplace_back();
            bp.get_to(orObject.m_blueprints.back());
        }
    }

    // Load scenes
    if (korJson.contains("scenes")) {
        // Deprecated 2/9/2021, now there is only one scene per scenario
        // Add scenes to scenario
        orObject.m_scene.setScenario(&orObject);
        korJson.at("scenes")[0].get_to(orObject.m_scene);
    }
    else if(korJson.contains("scene")){
        orObject.m_scene.setScenario(&orObject);
        korJson.at("scene").get_to(orObject.m_scene);
    }
    else {
        Logger::Throw("no scenes found");
    }
    progress.setValue(count++);

    // Load Debug manager
    if (korJson.contains("debug")) {
        korJson.at("debug").get_to(*orObject.m_engine->debugManager());
    }

    // Load sound manager
    if (korJson.contains("sound")) {
        korJson["sound"].get_to(*orObject.m_engine->soundManager());
    }
    
    // Emit signal that scenario changed
	orObject.m_engine->s_scenarioLoaded.emitForAll();
    emit orObject.m_engine->scenarioNeedsRedraw();
}

void Scenario::resizeCameras(uint32_t width, uint32_t height) {
    std::vector<CameraComponent*>& cameras = m_scene.cameras();
    uint32_t numCameras = (uint32_t)cameras.size();
    for (uint32_t i = 0; i < numCameras; i++) {
        // Update camera's framebuffer and light cluster grid sizes
        SceneCamera& cam = cameras[i]->camera();
        cam.resizeFrame(width, height);
        cam.lightClusterGrid().onResize();
    }

    SceneCamera& debugCam = m_engine->debugManager()->camera()->camera();
    debugCam.resizeFrame(width, height);
    debugCam.lightClusterGrid().onResize();
}

void Scenario::initialize() {
    // Initialize path to scenario and name
    m_path = "";
    m_name = GString("New Scenario");
    if (SCENARIO_COUNT > 0) {
        m_name += " " + GString::FromNumber(SCENARIO_COUNT);
    }

    // Make sure that scene has the pointers it needs
    m_scene.m_engine = m_engine;
    m_scene.m_scenario = this;
    m_scene.initializeSignals();

    // Connect to widgets
    connect(m_engine->openGlRenderer()->widget(), &GLWidgetInterface::resized, this, &Scenario::resizeCameras);

    m_signalIndex = m_engine->s_scenarioLoaded.connect(
        [this]() {
            GLWidgetInterface* widget = m_engine->openGlRenderer()->widget();
            resizeCameras(widget->pixelWidth(), widget->pixelHeight());
        }
    );

    // Update scene object as necessary when resources are loaded
    connect(&ResourceCache::Instance(), &ResourceCache::doneLoadingResource, this,
        &Scenario::onResourceLoaded);

    // Increment scenario count
    SCENARIO_COUNT++;
}

void Scenario::onResourceLoaded(const Uuid & resourceId)
{
    std::shared_ptr<ResourceHandle> handle = ResourceCache::Instance().getHandle(resourceId);
    if (!handle) {
        Logger::Throw("Scenario::onResourceLoaded: Resource not found");
    }

    //switch (handle->getResourceType()) {
    //default:
    //    break;
    //}
}

unsigned int Scenario::SCENARIO_COUNT = 0;



// End namespaces        
}