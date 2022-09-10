#include "core/scene/GScene.h"

#include "core/scene/GSceneObject.h"
#include "core/scene/GScenario.h"

#include "fortress/system/memory/GPointerTypes.h"
#include "core/GCoreEngine.h"
#include "fortress/containers/GColor.h"
#include "core/resource/GResourceCache.h"

#include "core/geometry/GRaycast.h"

#include "core/components/GAnimationComponent.h"
#include "core/components/GCameraComponent.h"
#include "core/components/GCanvasComponent.h"
#include "core/components/GLightComponent.h"
#include "core/components/GModelComponent.h"
#include "core/components/GShaderComponent.h"
#include "core/components/GPhysicsSceneComponent.h"
#include "core/components/GCubeMapComponent.h"

#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/renderer/GRenderCommand.h"

#include "core/rendering/lighting/GShadowMap.h"
#include "core/rendering/view/GRenderProjection.h"
#include "core/rendering/shaders/GShaderProgram.h"

#include "core/physics/GPhysicsManager.h"
#include "core/physics/GPhysicsScene.h"

#include "core/loop/GSimLoop.h"
#include "core/debugging/GDebugManager.h"

namespace rev {

Scene::Scene() :
    m_scenario(nullptr),
    m_engine(nullptr)
{
}

Scene::Scene(CoreEngine * engine) :
    m_scenario(nullptr),
    m_engine(engine)
{
    initialize();
}

Scene::Scene(Scenario* scenario):
    m_scenario(scenario),
    m_engine(scenario->engine())
{
    initialize();
}

Scene::~Scene()
{
    disconnectSignals();
    clear();
}

Component* Scene::getComponent(const Uuid& uuid)
{
    auto iter = std::find_if(m_components.begin(), m_components.end(),
        [&](const auto& component) {
            return component->getUuid() == uuid;
        });

    if (iter != m_components.end()) {
        return *iter;
    }
    else {
        return nullptr;
    }
}

Component* Scene::getComponent(ComponentType type)
{
    return m_components[(int)type];
}

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

void Scene::retrieveDrawCommands(OpenGlRenderer & renderer)
{
    GSimulationPlayMode mode = m_engine->simulationLoop()->getPlayMode();

    if (mode == ESimulationPlayMode::eStandard) {
        // Standard mode, iterate through cameras
        for (CameraComponent*& camera : m_cameras) {
            camera->retrieveDrawCommands(*this, renderer, mode);
        }
    }
    else if (mode == ESimulationPlayMode::eDebug) {
        // Debug mode is enabled, use only debug camera
        m_engine->debugManager()->camera()->retrieveDrawCommands(*this, renderer, mode);
    }

    // Retrieve shadow map draw commands
    for (ShadowMap* shadowMap: renderer.renderContext().lightingSettings().shadowMaps()) {
        for (const auto& sceneObject : m_topLevelSceneObjects) {
            sceneObject->retrieveShadowDrawCommands(renderer, shadowMap);
        }
    }
}

void Scene::addCamera(CameraComponent * camera)
{
    m_cameras.emplace_back(camera);
}

void Scene::removeCamera(CameraComponent * camera)
{
    auto it = std::find_if(m_cameras.begin(), m_cameras.end(),
        [&](CameraComponent* cam) {
        return cam->getUuid() == camera->getUuid();
    });

    if (it == m_cameras.end()) {
        Logger::Throw("Error, camera not found");
    }

    m_cameras.erase(it);
}

void Scene::addCanvas(CanvasComponent * canvas)
{
    Vec::EmplaceBack(m_canvases, canvas);
}

void Scene::removeCanvas(CanvasComponent * canv)
{
    auto it = std::find_if(m_canvases.begin(), m_canvases.end(),
        [&](CanvasComponent* canvas) {
        return canv->getUuid() == canvas->getUuid();
    });

    m_canvases.erase(it);
}

void Scene::addCubeMap(CubeMapComponent * cubemap)
{
    Vec::EmplaceBack(m_cubeMaps, cubemap);
}

void Scene::removeCubeMap(CubeMapComponent * map)
{
    auto it = std::find_if(m_cubeMaps.begin(), m_cubeMaps.end(),
        [&](CubeMapComponent* cubeMap) {
        return cubeMap->getUuid() == map->getUuid();
    });
    if (it == m_cubeMaps.end()) 
        Logger::Throw("Error, cubemap not found");
    m_cubeMaps.erase(it);
}

void Scene::addModel(ModelComponent* model)
{
    Vec::EmplaceBack(m_models, model);
}

void Scene::removeModel(ModelComponent* model)
{
    auto it = std::find_if(m_models.begin(), m_models.end(),
        [&](ModelComponent* m) {
            return m->getUuid() == model->getUuid();
        });

    m_models.erase(it);
}

void Scene::addLight(LightComponent* light)
{
    Vec::EmplaceBack(m_lights, light);
}

void Scene::removeLight(LightComponent* light)
{
    auto it = std::find_if(m_lights.begin(), m_lights.end(),
        [&](LightComponent* l) {
            return l->getUuid() == light->getUuid();
        });

    m_lights.erase(it);
}

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

std::shared_ptr<PhysicsScene> Scene::physics()
{
    if (physicsComponent()) {
        return physicsComponent()->physicsScene();
    }
    return nullptr;
}

PhysicsSceneComponent * Scene::physicsComponent()
{
    PhysicsSceneComponent* physics = static_cast<PhysicsSceneComponent*>(m_components[(int)ComponentType::kPhysicsScene]);
    return physics;
}

Scenario * Scene::scenario() const
{
    return m_scenario;
}

void Scene::setScenario(Scenario* scenario)
{
    m_scenario = scenario;
    m_engine = scenario->m_engine;
}

std::vector<CameraComponent*>& Scene::cameras()
{
    return m_cameras;
}

const std::vector<CameraComponent*>& Scene::cameras() const
{
    return m_cameras;
}

std::vector<CanvasComponent*>& Scene::canvases()
{
    return m_canvases;
}

std::vector<CubeMapComponent*>& Scene::cubeMaps()
{
    return m_cubeMaps;
}

//void Scene::setAmbientLight(const Color & lightColor)
//{
//    m_uniforms["ambientColor"] = Uniform("ambientColor", lightColor.toVector3g());
//}

void Scene::addPhysics()
{
    // Return if there is already physics
    if (physics()) {
#ifdef DEBUG_MODE
        Logger::Throw("Error, scene already has physics");
#endif
        return;
    }

    if (m_scenario) {
        // Create physics scene if this scene is a member of a scenario
        // This is a check for debug scene
        new PhysicsSceneComponent(this);
    }
}

std::shared_ptr<SceneObject> Scene::getSceneObject(uint32_t id) const
{
    std::shared_ptr<SceneObject> match = nullptr;

    match = SceneObject::Get(id);

    if (!match) {
#ifdef DEBUG_MODE
        GString err = "Error, failed to find scene object for given ID in the specified scene";
        Logger::LogError(err);
        Logger::Throw(err);
#endif
        return nullptr;
    }
    else {
        return match;
    }
}

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
//        Logger::LogError(err);
//        Logger::Throw(err);
//#endif
        return nullptr;
    }
    else {
        return match;
    }
}

bool Scene::hasTopLevelObject(const std::shared_ptr<SceneObject>& object)
{
    auto iterator = getIterator(object);

    return iterator != m_topLevelSceneObjects.end();
}

void Scene::addObject(const std::shared_ptr<SceneObject>& object)
{
    if (hasTopLevelObject(object)) {
        Logger::Throw("Error, a top-level scene object with this UUID exists");
    }

    if (!object->hasParents()) {
        m_topLevelSceneObjects.push_back(object);
    }
}

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

    /// @todo Not a great fix, but need this check for debug scene
    if (SceneObject::DagNodes().size()) {
        SceneObject::EraseFromNodeVec(object->id());
    }
}

void Scene::demoteObject(const std::shared_ptr<SceneObject>& object)
{
    if (hasTopLevelObject(object)) {
        // Remove from top-level object set if applicable
        auto iterator = getIterator(object);
        m_topLevelSceneObjects.erase(iterator);
    }
}

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
    m_models.clear();
    m_lights.clear();
    m_defaultCubeMap = nullptr;

    // Clear scene components
    for (const auto& comp : m_components) {
        if (comp) {
            delete comp;
        }
    }
    m_components.clear();

    m_components.resize((int)ComponentType::MAX_SCENE_COMPONENT_TYPE_VALUE);
}

//void Scene::applyUniforms(DrawCommand& command)
//{
//    // Set uniforms (just ambient color for now)
//    for (const Uniform& uniform : m_uniforms) {
//        if (command.shaderProgram()->hasUniform(uniform.getName())) {
//            command.setUniform(uniform);
//        }
//    }
//}

std::vector<std::shared_ptr<SceneObject>>::iterator Scene::getIterator(const std::shared_ptr<SceneObject>& object)
{
    auto iterator = std::find_if(m_topLevelSceneObjects.begin(), m_topLevelSceneObjects.end(),
        [object](const std::shared_ptr<SceneObject>& so) {
        return object->id() == so->id();
    });

    return iterator;
}

void Scene::removeComponent(Component * component)
{
    removeComponent(component->getUuid());
}

void Scene::removeComponent(const Uuid& componentId)
{
    std::vector<Component*>::iterator iC = std::find_if(m_components.begin(), m_components.end(),
        [&](const auto& comp) {return comp->getUuid() == componentId; });
    if (iC != m_components.end()) {
        ComponentType compType = (*iC)->getComponentType();
        delete* iC;
        m_components[int(compType)] = nullptr;
    }
    else {
#ifdef DEBUG_MODE
        Logger::LogError("Error, component does not exist for removal from Scene");
#endif
    }
}

void to_json(json& orJson, const Scene& korObject)
{

    orJson["name"] = korObject.m_name.c_str();
    orJson["uuid"] = korObject.getUuid(); // Not used when loading from JSON, just for widgets

    // Serialize scene objects
    json sceneObjects = json::array();
    for (const auto& object : korObject.m_topLevelSceneObjects) {
        if (object->isScriptGenerated()) continue;
        sceneObjects.push_back(*object);
    }
    orJson["sceneObjects"] = sceneObjects;

    // Serialize uniforms used by the scene
    //json uniforms;
    //for (const Uniform& uniform : korObject.m_uniforms) {
    //    uniforms[uniform.getName().c_str()] = uniform;
    //}
    //orJson["uniforms"] = uniforms;

    // Serialize components
    json components = json::array();
    for (const auto& comp : korObject.m_components) {
        if (!comp) continue;

        // Skip if component was created via a script
        if (comp->isScriptGenerated()) continue;

        // Append to components JSON
        components.push_back(*comp);
    }
    orJson["components"] = components;

    // Serialize default skybox
    if (korObject.m_defaultCubeMap) {
        orJson["defaultSkybox"] = korObject.m_defaultCubeMap->getUuid().asString().c_str();
    }

}

void from_json(const json& korJson, Scene& orObject)
{
    // Make sure signals are initialized 
    orObject.initializeSignals();

    // Clear any existing objects from scene
    orObject.clear();

    // Set scene name
    if (korJson.contains("name")) {
        orObject.m_name = korJson.at("name").get_ref<const std::string&>().c_str();
    }
    if (orObject.m_name.isEmpty()) {
        orObject.m_name = orObject.m_uuid.asString();
    }

    // Load uniforms used by the scene
    //const json& uniforms = korJson["uniforms"];
    //for (const auto& jsonPair : uniforms.items()) {
    //    orObject.m_uniforms.push_back(Uniform(jsonPair.value()));
    //}
    //orObject.setUniformStaleness(true);

    // Add scene components
    if (korJson.contains("components")) {
        for (const json& componentJson : korJson.at("components")) {
            ComponentType componentType = ComponentType(componentJson.at("type").get<Int32_t>());

            // Load component
            switch (componentType) {
            case ComponentType::kPhysicsScene:
            {
                auto pComp = new PhysicsSceneComponent(&orObject, componentJson);
                Q_UNUSED(pComp);
                break;
            }
            default:
#ifdef DEBUG_MODE
                Logger::Throw("loadFromJson:: Error, this type of component is not implemented");
#endif
                break;
            }
        }
    }

    // Load scene objects
    if (korJson.contains("sceneObjects")) {
        // Add scene objects to scene
        for (const auto& objectJson : korJson.at("sceneObjects")) {
            // Get pointer to this scene and use it to instantiate a scene object
            Scene* scene;
            if (orObject.m_scenario) {
                scene = &orObject;
            }
            else {
                // If no scenario found, it is the debug scene
                scene = orObject.m_engine->debugManager()->scene().get();
            }
            std::shared_ptr<SceneObject> sceneObject = SceneObject::Create(scene);
            objectJson.get_to(*sceneObject);
        }
    }

    // Load skybox
    if (korJson.contains("defaultSkybox")) {
        Uuid skyboxUuid = korJson["defaultSkybox"];
        orObject.m_defaultCubeMap = orObject.getCubeMap(skyboxUuid);
    }
}

void Scene::setComponent(Component * component)
{
    // Delete all components of the given type
    ComponentType type = component->getComponentType();
    Component* currentComp = m_components[(int)type];
    if (currentComp) {
        delete currentComp;
    }

    // Replace components
    m_components[(int)type] = { component };
}

CoreEngine * Scene::engine() const
{
    return m_engine;
}

void Scene::initialize()
{
    // Make sure component vector can fit all types
    m_components.resize((int)ComponentType::MAX_SCENE_COMPONENT_TYPE_VALUE);

    // Make sure that list of scene object transforms doesn't need to reallocate too often
    constexpr Uint32_t spaceForMatrices = 1000;
    m_worldMatrices.container().reserve(spaceForMatrices);

    // Set unique name
    m_name = m_uuid.asString();
}

void Scene::initializeSignals()
{
    /// @todo Remove once debug scene removed, but don't connect if debug scene
    constexpr Int32_t s_invalidId = -1;
    if (m_scenario && m_resourceAddedId == s_invalidId && m_playModeChangedConnectionId == s_invalidId) {
        // Add connection to create draw commands on model load
        m_resourceAddedId = ResourceCache::Instance().m_resourceAdded.connect(this, &Scene::onResourceLoaded);

        // Add connection to create draw commands on play mode change
        m_playModeChangedConnectionId = m_engine->simulationLoop()->changedPlayModeSignal().connect(
            this,
            &Scene::recreateAllDrawCommands
        );
    }
}

void Scene::onResourceLoaded(Uuid id)
{
    std::shared_ptr<ResourceHandle> handle = ResourceCache::Instance().getHandle(id);
    switch ((EResourceType)handle->getResourceType()) {
    case EResourceType::eModel:
        for (ModelComponent* m : m_models) {
            if (!m->modelHandle()) { continue; }
            if (id != m->modelHandle()->getUuid()) { continue; }

            m->sceneObject()->updateBounds(m->sceneObject()->transform());

            if (!m->hasDrawCommands()) {
                m->sceneObject()->createModelDrawCommands();
                m->sceneObject()->createShadowDrawCommands();
            }
        }
        break;
    case EResourceType::eAnimation:
        for (ModelComponent* m : m_models) {
            BoneAnimationComponent* animComp = m->sceneObject()->getComponent<BoneAnimationComponent>(ComponentType::kBoneAnimation);
            if (!animComp) { continue; }
            if (!m->modelHandle()) { continue; }
            
            m->sceneObject()->updateBounds(m->sceneObject()->transform());
            m->sceneObject()->createModelDrawCommands(); /// @todo Can probably just set correct uniforms to represent that this is an animation
            m->sceneObject()->createShadowDrawCommands();
        }
        break;
    case EResourceType::eShaderProgram:
        for (ModelComponent* m : m_models) {
            if (!m->modelHandle()) { continue; }

            if (!m->hasDrawCommands()) {
                m->sceneObject()->createModelDrawCommands();
                m->sceneObject()->createShadowDrawCommands();
            }
        }
      
        break;
    }
}

void Scene::disconnectSignals()
{
    ResourceCache::Instance().m_resourceAdded.disconnect(m_resourceAddedId);
    m_engine->simulationLoop()->changedPlayModeSignal().disconnect(m_playModeChangedConnectionId);
}

void Scene::recreateAllDrawCommands(GSimulationPlayMode playMode)
{
    // Recreate draw commands for all cameras and scene objects
    for (ModelComponent* m : m_models) {
        if (!m->modelHandle()) { continue; }

        m->sceneObject()->createModelDrawCommands();
    }
}

void Scene::recreateAllShadowDrawCommands()
{
    // Recreate shadow draw commands for all shadow maps and scene objects
    for (ModelComponent* m : m_models) {
        if (!m->modelHandle()) { continue; }

        m->sceneObject()->createShadowDrawCommands();
    }
}



} // end namespacing
