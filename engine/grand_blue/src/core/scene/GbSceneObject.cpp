///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GbSceneObject.h"

// Standard Includes

// External

// Project
#include "GbScene.h"
#include "GbScenario.h"
#include "../GbCoreEngine.h"
#include "../scripting/GbPyWrappers.h"
#include "../components/GbTransformComponent.h"
#include "../components/GbComponent.h"
#include "../components/GbScriptComponent.h"
#include "../components/GbRendererComponent.h"
#include "../components/GbCamera.h"
#include "../components/GbLight.h"
#include "../components/GbModelComponent.h"
#include "../components/GbListenerComponent.h"
#include "../components/GbPhysicsComponents.h"
#include "../components/GbCanvasComponent.h"
#include "../components/GbAnimationComponent.h"
#include "../processes/GbScriptedProcess.h"
#include "../rendering/renderer/GbRenderers.h"
#include "../rendering/shaders/GbShaders.h"
#include "../events/GbEventManager.h"
#include "../readers/GbJsonReader.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Implementations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Scene Object
std::shared_ptr<SceneObject> SceneObject::create(std::shared_ptr<Scene> scene)
{
    // Create scene object and add to node map
    std::shared_ptr<SceneObject> sceneObjectPtr = std::make_shared<SceneObject>(scene);
    if (sceneObjectPtr->type() == NodeType::kBase) {
        throw("Error, an abstract DAG node cannot be instantiated");
    }
    sceneObjectPtr->addToNodeMap();

    // Add to scene
    sceneObjectPtr->setScene(scene);
    scene->addObject(sceneObjectPtr);

    // Add transform to scene object
    sceneObjectPtr->m_transformComponent = std::make_shared<TransformComponent>(sceneObjectPtr);

    // Set unique name
    sceneObjectPtr->setName(std::move(sceneObjectPtr->getUuid().asString()));

    return sceneObjectPtr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneObject::create(CoreEngine * core, const QJsonValue & json)
{
    // Get scene from name
    const QJsonObject& jsonObject = json.toObject();
    std::shared_ptr<Scene> scene;
    if (jsonObject.contains("scene")) {
        QString sceneName = jsonObject["scene"].toString();
        scene = core->scenario()->getSceneByName(sceneName);
    }
    else if (core->scenario()->getScenes().size() == 1) {
        scene = core->scenario()->getScenes().begin()->second;
    }
    else {
#ifdef DEBUG_MODE
        throw("Error, invalid SceneObject json");
#endif
        qDebug() << ("create::Error, invalid SceneObjectjson");
    }

    // Initialize scene object from scene and load from JSON
    std::shared_ptr<SceneObject> sceneObject = SceneObject::create(scene);
    sceneObject->loadFromJson(json);
    return sceneObject;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneObject::get(const Uuid & uuid)
{
    if (DagNode::getDagNodeMap().find(uuid) != DagNode::getDagNodeMap().end()) {
        return std::static_pointer_cast<SceneObject>(DagNode::getDagNodeMap().at(uuid));
    }

    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneObject::get(const QString & uuidStr)
{
    Uuid uuid = Uuid(uuidStr);
    if (DagNode::getDagNodeMap().find(uuid) != DagNode::getDagNodeMap().end()) {
        return std::static_pointer_cast<SceneObject>(DagNode::getDagNodeMap().at(uuid));
    }

    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneObject::getByName(const QString & name)
{
    static QString soStr = QStringLiteral("SceneObject");

    auto iter = std::find_if(DagNode::getDagNodeMap().begin(), DagNode::getDagNodeMap().end(),
        [&](const std::pair<Uuid, std::shared_ptr<DagNode>>& dagPair) {
        if (dagPair.second->className() == soStr) {
            auto sceneObject = std::static_pointer_cast<SceneObject>(dagPair.second);
            return sceneObject->getName() == name;
        }
        return false;
    });

    if (iter != DagNode::getDagNodeMap().end()) {
        return std::static_pointer_cast<SceneObject>(iter->second);
    }

    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneObject::~SceneObject()
{
    // Delete all components
    for (const std::pair<Component::ComponentType, std::vector<Component*>>& componentTypePair : m_components) {
        for (auto& component : componentTypePair.second) {
            delete component;
        }
    }

    //logInfo("Deleting " + m_name);

    emit m_engine->eventManager()->deletedSceneObject(m_uuid);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scene> SceneObject::scene() const {
    return m_scene;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RendererComponent* SceneObject::rendererComponent()
{
    if (m_components.find(Component::kRenderer) != m_components.end()) {
        if (m_components[Component::kRenderer].size() == 0) return nullptr;
        RendererComponent* renderComp = static_cast<RendererComponent*>(m_components[Component::kRenderer][0]);
        return renderComp;
    }
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::setRenderer(std::shared_ptr<Renderer> r)
{
    if (!r) {
        // Remove renderer if setting to nullptr
        if (m_components.count(Component::kRenderer) != 0) {
            m_components.erase(Component::kRenderer);
        }
    }
    else {
        if (m_components.count(Component::kRenderer) == 0) {
            auto rendererComponent =
                std::make_shared<RendererComponent>(std::dynamic_pointer_cast<SceneObject>(sharedPtr()),
                    r);
        }
        else {
            rendererComponent()->setRenderer(r);
        }
    }
    
    // Remove from scene and add again to fix sorting
    auto thisShared = std::dynamic_pointer_cast<SceneObject>(sharedPtr());
    scene()->removeObject(thisShared);
    scene()->m_topLevelSceneObjects.insert(thisShared);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CameraComponent* SceneObject::camera()
{
    if (m_components.find(Component::kCamera) != m_components.end()) {
        if (m_components[Component::kCamera].size() == 0) return nullptr;
        CameraComponent* camera = static_cast<CameraComponent*>(m_components[Component::kCamera][0]);
        return camera;
    }
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Light* SceneObject::light()
{
    if (m_components.find(Component::kLight) != m_components.end()) {
        if (m_components[Component::kLight].size() == 0) return nullptr;
        Light* light = static_cast<Light*>(m_components[Component::kLight][0]);
        return light;
    }
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::setCamera(CameraComponent* camera)
{
    camera->setSceneObject(std::dynamic_pointer_cast<SceneObject>(sharedPtr()));
    setComponent(camera);

    // Need to recompute view matrix with scene object position
    m_transformComponent->computeWorldMatrix();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CharControlComponent * SceneObject::characterController()
{
    if (m_components.find(Component::kCharacterController) != m_components.end()) {
        if (m_components[Component::kCharacterController].size() == 0) return nullptr;
        CharControlComponent* cc = static_cast<CharControlComponent*>(m_components[Component::kCharacterController][0]);
        return cc;
    }
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModelComponent * SceneObject::modelComponent()
{
    if (m_components.find(Component::kModel) != m_components.end()) {
        if (m_components[Component::kModel].size() == 0) return nullptr;
        ModelComponent* m = static_cast<ModelComponent*>(m_components[Component::kModel][0]);
        return m;
    }
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BoneAnimationComponent * SceneObject::boneAnimation()
{
    if (m_components.find(Component::kBoneAnimation) != m_components.end()) {
        if (m_components[Component::kBoneAnimation].size() == 0) return nullptr;
        BoneAnimationComponent* ba = static_cast<BoneAnimationComponent*>(m_components[Component::kBoneAnimation][0]);
        return ba;
    }
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::clearModels()
{
    components().at(Component::kModel).clear();

    for (std::shared_ptr<SceneObject>& child : children()) {
        child->clearModels();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::updatePhysics()
{
    if (hasComponent(Component::kRigidBody)) {
        for (auto& comp : m_components[Component::kRigidBody]) {
            RigidBodyComponent* rigidBodyComp = static_cast<RigidBodyComponent*>(comp);
            rigidBodyComp->updateTransformFromPhysics();
        }
    }

    for (std::shared_ptr<SceneObject>& child : children()) {
        child->updatePhysics();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::abortScriptedProcesses()
{
    if (m_components.find(Component::kPythonScript) == m_components.end()) {
        return;
    }
    for (Component*& component : m_components.at(Component::kPythonScript)) {
        // Cast script component
        ScriptComponent* scriptComponent = static_cast<ScriptComponent*>(component);

        // Abort the process
        if(!scriptComponent->process()->isAborted())
        scriptComponent->process()->abort();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SceneObject::addComponent(Component*  component)
{
    if (canAdd(component)) {
        if (m_components.find(component->getComponentType()) != m_components.end()) {
            //if (m_components.at(component->getComponentType()).count(component->getUuid().asString())) {
            //    throw("Error, component with this UUID already on scene object");
            //}
            m_components.at(component->getComponentType()).push_back(component);
        }
        else {
            setComponent(component);
        }
        return true;
    }
    else{
#ifdef DEBUG_MODE
        throw("Error, failed to add component");
#endif
        delete component;
        return false;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SceneObject::removeComponent(Component * component)
{
    if (m_components.find(component->getComponentType()) != m_components.end()) {
        std::vector<Component*>& componentVec = m_components.at(component->getComponentType());
        std::vector<Component*>::iterator iC = std::find_if(componentVec.begin(),
            componentVec.end(),
            [&](const auto& comp) {return comp->getUuid() == component->getUuid(); });
        if (iC != componentVec.end()) {
            delete *iC;
            componentVec.erase(iC);
            return true;
        }
        else {
#ifdef DEBUG_MODE
            logError("Error, component does not exist for removal from Scene Object");
#endif
            return false;
        }
        return true;
    }
    else {
#ifdef DEBUG_MODE
        logError("Error, component does not exist for removal from Scene Object");
#endif
        return false;
    }
    emit 
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SceneObject::canAdd(Component * component)
{
    if (component->isSceneComponent()) return false;

    Component::ComponentType type = component->getComponentType();

    // Return false if component is a transform type
    if (type == Component::kTransform) {
#ifdef DEBUG_MODE
        logWarning("canAdd:: Warning, cannot add additional transforms to a scene object");
#endif
        return false;
    }

    // Return false if the scene object doesn't have the requisite components to add this one
    if (!hasRequirements(type)) return false;

    // Check component type
    switch (type) {
    case Component::kCamera:
    case Component::kLight:
    case Component::kPythonScript:
    case Component::kRenderer:
    case Component::kModel:
    case Component::kListener:
    case Component::kRigidBody:
    case Component::kCanvas:
    case Component::kCharacterController:
    case Component::kBoneAnimation:
        break;
    case Component::kTransform:
#ifdef DEBUG_MODE
        logError("canAdd:: Warning, cannot add additional transforms to a scene object");
#endif
    default:
#ifdef DEBUG_MODE
        throw("canAdd:: Warning, failed to add component to scene object");
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
    if((numComponents >= maxAllowed) && (maxAllowed > 0)){
        // Return false if component exceeds max allowed per SceneObject
        return false;
    }
    else {
        return true;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SceneObject::hasRequirements(const Component::ComponentType & type) const
{
    bool satisfies = true;
    Component::Requirements reqs = Component::GetRequirements(type);
    for (const Component::ComponentType& type: reqs.m_requiredTypes) {
        satisfies &= hasComponent(type);
    }
    return satisfies;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::shared_ptr<SceneObject>> SceneObject::children() const
{
    std::vector<std::shared_ptr<SceneObject>> childVec;
    for (const auto& dagNodePair : m_children) {
        Vec::EmplaceBack(childVec, std::static_pointer_cast<SceneObject>(dagNodePair.second));
    }
    return childVec;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneObject::getChild(const Uuid & uuid) const
{
    std::vector<std::shared_ptr<SceneObject>> kids = children();
    std::shared_ptr<SceneObject> chosenChild = nullptr;
    for (const std::shared_ptr<SceneObject>& child : kids) {
        if (child->getUuid() == uuid) {
            chosenChild = child;
            break;
        }
        else {
            chosenChild = child->getChild(uuid);
            if (chosenChild)
                break;
        }
    }

    return chosenChild;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneObject::getChildByName(const QString & name) const
{
    std::vector<std::shared_ptr<SceneObject>> kids = children();
    std::shared_ptr<SceneObject> chosenChild = nullptr;
    for (std::shared_ptr<SceneObject>& child : kids) {
        if (child->getName() == name) {
            chosenChild = child;
            break;
        }
        else {
            chosenChild = child->getChildByName(name);
            if (chosenChild)
                break;
        }
    }

    return chosenChild;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::switchScene(std::shared_ptr<Scene> newScene)
{
    // Remove object from its current scene
    if (scene()) {
        scene()->removeObject(std::static_pointer_cast<SceneObject>(sharedPtr()));
    }

    // Add to new scene
    newScene->addObject(std::static_pointer_cast<SceneObject>(sharedPtr()));
    setScene(newScene);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneObject::parent() const
{
    if (hasParents()) {
        for (const auto& parentPair : m_parents) {
            if (std::shared_ptr<DagNode> parent = parentPair.second.lock()) {
                return std::static_pointer_cast<SceneObject>(parent);
            }
            else {
                throw("Error, parent pointer no longer valid");
            }
        }
        return nullptr;
    }else{
        return nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::setParent(const std::shared_ptr<SceneObject>& newParent)
{
    if (parent()) {
        removeParent(parent()->getUuid());
        m_transformComponent->clearParent();
    }

    if (hasParents()) {
        throw("Error, scene object still has parents");
    }

    if (newParent) {
        addParent(newParent);
        m_transformComponent->setParent(newParent->transform().get());

        // Remove from top-level item list if present
        auto thisShared = std::static_pointer_cast<SceneObject>(sharedPtr());
        if (m_scene->hasTopLevelObject(thisShared)) {
            m_scene->removeObject(thisShared);
        }
    }
    else {
        // Add to top-level item list if not present
        auto thisShared = std::static_pointer_cast<SceneObject>(sharedPtr());
        if (!m_scene->hasTopLevelObject(thisShared)) {
            m_scene->addObject(thisShared);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SceneObject::hasComponent(Component::ComponentType type) const
{
    if (m_components.find(type) == m_components.end()) {
        return false;
    }
    else if (m_components.at(type).size() == 0) {
        return false;
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SceneObject::hasCamera() const
{
    return hasComponent(Component::kCamera);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SceneObject::hasRenderer() const
{
    return hasComponent(Component::kRenderer);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::draw(RenderableType type)
{
    // Return if no renderer component (likely a child object without a renderer)
    RendererComponent* rc = rendererComponent();
    if (rc) {
        if (rc->isEnabled()) {

            const std::shared_ptr<Renderer>& r = rc->renderer();
            const std::shared_ptr<ShaderProgram>& shaderProgram = r->shaderProgram();
            RenderSettings& renderSettings = r->renderSettings();

            if (!shaderProgram) {
                // No shader program assigned to the renderer
                return;
            }

            // Bind shader 
            bool bindShader = renderSettings.hasShaderFlag(RenderSettings::kBind);
            bool releaseShader = renderSettings.hasShaderFlag(RenderSettings::kRelease);
            if (bindShader) {
                shaderProgram->bind();

                // Set uniforms for the scene
                // TODO: Set these at the scene level
                if (!scene()) throw("Error, object must be in scene to be rendered");
                scene()->bindUniforms(shaderProgram);
            }

            // Set uniforms for scene object
            bindUniforms(shaderProgram);

            // Toggle off bind/release shader settings for renderer
            renderSettings.setShaderBind(false);
            renderSettings.setShaderRelease(false);

            // Render
            // Do not bind shader in this step, since current call handles that
            std::vector<Renderable*> renderables;
            switch (type) {
            case kModel:
            {
                ModelComponent* model = modelComponent();
                if (model) {
                    renderables.push_back(model);
                }
                break;
            }
            case kCanvas:
                renderables = getCanvasComponents();
                break;
            default:
                throw("Error, renderable type unrecognized");
            }
            r->draw(renderables);

            // Release shader if mode specifies
            if (renderSettings.hasShaderFlag(RenderSettings::kRelease)) {
                shaderProgram->release();
            }

            // Toggle render settings back
            renderSettings.setShaderBind(bindShader);
            renderSettings.setShaderRelease(releaseShader);
        }
    }

    // Render children
    for (const std::shared_ptr<SceneObject>& child : children()) {
        child->draw(type);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::remove()
{
    // Abort all scripted processes before removal
    abortScriptedProcesses();

    // Remove from static DAG node map
    scene()->removeObject(std::static_pointer_cast<SceneObject>(sharedPtr()), true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue SceneObject::asJson() const
{
    QJsonObject object = QJsonObject();
    QJsonArray components;
    for (const auto& componentVecPair : m_components) {
        for (const auto& component: componentVecPair.second) {
            // Skip if component was created via a script
            if (component->isPythonGenerated()) continue;

            // Append to components JSON
            components.append(component->asJson());
        }
    }
    object.insert("name", m_name);
    object.insert("components", components);
    object.insert("transform", m_transformComponent->asJson());
    if(m_scene)
        object.insert("scene", scene()->getName());

    // Add child objects to json
    QJsonArray kids;
    for (const auto& child : children()) {
        kids.append(child->asJson());
    }
    object.insert("children", kids);

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::loadFromJson(const QJsonValue & json)
{
    // Set scene object name
    const QJsonObject& jsonObject = json.toObject();
    if (jsonObject.contains("name")) {
        m_name = jsonObject.value("name").toString();
    }

    // Add scene object components
    if (jsonObject.contains("transform")) {
        m_transformComponent->loadFromJson(jsonObject.value("transform"));
    }

    // Add scene object components
    if (jsonObject.contains("components")) {
        // Set up component arrays
        QJsonArray remaining = jsonObject.value("components").toArray();
        QJsonArray remainingCache = QJsonArray();

        // Get pointer to this scene object 
        std::shared_ptr<SceneObject> thisSceneObject;
        if (scene()) {
            thisSceneObject = scene()->getSceneObject(getUuid());
        }
        else {
            thisSceneObject = std::static_pointer_cast<SceneObject>(DagNode::getDagNodeMap().at(m_uuid));
        }

        // iterate over components to construct
        while (remaining.size()) {
            for (const auto& componentJson : remaining) {
                // Get component type
                QJsonObject componentJsonObject = componentJson.toObject();
                Component::ComponentType componentType = Component::ComponentType(componentJsonObject.value("type").toInt());

                QString jsonStr = JsonReader::getJsonValueAsQString(componentJsonObject);
                // Check if scene object satisfies type requirements
                if (!hasRequirements(componentType)) {
                    remainingCache.append(componentJsonObject);
                    continue;
                }

                Component::create(thisSceneObject, componentJsonObject);
            } 

            remaining.swap(remainingCache);
            remainingCache = QJsonArray();
        } // End while
    }

    // Load child scene objects
    if (jsonObject.contains("children")) {
        for (const auto& childJson : jsonObject.value("children").toArray()) {
            // Get pointer to this object's scene and use it to instantiate a child object
            std::shared_ptr<SceneObject> child = SceneObject::create(m_scene);
            child->setParent(SceneObject::get(m_uuid));
            child->loadFromJson(childJson.toObject());
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<Renderable*> SceneObject::getCanvasComponents() const
{
    std::vector<Renderable*> canvases;
    if (m_components.count(Component::kCanvas) != 0) {

        for (const auto& canvas : m_components.at(Component::kCanvas)) {
            Vec::EmplaceBack(canvases, static_cast<CanvasComponent*>(canvas));
        }
    }

    return canvases;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::setComponent(Component * component)
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
void SceneObject::bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    // Set world matrix uniform and update uniforms in shader queue
    auto worldMatrix = m_transformComponent->worldMatrix();
    shaderProgram->setUniformValue("worldMatrix", worldMatrix);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneObject::SceneObject():
    m_isEnabled(true)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneObject::SceneObject(std::shared_ptr<Scene> scene):
    DagNode(),
    m_isEnabled(true)
{
    if (scene) { 
        m_scene = scene; 
        m_engine = scene->engine();
    }
    else {
#ifdef DEBUG_MODE
        throw("Error, scene object must be instantiated with a scene");
#endif
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneObject::SceneObject(CoreEngine* engine) :
    DagNode(),
    m_isEnabled(true)
{
    m_scene = nullptr;
    m_engine = engine;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing
