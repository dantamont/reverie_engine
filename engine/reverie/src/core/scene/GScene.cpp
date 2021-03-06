#include "GScene.h"

#include "GSceneObject.h"
#include "GScenario.h"

#include "../utils/GMemoryManager.h"
#include "../GCoreEngine.h"
#include "../containers/GColor.h"
#include "../resource/GResourceCache.h"

#include "../geometry/GRaycast.h"

#include "../components/GCameraComponent.h"
#include "../components/GCanvasComponent.h"
#include "../components/GLightComponent.h"
#include "../components/GShaderComponent.h"
#include "../components/GPhysicsSceneComponent.h"
#include "../components/GCubeMapComponent.h"

#include "../rendering/renderer/GMainRenderer.h"
#include "../rendering/renderer/GRenderCommand.h"

#include "../rendering/lighting/GShadowMap.h"
#include "../rendering/view/GRenderProjection.h"
#include "../rendering/shaders/GShaderProgram.h"

#include "../physics/GPhysicsManager.h"
#include "../physics/GPhysicsScene.h"

#include "../loop/GSimLoop.h"
#include "../debugging/GDebugManager.h"

namespace rev {
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
Scene::~Scene()
{
    clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::raycast(const WorldRay & ray, std::vector<WorldRayHit>& outHits)
{
    // Iterate through all scene objects to cast
    for (const auto& so : m_topLevelSceneObjects) {
        ray.cast(*so, outHits);
    }

    // Sort hits by distance from ray origin (closest first)
    std::sort(std::begin(outHits), std::end(outHits),
        [](const WorldRayHit& h1, const WorldRayHit& h2) {
        return h1.m_distance < h2.m_distance;
    });
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

    if (it == m_cameras.end()) {
        throw("Error, camera not found");
    }

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
    if (m_components[(int)ComponentType::kPhysicsScene].size() == 0) {
        return nullptr;
    }
    else {
        PhysicsSceneComponent* physics = static_cast<PhysicsSceneComponent*>(m_components[(int)ComponentType::kPhysicsScene][0]);
        return physics;
    }
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

    if (m_scenario) {
        // Create physics scene if this scene is a member of a scenario
        // This is a check for debug scene
        new PhysicsSceneComponent(this);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> Scene::getSceneObject(size_t id) const
{
    std::shared_ptr<SceneObject> match = nullptr;

    match = SceneObject::Get(id);

    if (!match) {
#ifdef DEBUG_MODE
        QString err = "Error, failed to find scene object for given ID in the specified scene";
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
//#ifdef DEBUG_MODE
//        QString err = "Error, failed to find scene object for given name";
//        logError(err);
//        throw(err);
//#endif
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
void Scene::removeObject(std::shared_ptr<SceneObject> object, bool eraseFromTopLevel)
{
    // Recursively remove all children before removing this scene object
    // This avoids issues with transforms deleting out of order
    for (const std::shared_ptr<SceneObject>& child : object->children()) {
        removeObject(child, eraseFromTopLevel);
    }

    // Clear child vector
    object->children().clear();

    if (eraseFromTopLevel) {
        if (hasTopLevelObject(object)) {
            // Remove from top-level object set if applicable
            auto iterator = getIterator(object);
            m_topLevelSceneObjects.erase(iterator);
        }
    }

    SceneObject::EraseFromNodeVec(object->id());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::demoteObject(const std::shared_ptr<SceneObject>& object)
{
    if (hasTopLevelObject(object)) {
        // Remove from top-level object set if applicable
        auto iterator = getIterator(object);
        m_topLevelSceneObjects.erase(iterator);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::clear()
{
    // Clear scene objects
    // Can't reference shared pointer, or will delete prematurely
    for (const std::shared_ptr<SceneObject>& object : m_topLevelSceneObjects) {
        //logInfo("removing parent " + object->getName());
        removeObject(object, false);
    }
    m_topLevelSceneObjects.clear();
    m_cameras.clear();
    m_canvases.clear();
    m_cubeMaps.clear();
    m_defaultCubeMap = nullptr;

    // Clear scene components
    for (const std::vector<Component*>& componentVec : m_components) {
        for (const auto& comp : componentVec) {
            delete comp;
        }
    }
    m_components.clear();

    m_components.resize((int)ComponentType::MAX_SCENE_COMPONENT_TYPE_VALUE);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::bindUniforms(DrawCommand& command)
{
    // Set uniforms (just ambient color for now)
    for (const Uniform& uniform : m_uniforms) {
        if (command.shaderProgram()->hasUniform(uniform.getName())) {
            command.addUniform(uniform);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::shared_ptr<SceneObject>>::iterator Scene::getIterator(const std::shared_ptr<SceneObject>& object)
{
    auto iterator = std::find_if(m_topLevelSceneObjects.begin(), m_topLevelSceneObjects.end(),
        [object](const std::shared_ptr<SceneObject>& so) {
        return object->id() == so->id();
    });

    return iterator;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Scene::addComponent(Component * component)
{
    if (canAdd(component)) {
        m_components[(int)component->getComponentType()].push_back(component);
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
    std::vector<Component*>& componentVec = m_components[(int)component->getComponentType()];
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Scene::canAdd(Component * component)
{
    if (!component->isSceneComponent()) return false;

    ComponentType type = component->getComponentType();

    // Check component type
    switch (type) {
    case ComponentType::kPhysicsScene:
        break;
    default:
#ifdef DEBUG_MODE
        throw("canAdd:: Warning, failed to add component to scene, type unrecognized");
#endif
        return false;
    }

    int numComponents = m_components[(int)type].size();
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
QJsonValue Scene::asJson(const SerializationContext& context) const
{
    QJsonObject object = QJsonObject();

    object.insert("name", m_name.c_str());

    // Serialize scene objects
    QJsonArray sceneObjects;
    for (const auto& object : m_topLevelSceneObjects) {
        if (object->isScriptGenerated()) continue;
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
    for (const auto& componentVec : m_components) {
        for (const auto& comp : componentVec) {
            // Skip if component was created via a script
            if (comp->isScriptGenerated()) continue;

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
            QJsonObject componentJsonObject = componentJson.toObject();
            ComponentType componentType = ComponentType(componentJsonObject.value("type").toInt());

            // Load component
            switch (componentType) {
            case ComponentType::kPhysicsScene:
            {
                auto pComp = new PhysicsSceneComponent(this, componentJsonObject);
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
            Scene* scene;
            if (m_scenario) {
                scene = this;
            }
            else {
                // If no scenario found, it is the debug scene
                scene = m_engine->debugManager()->scene().get();
            }
            std::shared_ptr<SceneObject> sceneObject = SceneObject::Create(scene);
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
    ComponentType type = component->getComponentType();
    for (const auto& comp : m_components[(int)type]) {
        delete comp;
    }

    // Replace components
    m_components[(int)type] = { component };
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CoreEngine * Scene::engine() const
{
    return m_engine;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::initialize()
{
    // Make sure component vector can fit all types
    m_components.resize((int)ComponentType::MAX_SCENE_COMPONENT_TYPE_VALUE);

    // Set unique name
    m_name = m_uuid.asString();

    // Removed, not necessary when explicitly binding in GLSL
    // Set shadow map uniform
    //m_uniforms.push_back(Uniform("shadowMap", int(0)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing
