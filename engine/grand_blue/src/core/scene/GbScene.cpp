#include "GbScene.h"

#include "GbSceneObject.h"
#include "GbScenario.h"

#include "../utils/GbMemoryManager.h"
#include "../GbCoreEngine.h"
#include "../containers/GbColor.h"
#include "../resource/GbResourceCache.h"

#include "../components/GbCameraComponent.h"
#include "../components/GbCanvasComponent.h"
#include "../components/GbLightComponent.h"
#include "../components/GbShaderComponent.h"
#include "../components/GbPhysicsSceneComponent.h"
#include "../components/GbCubeMapComponent.h"

#include "../rendering/renderer/GbMainRenderer.h"
#include "../rendering/renderer/GbRenderCommand.h"

#include "../rendering/lighting/GbShadowMap.h"
#include "../rendering/view/GbRenderProjection.h"
#include "../rendering/shaders/GbShaders.h"

#include "../physics/GbPhysicsManager.h"
#include "../physics/GbPhysicsScene.h"

#include "../loop/GbSimLoop.h"
#include "../debugging/GbDebugManager.h"

namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Scene::Scene() :
    m_scenario(nullptr),
    m_engine(nullptr)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Scene::Scene(CoreEngine * engine) :
    m_scenario(nullptr),
    m_engine(engine)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Scene::Scene(Scenario* scenario):
    m_scenario(scenario),
    m_engine(scenario->engine())
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> Scene::create(Scenario * scenario)
{
    std::shared_ptr<Scene> scene = prot_make_shared<Scene>(scenario);
    scenario->addScene(scene);
    scene->postConstruction();
    return scene;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> Scene::create(CoreEngine * engine)
{
    std::shared_ptr<Scene> scene = prot_make_shared<Scene>(engine);
    scene->postConstruction();
    return scene;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Scene::~Scene()
{
    clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const AABB& Scene::getVisibleFrustumBounds() const
{
    return m_viewBounds;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::updateVisibleFrustumBounds()
{
    AABBData& sceneFrustumBounds = m_viewBounds.boxData();
    for (CameraComponent* camera : m_cameras) {
        // Get camera info
        const SceneCamera& sc = camera->camera();
        const Matrix4x4g& cameraViewMatrix = sc.getViewMatrix();
        const Matrix4x4g& cameraProjectionMatrix = sc.renderProjection().projectionMatrix();

        // Get bounds of camera's view frustum
        AABBData frustumBounds = Frustum::FrustomBoundingBox(cameraViewMatrix, cameraProjectionMatrix);
        //std::vector<Vector3> points;
        //frustumBounds.getPoints(points);
        //sceneFrustumBounds.resize(points);
        sceneFrustumBounds.resize(std::vector<Vector4>{frustumBounds.m_min, frustumBounds.m_max});
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::createDrawCommands(MainRenderer & renderer)
{
    SimulationLoop::PlayMode mode = m_engine->simulationLoop()->getPlayMode();

    if (mode == SimulationLoop::kStandard) {
        // Standard mode, iterate through cameras
        for (CameraComponent*& camera : m_cameras) {
            camera->createDrawCommands(*this, renderer);
        }
    }
    else if (mode == SimulationLoop::kDebug) {
        // Debug mode is enabled, use only debug camera
        //m_engine->debugManager()->camera()->createDrawCommands(*this, renderer);
        m_engine->debugManager()->camera()->createDebugDrawCommands(*this, renderer);
    }

    // Create shadow map draw commands
    std::vector<ShadowMap*>& shadowMaps = renderer.renderContext().lightingSettings().shadowMaps();
    size_t numShadowMaps = renderer.renderContext().lightingSettings().shadowMaps().size();
    for (size_t i = 0; i < numShadowMaps; i++) {
        shadowMaps[i]->createDrawCommands(*this, renderer);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::addCamera(CameraComponent * camera)
{
    m_cameras.emplace_back(camera);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::removeCamera(CameraComponent * camera)
{
    auto it = std::find_if(m_cameras.begin(), m_cameras.end(),
        [&](CameraComponent* cam) {
        return cam->getUuid() == camera->getUuid();
    });

    m_cameras.erase(it);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::addCanvas(CanvasComponent * canvas)
{
    Vec::EmplaceBack(m_canvases, canvas);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::removeCanvas(CanvasComponent * canv)
{
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&](CanvasComponent* canvas) {
        return canv->getUuid() == canvas->getUuid();
    });

    m_canvases.erase(it);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::addCubeMap(CubeMapComponent * cubemap)
{
    Vec::EmplaceBack(m_cubeMaps, cubemap);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::removeCubeMap(CubeMapComponent * map)
{
    auto it = std::find_if(m_cubeMaps.begin(), m_cubeMaps.end(),
        [&](CubeMapComponent* cubeMap) {
        return cubeMap->getUuid() == map->getUuid();
    });
    if (it == m_cubeMaps.end()) 
        throw("Error, cubemap not found");
    m_cubeMaps.erase(it);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CubeMapComponent * Scene::getCubeMap(const Uuid & uuid)
{
    auto it = std::find_if(m_cubeMaps.begin(), m_cubeMaps.end(),
        [&](CubeMapComponent* cubeMap) {
        return cubeMap->getUuid() == uuid;
    });
    if (it == m_cubeMaps.end()) {
        return nullptr;
    }
    else {
        return *it;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PhysicsScene> Scene::physics()
{
    if (physicsComponent()) {
        return physicsComponent()->physicsScene();
    }
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsSceneComponent * Scene::physicsComponent()
{
    if (m_components.find(Component::ComponentType::kPhysicsScene) != m_components.end()) {
        if (m_components[Component::ComponentType::kPhysicsScene].size() == 0) return nullptr;
        PhysicsSceneComponent* physics = static_cast<PhysicsSceneComponent*>(m_components[Component::ComponentType::kPhysicsScene][0]);
        return physics;
    }
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Scenario * Scene::scenario() const
{
    return m_scenario;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<CameraComponent*>& Scene::cameras()
{
    return m_cameras;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::vector<CameraComponent*>& Scene::cameras() const
{
    return m_cameras;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<CanvasComponent*>& Scene::canvases()
{
    return m_canvases;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<CubeMapComponent*>& Scene::cubeMaps()
{
    return m_cubeMaps;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void Scene::setAmbientLight(const Color & lightColor)
//{
//    m_uniforms["ambientColor"] = Uniform("ambientColor", lightColor.toVector3g());
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::addPhysics()
{
    // Return if there is already physics
    if (physics()) {
#ifdef DEBUG_MODE
        throw("Error, scene already has physics");
#endif
        return;
    }

    // Create physics scene if this scene is a member of a scenario
    if (m_scenario) {
        auto thisScene = m_scenario->getSceneByName(m_name);
        if (!thisScene) {
            throw("Error, scene not found in scenario. addPhysics likely called before or during construction of scene");
        }
        // Is added automatically to scene
        new PhysicsSceneComponent(thisScene);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> Scene::getSceneObject(const Uuid & uuid) const
{
    std::shared_ptr<SceneObject> match = nullptr;

    match = SceneObject::get(uuid);

    //// Try all top-level objects first
    //for (const std::shared_ptr<SceneObject>& so : m_topLevelSceneObjects) {
    //    if (uuid == so->getUuid()) {
    //        match = so;
    //    }
    //}

    //// Then do depth-wise search
    //if (!match) {
    //    for (const std::shared_ptr<SceneObject>& so : m_topLevelSceneObjects) {
    //        match = so->getChild(uuid);
    //    }
    //}

    //auto iterator = std::find_if(m_topLevelSceneObjects.begin(), m_topLevelSceneObjects.end(),
    //    [uuid](const std::shared_ptr<SceneObject>& so) {
    //    if (uuid == so->getUuid()) {
    //        return true;
    //    }
    //    else {
    //        return (so->getChild(uuid) != nullptr);
    //    }
    //});

    if (!match) {
#ifdef DEBUG_MODE
        QString err = "Error, failed to find scene object for given UUID in the specified scene";
        logError(err);
        throw(err);
#endif
        return nullptr;
    }
    else {
        return match;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> Scene::getSceneObjectByName(const GString & name) const
{
    // TODO: Maybe change this to use SceneObject::get static method instead

    //auto iterator = std::find_if(m_topLevelSceneObjects.begin(), m_topLevelSceneObjects.end(),
    //    [name](const std::shared_ptr<SceneObject>& so) {
    //    return name == so->getName();
    //});

    std::shared_ptr<SceneObject> match = nullptr;

    // Try all top-level objects first
    for (const std::shared_ptr<SceneObject>& so : m_topLevelSceneObjects) {
        if (name == so->getName()) {
            match = so;
        }
    }

    // Then do depth-wise search if not found in top-level objects
    if (!match) {
        for (const std::shared_ptr<SceneObject>& so : m_topLevelSceneObjects) {
            match = so->getChildByName(name);
        }
    }

    if (!match) {
#ifdef DEBUG_MODE
        QString err = "Error, failed to find scene object for given name";
        logError(err);
        throw(err);
#endif
        return nullptr;
    }
    else {
        return match;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Scene::hasTopLevelObject(const std::shared_ptr<SceneObject>& object)
{
    auto iterator = getIterator(object);

    return iterator != m_topLevelSceneObjects.end();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::addObject(const std::shared_ptr<SceneObject>& object)
{
    if (hasTopLevelObject(object)) {
        throw("Error, a top-level scene object with this UUID exists");
    }

    if (!object->hasParents()) {
        m_topLevelSceneObjects.push_back(object);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::removeObject(const std::shared_ptr<SceneObject>& object, bool eraseFromNodeMap)
{
    if (hasTopLevelObject(object)) {
        // Remove from top-level object set if applicable
        auto iterator = getIterator(object);
        m_topLevelSceneObjects.erase(iterator);
    }

    if (eraseFromNodeMap) {
        SceneObject::EraseFromNodeMap(object->getUuid());
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::clear()
{
    // Clear scene objects
    for (const std::shared_ptr<SceneObject>& object : m_topLevelSceneObjects) {
        //logInfo("removing parent " + object->getName());

        // Remove all children
        for (const std::shared_ptr<SceneObject>& child : object->children()) {
            //logInfo("removing child " + object->getName());

            child->abortScriptedProcesses();
            SceneObject::EraseFromNodeMap(child->getUuid());
        }

        // Remove scene object
        object->abortScriptedProcesses();
        SceneObject::EraseFromNodeMap(object->getUuid());
    }
    m_topLevelSceneObjects.clear();
    m_cameras.clear();
    m_canvases.clear();
    m_cubeMaps.clear();
    m_defaultCubeMap = nullptr;

    // Clear scene components
    for (const std::pair<Component::ComponentType, std::vector<Component*>>& componentMapPair : m_components) {
        for (const auto& comp : componentMapPair.second) {
            delete comp;
        }
    }
    m_components.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::bindUniforms(DrawCommand& command)
{
    // Set uniforms (just ambient color for now)
    for (const Uniform& uniform : m_uniforms) {
        if (command.shaderProgram()->hasUniform(uniform.getName())) {
            command.setUniform(uniform);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::shared_ptr<SceneObject>>::iterator Scene::getIterator(const std::shared_ptr<SceneObject>& object)
{
    auto iterator = std::find_if(m_topLevelSceneObjects.begin(), m_topLevelSceneObjects.end(),
        [object](const std::shared_ptr<SceneObject>& so) {
        return object->getUuid() == so->getUuid();
    });

    return iterator;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Scene::addComponent(Component * component)
{
    if (canAdd(component)) {
        if (m_components.find(component->getComponentType()) != m_components.end()) {
            m_components.at(component->getComponentType()).push_back(component);
        }
        else {
            setComponent(component);
        }
        return true;
    }
    else {
#ifdef DEBUG_MODE
        throw("Error, failed to add component to scene");
#endif
        return false;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Scene::removeComponent(Component * component, bool deletePointer)
{
    if (m_components.find(component->getComponentType()) != m_components.end()) {
        std::vector<Component*>& componentVec = m_components.at(component->getComponentType());
        std::vector<Component*>::iterator iC = std::find_if(componentVec.begin(),
            componentVec.end(),
            [&](const auto& comp) {return comp->getUuid() == component->getUuid(); });
        if (iC != componentVec.end()) {
            if (deletePointer) {
                delete *iC;
            }
            componentVec.erase(iC);
            return true;
        }
        else {
#ifdef DEBUG_MODE
            logError("Error, component does not exist for removal from Scene");
#endif
            return false;
        }
        return true;
    }
    else {
#ifdef DEBUG_MODE
        logError("Error, component does not exist for removal from Scene");
#endif
        return false;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Scene::canAdd(Component * component)
{
    if (!component->isSceneComponent()) return false;

    Component::ComponentType type = component->getComponentType();

    // Check component type
    switch (type) {
    case Component::ComponentType::kPhysicsScene:
        break;
    default:
#ifdef DEBUG_MODE
        throw("canAdd:: Warning, failed to add component to scene, type unrecognized");
#endif
        return false;
    }

    bool hasComponentType = m_components.find(type) != m_components.end();
    if (!hasComponentType) {
        // Return true if no component of the given type was found
        return true;
    }

    int numComponents = m_components.find(type)->second.size();
    int maxAllowed = component->maxAllowed();
    if ((numComponents >= maxAllowed) && (maxAllowed > 0)) {
        // Return false if component exceeds max allowed per scene
        return false;
    }
    else {
        return true;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Scene::asJson() const
{
    QJsonObject object = QJsonObject();

    object.insert("name", m_name.c_str());

    // Serialize scene objects
    QJsonArray sceneObjects;
    for (const auto& object : m_topLevelSceneObjects) {
        if (object->isPythonGenerated()) continue;
        QJsonObject sceneObjectJson = object->asJson().toObject();
        sceneObjects.append(sceneObjectJson);
    }
    object.insert("sceneObjects", sceneObjects);

    // Serialize uniforms used by the scene
    QJsonObject uniforms;
    for (const Uniform& uniform : m_uniforms) {
        uniforms.insert(uniform.getName(), uniform.asJson());
    }
    object.insert("uniforms", uniforms);

    // Serialize components
    QJsonArray components;
    for (const auto& componentMapPair : m_components) {
        for (const auto& comp : componentMapPair.second) {
            // Skip if component was created via a script
            if (comp->isPythonGenerated()) continue;

            // Append to components JSON
            components.append(comp->asJson());
        }
    }
    object.insert("components", components);

    // Serialize default skybox
    if(m_defaultCubeMap)
        object.insert("defaultSkybox", m_defaultCubeMap->getUuid().asQString());

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    // Clear any existing objects from scene
    clear();

    // Get Json as object
    const QJsonObject& object = json.toObject();

    // Set scene name
    if (object.contains("name")) {
        m_name = object.value("name").toString();
    }
    if (m_name.isEmpty()) {
        m_name = m_uuid.asString();
    }

    // Load uniforms used by the scene
    const QJsonObject& uniforms = object["uniforms"].toObject();
    for (const auto& key : uniforms.keys()) {
        QJsonObject uniformObject = uniforms.value(key).toObject();
        m_uniforms.push_back(Uniform(uniformObject));
    }

    // Add scene components
    if (object.contains("components")) {
        for (const auto& componentJson : object.value("components").toArray()) {
            auto thisScene = m_scenario->getSceneByName(m_name);
            QJsonObject componentJsonObject = componentJson.toObject();
            Component::ComponentType componentType = Component::ComponentType(componentJsonObject.value("type").toInt());

            // Load component
            switch (componentType) {
            case Component::ComponentType::kPhysicsScene:
            {
                auto pComp = new PhysicsSceneComponent(thisScene, componentJsonObject);
                Q_UNUSED(pComp);
                break;
            }
            default:
#ifdef DEBUG_MODE
                throw("loadFromJson:: Error, this type of component is not implemented");
#endif
                break;
            }
        }
    }

    // Load scene objects
    if (object.contains("sceneObjects")) {
        // Add scene objects to scene
        for (const auto& objectJson : object.value("sceneObjects").toArray()) {
            // Get pointer to this scene and use it to instantiate a scene object
            std::shared_ptr<Scene> scene;
            if (m_scenario) {
                scene = m_scenario->getScene(m_uuid);
            }
            else {
                // If no scene found, it is the debug scene
                scene = m_engine->debugManager()->scene();
            }
            std::shared_ptr<SceneObject> sceneObject = SceneObject::create(scene);
            sceneObject->loadFromJson(objectJson.toObject());
        }
    }

    // Load skybox
    if (object.contains("defaultSkybox")) {
        Uuid skyboxUuid = Uuid(object["defaultSkybox"].toString());
        m_defaultCubeMap = getCubeMap(skyboxUuid);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::setComponent(Component * component)
{
    // Delete all components of the given type
    Component::ComponentType type = component->getComponentType();
    if (m_components.find(type) != m_components.end()) {
        for (const auto& comp : m_components.at(type)) {
            delete comp;
        }
    }
    // Replace components
    m_components[type] = { component };
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CoreEngine * Scene::engine() const
{
    return m_engine;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::initialize()
{
    // Set unique name
    m_name = m_uuid.asString();

    // Removed, not necessary when explicitly binding in GLSL
    // Set shadow map uniform
    //m_uniforms.push_back(Uniform("shadowMap", int(0)));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::postConstruction()
{
    // Add physics component
    //addPhysics();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing
