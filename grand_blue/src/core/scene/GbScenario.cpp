#include "GbScenario.h"

#include <QJsonDocument>
#include <QMessageBox>

#include "../GbCoreEngine.h"
#include "GbScene.h"
#include "GbSceneObject.h"

#include "../readers/GbFileReader.h"
#include "../readers/GbJsonReader.h"

#include "../resource/GbResourceCache.h"
#include "../processes/GbProcessManager.h"
#include "../physics/GbPhysicsManager.h"
#include "../../view/GbWidgetManager.h"
#include "../sound/GbSoundManager.h"
#include "../../view/GL/GbGLWidget.h"
#include "../../GbMainWindow.h"
#include "../scripting/GbPythonScript.h"
#include "../animation/GbAnimationManager.h"

#include "../debugging/GbDebugManager.h"
#include "../rendering/shaders/GbShaderPreset.h"
#include "../rendering/renderer/GbMainRenderer.h"
#include "../components/GbCameraComponent.h"
#include "../loop/GbSimLoop.h"

namespace Gb{
///////////////////////////////////////////////////////////////////////////////////////////////////
ScenarioSettings::ScenarioSettings(Scenario * scenario) :
    m_scenario(scenario) {

    // Add core render layers
    addRenderLayer("skybox", -10);
    addRenderLayer("world", 0);
    addRenderLayer("effects", 5);
    addRenderLayer("ui", 10);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ScenarioSettings::asJson() const
{
    QJsonObject object = QJsonObject();
    
    QJsonArray renderLayers;
    for (const auto& renderLayer : m_renderLayers) {
        renderLayers.append(renderLayer->asJson());
    }
    object.insert("renderLayers", renderLayers);

    QJsonArray shaderPresets;
    for (const auto& preset : m_shaderPresets) {
        shaderPresets.append(preset->asJson());
    }
    object.insert("shaderPresets", shaderPresets);

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScenarioSettings::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context);

    m_renderLayers.clear();
    m_shaderPresets.clear();

    QJsonObject object = json.toObject();
    if (object.contains("renderLayers")) {
        QJsonArray renderLayers = object["renderLayers"].toArray();
        for (const auto& layerJson : renderLayers) {
            m_renderLayers.push_back(
                std::make_shared<SortingLayer>(layerJson));
        }
    }
    sortRenderLayers();

    //if (!m_renderLayers.size()) {
    //    // Ensure that there is always the default render layer
    //    m_renderLayers.push_back(std::make_shared<SortingLayer>());
    //}

    if (object.contains("shaderPresets")) { // legacy check
        const QJsonArray& shaderPresets = object.value("shaderPresets").toArray();
        for (const auto& shaderJson : shaderPresets) {
            auto shaderPreset = std::make_shared<ShaderPreset>(m_scenario->engine(), shaderJson);
            m_shaderPresets.push_back(shaderPreset);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SortingLayer> ScenarioSettings::addRenderLayer()
{
    QString uniqueName = Uuid::UniqueName();
    auto layer = std::make_shared<SortingLayer>();
    m_renderLayers.push_back(layer);
    m_renderLayers.back()->setName(uniqueName);
    sortRenderLayers();
    return layer;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SortingLayer> ScenarioSettings::addRenderLayer(const GString & name, int order)
{
    auto layer = std::make_shared<SortingLayer>(name, order);
    m_renderLayers.push_back(layer);
    sortRenderLayers();
    return layer;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool ScenarioSettings::removeRenderLayer(const GString & label)
{
    if (!renderLayer(label)) {
#ifdef DEBUG_MODE
        throw("Error, failed to find render layer matching the given label");
#endif
        return false;
    }

    auto iter = std::find_if(m_renderLayers.begin(), m_renderLayers.end(),
        [&](const std::shared_ptr<SortingLayer>& layer) {
        return layer->getName() == label;
    });

    m_renderLayers.erase(iter);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ScenarioSettings::sortRenderLayers()
{
    std::sort(m_renderLayers.begin(), m_renderLayers.end(), SortRenderLayers);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool ScenarioSettings::SortRenderLayers(const std::shared_ptr<SortingLayer>& l1,
    const std::shared_ptr<SortingLayer>& l2)
{
    return *l1 < *l2;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Scenario::LoadFromFile(const GString& filepath, CoreEngine* core) {
    auto reader = JsonReader(filepath);
    bool exists = reader.fileExists();
    if (exists) {
        QJsonObject json = reader.getContentsAsJsonObject();
        if (!json.isEmpty()) {
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
/////////////////////////////////////////////////////////////////////////////////////////////s
bool ScenarioSettings::removeShaderPreset(const GString & name)
{
    int idx;
    if (hasShaderPreset(name, &idx)) {
        m_shaderPresets.erase(idx + m_shaderPresets.begin());
        return true;
    }

    throw("Error, shader material with the specified name not found");
    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ScenarioSettings::hasShaderPreset(const GString & name, int* iterIndex) const
{
    auto iter = std::find_if(m_shaderPresets.begin(), m_shaderPresets.end(),
        [&](const std::shared_ptr<ShaderPreset>& preset) {
        return preset->getName() == name;
    });

    bool hasPreset = iter != m_shaderPresets.end();
    if (hasPreset) {
        *iterIndex = iter - m_shaderPresets.begin();
    }

    return hasPreset;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ShaderPreset> ScenarioSettings::getShaderPreset(const GString & name, bool & created)
{
    // Check if shader material is in the map 
    int idx;
    if (hasShaderPreset(name, &idx)) {
        created = false;
        return m_shaderPresets[idx];
    }

    // Create new shader preset if not in map
    created = true;
    auto preset = std::make_shared<ShaderPreset>(m_scenario->engine(), name);
    m_shaderPresets.push_back(preset);
    return preset;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//std::shared_ptr<ShaderPreset> ScenarioSettings::getShaderPreset(const Uuid & uuid)
//{
//    return m_shaderPresets[uuid];
//}



///////////////////////////////////////////////////////////////////////////////////////////////////
// Scenario
///////////////////////////////////////////////////////////////////////////////////////////////////
Scenario::Scenario() :
    QObject(nullptr),
    m_engine(nullptr),
    m_settings(this)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
Scenario::Scenario(const Scenario & scenario):
    QObject(nullptr),
    m_settings(scenario.m_settings),
    m_engine(scenario.m_engine)
{
    m_settings.m_scenario = this;
}
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
//    for (const std::shared_ptr<Scene>& scene : m_scenes) {
//        scene->updateVisibleFrustumBounds();
//        const AABBData& sceneViewBounds = scene->getVisibleFrustumBounds().boxData();
//        m_visibleBounds.resize(std::vector<Vector4>{ sceneViewBounds.m_max, sceneViewBounds.m_min });
//    }
//}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Scenario::isVisible(const AABB& boundingBox)
{
    if (m_engine->simulationLoop()->getPlayMode() == SimulationLoop::kDebug) {
        return m_engine->debugManager()->camera()->camera().canSee(boundingBox);
    }
    else {
        for (const std::shared_ptr<Scene>& scene : m_scenes) {
            for (CameraComponent* camera : scene->cameras()) {
                if (camera->camera().canSee(boundingBox)) {
                    return true;
                }
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
    QFile scenarioFile(filepath);
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
std::shared_ptr<Scene> Scenario::addScene()
{
    return Scene::create(this);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Scenario::addScene(const std::shared_ptr<Scene>& scene)
{
    m_scenes.emplace_back(scene);
    emit addedScene(scene);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Scenario::removeScene(const std::shared_ptr<Scene>& scene)
{
    auto sceneIter = std::find_if(m_scenes.begin(), m_scenes.end(), 
        [scene](const std::shared_ptr<Scene>& s) {
        return s->getUuid() == scene->getUuid();
    });

    if (sceneIter == m_scenes.end()) {
        throw("Error, scene not found in scenario");
    }

    scene->clear();
    m_scenes.erase(sceneIter);
    emit removedScene(scene);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
const std::shared_ptr<Scene>& Scenario::getScene(const Uuid & uuid)
{
    auto sceneIter = std::find_if(m_scenes.begin(), m_scenes.end(),
        [uuid](const std::shared_ptr<Scene>& s) {
        return s->getUuid() == uuid;
    });

    if (sceneIter == m_scenes.end()) {
#ifdef DEBUG_MODE
        logWarning("Warning, scene with the UUID " + uuid.asString() + " not found");
#endif
        return nullptr;
    }
    else {
        return *sceneIter;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> Scenario::getSceneByName(const GString & name)
{
    auto sceneIter = std::find_if(m_scenes.begin(),
        m_scenes.end(),
        [name](const std::shared_ptr<Scene>& scene) {
        return scene->getName() == name;
    });
    if (sceneIter == m_scenes.end()) {
#ifdef DEBUG_MODE
        logWarning("Warning, scene with the name " + name + " not found");
#endif
        return nullptr;
    }
    else {
        return *sceneIter;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Scenario::clear()
{
    clearSceneObjects();

    removeScenes();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Scenario::clearSceneObjects()
{
    for (const auto& scene : m_scenes) {
        scene->clear();
    }
    emit clearedSceneObjects();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Scenario::removeScenes()
{
    m_scenes.clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Scenario::asJson() const
{
    // Create JSON object
    //QJsonObject jsonObject = Loadable::asJson().toObject(); // don't need filepath
    QJsonObject jsonObject;
    jsonObject.insert("name", m_name.c_str());

    // Add process sorting layers
    QJsonObject processManager = m_engine->processManager()->asJson().toObject();
    jsonObject.insert("processManager", processManager);

    // Add physics manager and static physics resources
    QJsonValue physicsManager = m_engine->physicsManager()->asJson();
    jsonObject.insert("physicsManager", physicsManager);

    // Add scenes
    QJsonArray scenes;
    for (const auto& scene: m_scenes) {
        QJsonObject sceneJson = scene->asJson().toObject();
        scenes.append(sceneJson);
    }
    jsonObject.insert("scenes", scenes);

    // Add resources from resource cache
    QJsonObject resourceCache = m_engine->resourceCache()->asJson().toObject();
    jsonObject.insert("resourceCache", resourceCache);

    // Settings
    jsonObject.insert("settings", m_settings.asJson());

    // Debug mangaer
    jsonObject.insert("debug", m_engine->debugManager()->asJson());

    // Sound manager
    jsonObject.insert("sound", m_engine->soundManager()->asJson());

    // Animation manager
    jsonObject.insert("animation", m_engine->animationManager()->asJson());

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
    QString jsonStr = JsonReader::ToQString(jsonObject);
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

    // Load scenes
    if (jsonObject.contains("scenes")) {
        // Add scenes to scenario
        for (const auto& sceneJson : jsonObject.value("scenes").toArray()) {
            const std::shared_ptr<Scene>& scene = addScene();
            scene->loadFromJson(sceneJson);
        }        
        progress.setValue(count++);
    }

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
    // TODO: Use vector instead of map
    for (const auto& scene : m_scenes) {
        std::vector<CameraComponent*>& cameras = scene->cameras();
        size_t numCameras = cameras.size();
        for (size_t i = 0; i < numCameras; i++) {
            // Update camera's framebuffer and light cluster grid sizes
            SceneCamera& cam = cameras[i]->camera();
            cam.resizeFrame(width, height);
            cam.lightClusterGrid().onResize();
        }
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

    // Connect to widgets
    connect(m_engine->mainRenderer()->widget(), &View::GLWidget::resized,
        this, &Scenario::resizeCameras);

    connect(m_engine, &CoreEngine::scenarioLoaded,
        this, [this]() {
        View::GLWidget* widget = m_engine->mainRenderer()->widget();
        resizeCameras(widget->width(), widget->height());
    });

    // Increment scenario count
    SCENARIO_COUNT++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int Scenario::SCENARIO_COUNT = 0;


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}