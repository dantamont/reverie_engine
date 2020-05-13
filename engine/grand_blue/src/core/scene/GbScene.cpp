#include "GbScene.h"

#include "GbSceneObject.h"
#include "GbScenario.h"

#include "../utils/GbMemoryManager.h"
#include "../GbCoreEngine.h"
#include "../containers/GbColor.h"
#include "../resource/GbResourceCache.h"
#include "../rendering/materials/GbCubeMap.h"
#include "../components/GbCamera.h"
#include "../components/GbCanvasComponent.h"
#include "../components/GbLight.h"
#include "../components/GbRendererComponent.h"
#include "../components/GbPhysicsSceneComponent.h"
#include "../rendering/renderer/GbRenderers.h"
#include "../rendering/view/GbRenderProjection.h"
#include "../rendering/shaders/GbShaders.h"

#include "../physics/GbPhysicsManager.h"
#include "../physics/GbPhysicsScene.h"

#include "../debugging/GbDebugManager.h"

namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TypeDefs
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef std::multiset<std::shared_ptr<SceneObject>, CompareByRenderLayer> SceneObjectSet;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CompareByRenderLayer::operator()(const std::shared_ptr<SceneObject>& a, const std::shared_ptr<SceneObject>& b) const
{
    if (!a->rendererComponent()) {
        return false;
    }
    if (!b->rendererComponent()) {
        return true;
    }
    return a->rendererComponent()->renderer()->getRenderLayer() < b->rendererComponent()->renderer()->getRenderLayer();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Scene::Scene() :
    m_scenario(nullptr),
    m_skybox(nullptr),
    m_engine(nullptr)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Scene::Scene(CoreEngine * engine) :
    m_scenario(nullptr),
    m_skybox(nullptr),
    m_engine(engine)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Scene::Scene(Scenario* scenario):
    m_scenario(scenario),
    m_skybox(nullptr),
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
    if (m_components.find(Component::kPhysicsScene) != m_components.end()) {
        if (m_components[Component::kPhysicsScene].size() == 0) return nullptr;
        PhysicsSceneComponent* physics = static_cast<PhysicsSceneComponent*>(m_components[Component::kPhysicsScene][0]);
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
std::vector<CanvasComponent*>& Scene::canvases()
{
    return m_canvases;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::vector<std::shared_ptr<Renderer>> Scene::renderers() const
{
    std::vector<std::shared_ptr<Renderer>> renderers;
    for (const auto& object : m_topLevelSceneObjects) {
        if (object->hasRenderer()) {
            RendererComponent* rc = object->rendererComponent();
            renderers.emplace_back(rc->renderer());
        }
    }
    return renderers;
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
        addComponent(new PhysicsSceneComponent(thisScene));
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
std::shared_ptr<SceneObject> Scene::getSceneObjectByName(const QString & name) const
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
        m_topLevelSceneObjects.insert(object);
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
        DagNode::eraseFromNodeMap(object->getUuid());
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::clear()
{
    // Clear scene objects
    for (auto object : m_topLevelSceneObjects) {
        //logInfo("removing parent " + object->getName());

        // Remove all children
        for (auto child : object->children()) {
            //logInfo("removing child " + object->getName());

            child->abortScriptedProcesses();
            DagNode::eraseFromNodeMap(child->getUuid());
        }

        // Remove scene object
        object->abortScriptedProcesses();
        DagNode::eraseFromNodeMap(object->getUuid());
    }
    m_topLevelSceneObjects.clear();

    // Clear scene components
    for (const std::pair<Component::ComponentType, std::vector<Component*>>& componentMapPair : m_components) {
        for (const auto& comp : componentMapPair.second) {
            delete comp;
        }
    }
    m_components.clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    // Set uniforms (just ambient color for now)
    for (const auto& uniformPair : m_uniforms) {
        if (!shaderProgram->hasUniform(uniformPair.first)) continue;
        shaderProgram->setUniformValue(uniformPair.second);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneObjectSet::iterator Scene::getIterator(const std::shared_ptr<SceneObject>& object)
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
    case Component::kPhysicsScene:
        break;
    default:
#ifdef DEBUG_MODE
        logWarning("canAdd:: Warning, failed to add component to scene");
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

    object.insert("name", m_name);

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
    for (const auto& uniformPair : m_uniforms) {
        uniforms.insert(uniformPair.first, uniformPair.second.asJson());
    }
    object.insert("uniforms", uniforms);

    // Serialize skybox
    if (m_skybox) {
        object.insert("skybox", m_skybox->getName());
    }

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

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::loadFromJson(const QJsonValue & json)
{
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
        m_uniforms[key] = Uniform(uniformObject);
    }

    // Add scene components
    if (object.contains("components")) {
        for (const auto& componentJson : object.value("components").toArray()) {
            auto thisScene = m_scenario->getSceneByName(m_name);
            QJsonObject componentJsonObject = componentJson.toObject();
            Component::ComponentType componentType = Component::ComponentType(componentJsonObject.value("type").toInt());

            // Load component
            switch (componentType) {
            case Component::kPhysicsScene:
            {
                auto pComp = new PhysicsSceneComponent(thisScene, componentJsonObject);
                Q_UNUSED(pComp);
                break;
            }
            default:
#ifdef DEBUG_MODE
                throw("loadFromJson:: Error, this type of component is not implemented");
#endif
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
    if (object.contains("skybox")) {
        m_skybox = m_scenario->engine()->resourceCache()->getCubemap(object["skybox"].toString());
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
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Scene::postConstruction()
{
    // Add physics component
    //addPhysics();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing
