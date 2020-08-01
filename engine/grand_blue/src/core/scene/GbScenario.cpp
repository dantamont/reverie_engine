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
#include "../../GbMainWindow.h"
#include "../scripting/GbPythonScript.h"

#include "../debugging/GbDebugManager.h"

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

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScenarioSettings::loadFromJson(const QJsonValue & json)
{
    m_renderLayers.clear();

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
std::shared_ptr<SortingLayer> ScenarioSettings::addRenderLayer(const QString & name, int order)
{
    auto layer = std::make_shared<SortingLayer>(name, order);
    m_renderLayers.push_back(layer);
    sortRenderLayers();
    return layer;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool ScenarioSettings::removeRenderLayer(const QString & label)
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
void Scenario::loadFromFile(const QString& filepath, CoreEngine* core) {
    auto reader = JsonReader(filepath);
    if (reader.fileExists()) {
        QJsonObject json = reader.getContentsAsJsonObject();
        if (!json.isEmpty()) {
            auto scenario = std::make_shared<Scenario>(core);
            scenario->m_isLoading = true;
            core->setScenario(scenario);
            scenario->loadFromJson(json);
            scenario->setPath(filepath);
            scenario->m_isLoading = false;
        }
        else {
            core->logError("Failed to load scenario from file, invalid JSON");
        }
    }
}

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
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Scenario::save()
{
    return save(m_path);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Scenario::save(const QString & filepath)
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
    m_scenes.emplace(scene->getUuid(), scene);
    emit addedScene(scene);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//std::shared_ptr<Scene> Scenario::addScene(const Uuid & uuid)
//{
//    auto scene = std::make_shared<Scene>(this);
//    scene->m_uuid = uuid;
//    m_scenes.emplace(uuid.asString(), scene);
//    return scene;
//}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Scenario::removeScene(const std::shared_ptr<Scene>& scene)
{
    if (m_scenes.find(scene->getUuid()) == m_scenes.end()) {
        throw("Error, scene not found in scenario");
    }
    else {
        scene->clear();
        m_scenes.erase(scene->getUuid());
    }
    emit removedScene(scene);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> Scenario::getScene(const Uuid & uuid)
{
    if (m_scenes.find(uuid) == m_scenes.end()) {
        return nullptr;
    }
    return m_scenes.at(uuid);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> Scenario::getSceneByName(const QString & name)
{
    auto sceneIter = std::find_if(m_scenes.begin(),
        m_scenes.end(),
        [&](const std::pair<Uuid, std::shared_ptr<Scene>>& scenePair) {
        return scenePair.second->getName() == name;
    });
    if (sceneIter == m_scenes.end()) {
#ifdef DEBUG_MODE
        logWarning("Warning, scene with the name " + name + " not found");
#endif
        return nullptr;
    }
    else {
        return sceneIter->second;
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
    for (const auto& scenePair : m_scenes) {
        scenePair.second->clear();
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
    jsonObject.insert("name", m_name);

    // Add process sorting layers
    QJsonObject processManager = m_engine->processManager()->asJson().toObject();
    jsonObject.insert("processManager", processManager);

    // Add physics manager and static physics resources
    QJsonValue physicsManager = m_engine->physicsManager()->asJson();
    jsonObject.insert("physicsManager", physicsManager);

    // Add scenes
    QJsonArray scenes;
    for (const auto& scenePair : m_scenes) {
        QJsonObject sceneJson = scenePair.second->asJson().toObject();
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

    return jsonObject;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Scenario::loadFromJson(const QJsonValue & json)
{
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

    // Load scenes
    if (jsonObject.contains("scenes")) {
        // Add scenes to scenario
        for (const auto& sceneJson : jsonObject.value("scenes").toArray()) {
            auto scene = addScene();
            scene->loadFromJson(sceneJson);
        }        
        progress.setValue(count++);
    }

    // Load Debug manager
    if (jsonObject.contains("debug")) {
        m_engine->debugManager()->loadFromJson(jsonObject.value("debug"));
    }
    
    // Emit signal that scenario changed
    emit m_engine->scenarioChanged();
	emit m_engine->scenarioLoaded();
    emit m_engine->scenarioNeedsRedraw();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Scenario::initialize() {
    // Initialize path to scenario and name
    m_path = "";
    m_name = QStringLiteral("New Scenario");
    if (SCENARIO_COUNT > 0) {
        m_name += " " + QString::number(SCENARIO_COUNT);
    }

    // Increment scenario count
    SCENARIO_COUNT++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int Scenario::SCENARIO_COUNT = 0;


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}