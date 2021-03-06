///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GSceneObject.h"

// Standard Includes

// External

// Project
#include "GScene.h"
#include "GScenario.h"
#include "../GCoreEngine.h"
#include "../resource/GResourceCache.h"

#include "../rendering/view/GPointLightCamera.h"

#include "../components/GTransformComponent.h"
#include "../components/GComponent.h"
#include "../components/GScriptComponent.h"
#include "../components/GShaderComponent.h"
#include "../components/GCameraComponent.h"
#include "../components/GLightComponent.h"
#include "../components/GModelComponent.h"
#include "../components/GListenerComponent.h"
#include <core/components/GCharControlComponent.h>
#include <core/components/GRigidBodyComponent.h>
#include "../components/GCanvasComponent.h"
#include "../components/GAnimationComponent.h"
#include "../components/GCubeMapComponent.h"
#include "../components/GAudioSourceComponent.h"
#include "../processes/GScriptedProcess.h"

#include "../rendering/shaders/GShaderProgram.h"
#include "../rendering/lighting/GShadowMap.h"
#include "../rendering/renderer/GRenderCommand.h"
#include "../rendering/renderer/GMainRenderer.h"

#include "../events/GEventManager.h"
#include "../readers/GJsonReader.h"

#include <core/scene/GBlueprint.h>
#include <core/debugging/GDebugManager.h>

#include "../utils/GMemoryManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Implementations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Scene Object
std::shared_ptr<SceneObject> SceneObject::Create(Scene* scene, SceneBehaviorFlags flags)
{
    // Create scene object and add to node map
    std::shared_ptr<SceneObject> sceneObjectPtr = prot_make_shared<SceneObject>(scene);
    sceneObjectPtr->addToNodeMap();

    // Set flags
    sceneObjectPtr->setFlags(flags);

    // Add to scene
    sceneObjectPtr->setScene(scene);
    scene->addObject(sceneObjectPtr);

    // Add transform to scene object
    // TODO: Maybe just cache a raw pointer, since component will never outlive the scene object
    sceneObjectPtr->m_transformComponent = TransformComponent(sceneObjectPtr);

    // Set unique name
    static std::atomic<size_t> count = 0;
    static GString name("SceneObject_");
    sceneObjectPtr->setName(name + GString::FromNumber(count.load()));
    count++;

    return sceneObjectPtr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneObject::Create(CoreEngine * core, const QJsonValue & json)
{
    // Get scene from name
    const QJsonObject& jsonObject = json.toObject();
    Scene* scene = &core->scenario()->scene();

    // Initialize scene object from scene and load from JSON
    std::shared_ptr<SceneObject> sceneObject = SceneObject::Create(scene);
    sceneObject->loadFromJson(json);
    return sceneObject;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneObject::Create(const SceneObject & other)
{
    // Duplicate the specified scene object
    const QJsonObject& json = other.asJson().toObject();
    Scene* scene = other.scene();

    // Initialize scene object from scene and load from JSON
    std::shared_ptr<SceneObject> sceneObject = SceneObject::Create(scene);
    if (other.parent()) {
        sceneObject->setParent(other.parent());
    }
    sceneObject->loadFromJson(json);
    return sceneObject;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneObject::Get(size_t id)
{
    return DagNode::DagNodes()[id];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneObject::getByName(const GString & name)
{
    const std::vector<std::shared_ptr<SceneObject>>& nodes = DagNode::DagNodes();
    auto iter = std::find_if(nodes.begin(), nodes.end(),
        [&](const std::shared_ptr<SceneObject>& so) {
            return so->getName() == name;
    });

    if (iter != DagNode::DagNodes().end()) {
        return *iter;
    }

    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneObject::~SceneObject()
{
    // Set scene to nullptr, so that components know the object is being deleted
    //m_scene = nullptr;

    // Delete all components
    for (const std::vector<Component*>& componentVec : m_components) {
        for (auto& component : componentVec) {
            component->preDestruction(m_scene->engine());
            delete component;
        }
    }

    emit m_scene->engine()->eventManager()->deletedSceneObject(id());
}
/////////////////////////////////////////////////////////////////////////////////////////////
Blueprint& SceneObject::createBlueprint() const
{
    std::vector<Blueprint>& blueprints = m_scene->scenario()->blueprints();
    blueprints.emplace_back(*this);
    return blueprints.back();
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool SceneObject::canDraw(SceneCamera & camera, MainRenderer & renderer, const SortingLayer & currentLayer, bool overrideLayer,
    std::shared_ptr<ShaderPreset>& outPreset) const
{
    // Skip if no shader component or does not contain render layer
    ShaderComponent* sc = hasComponent<ShaderComponent>(ComponentType::kShader);
    bool drawLayer = hasRenderLayer(currentLayer.id()) || overrideLayer;
    if (sc && drawLayer) {
        if (sc->isEnabled()) {
            // If shader component is not enabled, don't need to draw anything!
            outPreset = sc->shaderPreset();
            if (outPreset) {
                // Check if shader program exists, and return true if so
                ShaderProgram* sp = outPreset->shaderProgram();
                if (sp) {
                    return true;
                }
            }
        }
    }

    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::createDrawCommands(SceneCamera& camera, 
    MainRenderer& renderer, 
    const SortingLayer& currentLayer,
    bool overrideLayer)
{
    // Check if this scene object should be drawn
    std::shared_ptr<ShaderPreset> shaderPreset;
    bool drawThis = canDraw(camera, renderer, currentLayer, overrideLayer, shaderPreset);

    // Generate draw commands
    if(drawThis){
        // Get render settings and shader
        ShaderProgram* shaderProgram = shaderPreset->shaderProgram();
        RenderSettings& renderSettings = shaderPreset->renderSettings();

        // Create render commands
        ModelComponent* modelComp = hasComponent<ModelComponent>(ComponentType::kModel);
        CanvasComponent* canvasComp = hasComponent<CanvasComponent>(ComponentType::kCanvas);
        std::vector<std::shared_ptr<DrawCommand>> renderCommands;
        if (modelComp) {
            modelComp->createDrawCommands(renderCommands,
                camera,
                *shaderProgram,
                shaderPreset->prepassShaderProgram());
        }
        if (canvasComp) {
            canvasComp->createDrawCommands(renderCommands,
                camera,
                *shaderProgram,
                shaderPreset->prepassShaderProgram());
        }

        // Set uniforms in render commands
#ifdef DEBUG_MODE
        if (!scene()) {
            throw("Error, object must be in scene to be rendered");
        }
#endif
        for (const std::shared_ptr<DrawCommand>& command : renderCommands) {
            // Set uniforms for the scene
            scene()->bindUniforms(*command);

            // Set uniforms for scene object
            bindUniforms(*command);

            // Set uniforms for shader preset
            command->addUniforms(shaderPreset->uniforms());

            // Set render settings
            // Note, shader preset overrides any sub-settings
            command->renderSettings().overrideSettings(renderSettings);

            // Set render layer of command
            command->setRenderLayer(&const_cast<SortingLayer&>(currentLayer));

            // Add command to renderer
            renderer.addRenderCommand(command);
        }
    }

    // Generate commands for children
    for (const std::shared_ptr<SceneObject>& child : children()) {
        child->createDrawCommands(camera, renderer, currentLayer, overrideLayer);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool SceneObject::hasRenderLayer(size_t layerId) const
{
    //auto layers = renderLayers();
    auto iter = std::find_if(m_renderLayers.begin(), m_renderLayers.end(),
        [&](size_t lid) {
        return lid == layerId;
    });

    return  iter != m_renderLayers.end();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::createDrawCommands(ShadowMap & sm, MainRenderer & renderer)
{
    // TODO: Clean this up with sub-routines
    // Skip if no shader component or does not contain render layer
    ShaderComponent* sc = hasComponent<ShaderComponent>(ComponentType::kShader);
    if (sc) {
        if (sc->isEnabled()) {
            // If shader component is not enabled, don't need to draw anything!
            std::shared_ptr<ShaderPreset> shaderPreset = sc->shaderPreset();
            if (shaderPreset) {
                // Grab some settings from shader preset
                ShaderProgram* shaderProgram = shaderPreset->shaderProgram();
                RenderSettings& renderSettings = shaderPreset->renderSettings();

                if (shaderProgram) {
                    // Create render commands
                    ModelComponent* modelComp = hasComponent<ModelComponent>(ComponentType::kModel);
                    std::vector<std::shared_ptr<DrawCommand>> renderCommands;
                    if (modelComp) {
                        ShaderProgram* prepassShader;
                        if (sm.lightComponent()->getLightType() != Light::kPoint) {
                            // Directional and spot lights are relatively straightforward, just one camera
                            prepassShader = shaderPreset->prepassShaderProgram();

                        }
                        else {
                            // Get cubemap prepass shader for point lights
                            prepassShader = m_scene->engine()->resourceCache()->getHandleWithName("prepass_shadowmap", 
                                ResourceType::kShaderProgram)->resourceAs<ShaderProgram>();
                        }
                        modelComp->createDrawCommands(renderCommands,
                            *sm.camera(),
                            *shaderProgram,
                            prepassShader);

                        // Set uniforms in render commands and add to renderer
                        for (const auto& command : renderCommands) {
                            // Set uniforms for the scene
                            scene()->bindUniforms(*command);

                            // Set uniforms for scene object
                            bindUniforms(*command);

                            // Set uniforms for shader preset
                            command->addUniforms(shaderPreset->uniforms());

                            // Set render settings
                            command->renderSettings().overrideSettings(renderSettings);

                            // Add command to renderer
                            renderer.addShadowMapCommand(command);
                        }
                    }
                }
            }
        }
    }

    // Generate commands for children
    for (const std::shared_ptr<SceneObject>& child : children()) {
        child->createDrawCommands(sm, renderer);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<SortingLayer*> SceneObject::renderLayers() const
{
    // Get render layers
    std::vector<SortingLayer*> layers;
    SortingLayers& sceneLayers = m_scene->engine()->scenario()->settings().renderLayers();
    for (size_t layerId : m_renderLayers) {
        SortingLayer* layer = sceneLayers.getLayerFromId(layerId);
        if (!layer) {
            const auto& debugLayer = m_scene->engine()->debugManager()->debugRenderLayer();
            if (layerId == debugLayer->id()) {
                layer = debugLayer.get();
            }
            else {
                throw("Error, layer not found");
            }
        }
        else{
            layers.push_back(layer);
        }
    }

    return layers;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//bool SceneObject::addRenderLayer(const std::shared_ptr<SortingLayer>& layer)
//{
//    auto iter = std::find_if(m_renderLayers.begin(), m_renderLayers.end(),
//        [&](size_t lid) {
//        return l->id() == layer->id();
//    });
//    if (iter != m_renderLayers.end()) {
//#ifdef DEBUG_MODE
//        throw("Error, layer already found");
//#endif
//        return false;
//    }
//    m_renderLayers.push_back(layer->id());
//    return true;
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::removeRenderLayer(size_t layerId)
{
    auto iter = std::find_if(m_renderLayers.begin(), m_renderLayers.end(),
        [layerId](size_t lid) {return lid == layerId; }
    );
    if (iter != m_renderLayers.end()) {
        m_renderLayers.erase(iter);
    }

    for (const auto& child : m_children) {
        child->removeRenderLayer(layerId);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::clearModels()
{
    components().at((int)ComponentType::kModel).clear();

    for (std::shared_ptr<SceneObject>& child : children()) {
        child->clearModels();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::updatePhysics()
{
    if (hasComponent(ComponentType::kRigidBody)) {
        for (auto& comp : m_components[(int)ComponentType::kRigidBody]) {
            RigidBodyComponent* rigidBodyComp = static_cast<RigidBodyComponent*>(comp);
            rigidBodyComp->updateTransformFromPhysics();
        }
    }

    for (const std::shared_ptr<SceneObject>& child : m_children) {
        child->updatePhysics();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Component * SceneObject::getComponent(const Uuid & uuid, ComponentType type)
{
    std::vector<Component*>& components = m_components[(int)type];
    auto iter = std::find_if(components.begin(), components.end(),
        [&](const auto& component) {
        return component->getUuid() == uuid;
    });

    if (iter != components.end()) {
        return *iter;
    }
    else {
        return nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SceneObject::addComponent(Component*  component)
{
    if (canAdd(component)) {
        if (hasComponent(component->getComponentType())) {
            //if (m_components.at(component->getComponentType()).count(component->getUuid().asString())) {
            //    throw("Error, component with this UUID already on scene object");
            //}
            m_components.at((int)component->getComponentType()).push_back(component);
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
    if (hasComponent(component->getComponentType())) {
        std::vector<Component*>& componentVec = m_components.at((int)component->getComponentType());
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
            throw("Error, component does not exist for removal from Scene Object");
#endif
            return false;
        }
        return true;
    }
    else {
#ifdef DEBUG_MODE
        throw("Error, component does not exist for removal from Scene Object");
#endif
        return false;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SceneObject::canAdd(Component * component)
{
    if (component->isSceneComponent()) return false;

    ComponentType type = component->getComponentType();

    // Return false if component is a transform type
    if (type == ComponentType::kTransform) {
#ifdef DEBUG_MODE
        logWarning("canAdd:: Warning, cannot add additional transforms to a scene object");
#endif
        return false;
    }

    // Return false if the scene object doesn't have the requisite components to add this one
    if (!satisfiesConstraints(type)) return false;

    // Check component type
    switch (type) {
    case ComponentType::kCamera:
    case ComponentType::kLight:
    case ComponentType::kPythonScript:
    case ComponentType::kShader:
    case ComponentType::kModel:
    case ComponentType::kListener:
    case ComponentType::kRigidBody:
    case ComponentType::kCanvas:
    case ComponentType::kCharacterController:
    case ComponentType::kBoneAnimation:
    case ComponentType::kCubeMap:
    case ComponentType::kAudioSource:
    case ComponentType::kAudioListener:
        break;
    case ComponentType::kTransform:
#ifdef DEBUG_MODE
        logError("canAdd:: Warning, cannot add additional transforms to a scene object");
#endif
    default:
#ifdef DEBUG_MODE
        throw("canAdd:: Warning, failed to add component to scene object");
#endif
        return false;
    }

    bool hasComponentType = hasComponent(type);
    if (!hasComponentType) {
        // Return true if no component of the given type was found
        return true;
    }

    int numComponents = m_components[(int)type].size();
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
bool SceneObject::satisfiesConstraints(const ComponentType & type) const
{
    bool satisfies = true;
    Component::Constraints reqs = Component::GetRequirements(type);
    for (const std::pair<ComponentType, bool>& typePair : reqs.m_constraints) {
        if (typePair.second){
            // Constraint is to have a component
            satisfies &= (bool)hasComponent(typePair.first);
        }
        else{
            // Constraint is to not have a component
            satisfies &= !(bool)hasComponent(typePair.first);
        }
    }
    return satisfies;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneObject::getChild(size_t childID) const
{
    std::shared_ptr<SceneObject> chosenChild = nullptr;
    for (const std::shared_ptr<SceneObject>& child : children()) {
        if (child->id() == childID) {
            chosenChild = child;
            break;
        }
        else {
            chosenChild = child->getChild(childID);
            if (chosenChild)
                break;
        }
    }

    return chosenChild;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> SceneObject::getChildByName(const GString & name) const
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
//void SceneObject::switchScene(std::shared_ptr<Scene> newScene)
//{
//    // Remove object from its current scene
//    if (scene()) {
//        scene()->removeObject(std::static_pointer_cast<SceneObject>(sharedPtr()));
//    }
//
//    // Add to new scene
//    newScene->addObject(std::static_pointer_cast<SceneObject>(sharedPtr()));
//    setScene(newScene);
//}
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
        removeParent(parent()->id());
    }

    if (hasParents()) {
        throw("Error, scene object still has parents");
    }

    if (newParent) {
        // The parent is being added
        addParent(newParent);
    }
    else {
        // The parent is being removed
        // Add to top-level item list if not present
        auto thisShared = std::static_pointer_cast<SceneObject>(sharedPtr());
        if (!m_scene->hasTopLevelObject(thisShared)) {
            m_scene->addObject(thisShared);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::removeFromScene()
{
    // Abort all scripted processes before removal
    //abortScriptedProcesses();

    // Remove from parent, if there is one
    if (parent()) {
        parent()->removeChild(this);
    }

    // Remove from static DAG node map
    scene()->removeObject(std::static_pointer_cast<SceneObject>(sharedPtr()));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::onModelLoaded(const std::shared_ptr<ResourceHandle>& modelHandle)
{
    ModelComponent* modelComp = hasComponent<ModelComponent>(ComponentType::kModel);
    if (modelComp) {
        if (modelComp->modelHandle()->getUuid() == modelHandle->getUuid()) {
            // Has this model, so update bounds with newly loaded geometry
            updateBounds(m_transformComponent);
        }
    }

    for (const std::shared_ptr<SceneObject>& child : m_children) {
        child->onModelLoaded(modelHandle);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue SceneObject::asJson(const SerializationContext& context) const
{
    QJsonObject object = QJsonObject();
    QJsonArray components;
    for (const auto& componentVec : m_components) {
        for (const auto& component: componentVec) {
            // Skip if component was created via a script
            if (component->isScriptGenerated()) continue;

            // Append to components JSON
            components.append(component->asJson());
        }
    }
    object.insert("name", m_name.c_str());
    object.insert("components", components);
    object.insert("transform", m_transformComponent.asJson());
    if(m_scene)
        object.insert("scene", scene()->getName().c_str());

    // Scene object flags
    object.insert("soFlags", (int)m_soFlags);

    // Render layers
    QJsonArray layers;
    std::vector<SortingLayer*> rLayers = renderLayers();
    for (const auto& layer : rLayers) {
        layers.append(layer->getName().c_str());
    }
    object.insert("renderLayers", layers);

    // Add child objects to json
    QJsonArray kids;
    for (const auto& child : children()) {
        kids.append(child->asJson());
    }
    object.insert("children", kids);

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context);

    // Set scene object name
    const QJsonObject& jsonObject = json.toObject();
    if (jsonObject.contains("name")) {
        m_name = jsonObject.value("name").toString();
    }

    // Set flags
    if (jsonObject.contains("soFlags")) {
        m_soFlags = (SceneBehaviorFlags)jsonObject["soFlags"].toInt();
    }

    // Add scene object components
    if (jsonObject.contains("transform")) {
        m_transformComponent.loadFromJson(jsonObject.value("transform"));
    }

    // Add scene object components
    if (jsonObject.contains("components")) {
        // Set up component arrays
        QJsonArray remaining = jsonObject.value("components").toArray();
        QJsonArray remainingCache = QJsonArray();

        // Get pointer to this scene object 
        std::shared_ptr<SceneObject> thisSceneObject;
        if (scene()) {
            thisSceneObject = scene()->getSceneObject(id());
        }
        else {
            thisSceneObject = std::static_pointer_cast<SceneObject>(DagNode::DagNodes().at(id()));
        }

        // iterate over components to construct
        while (remaining.size()) {
            for (const auto& componentJson : remaining) {
                // Get component type
                QJsonObject componentJsonObject = componentJson.toObject();
                ComponentType componentType = ComponentType(componentJsonObject.value("type").toInt());

                //QString jsonStr = JsonReader::ToString<QString>(componentJsonObject);
                // Check if scene object satisfies type requirements
                if (!satisfiesConstraints(componentType)) {
                    remainingCache.append(componentJsonObject);
                    continue;
                }

                Component::create(thisSceneObject, componentJsonObject);
            } 

            remaining.swap(remainingCache);
            remainingCache = QJsonArray();
        } // End while
    }

    // Load render layers
    m_renderLayers.clear();
    if (jsonObject.contains("renderLayers")) {
        QJsonArray layers = jsonObject["renderLayers"].toArray();
        SortingLayers& sceneLayers = m_scene->engine()->scenario()->settings().renderLayers();
        for (const auto& layerJson : layers) {
            // Find layer in scene
            QString layerName = layerJson.toString();
            SortingLayer* layer = sceneLayers.getLayer(layerName);
            if (!layer) {
                const auto& debugLayer = m_scene->engine()->debugManager()->debugRenderLayer();
                if (debugLayer->getName() == layerName) {
                    layer = debugLayer.get();
                }
                else {
                    throw("Error, layer not found");
                }
            }

            // Set member using layer ID
            Vec::EmplaceBack(m_renderLayers, layer->id());
        }
    }

    // Load child scene objects
    if (jsonObject.contains("children")) {
        for (const auto& childJson : jsonObject.value("children").toArray()) {
            // Get pointer to this object's scene and use it to instantiate a child object
            std::shared_ptr<SceneObject> child = SceneObject::Create(m_scene);
            child->setParent(SceneObject::Get(id()));
            child->loadFromJson(childJson.toObject());
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::onAddChild(const std::shared_ptr<SceneObject>& child)
{
    // Don't need to do anything with transforms, since onAddParent covers it
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::onAddParent(const std::shared_ptr<SceneObject>& parent)
{
    // Set transform's parent
    m_transformComponent.setParent(&parent->transform());

    // Remove from top-level item list if present
    auto thisShared = std::static_pointer_cast<SceneObject>(sharedPtr());
    m_scene->demoteObject(thisShared);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::onRemoveChild(const std::shared_ptr<SceneObject>& child)
{
    // Don't need to do anything with transforms, since onRemoveParent covers it
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::onRemoveParent(const std::shared_ptr<SceneObject>& parent)
{
    m_transformComponent.clearParent(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::updateBounds(const Transform& transform)
{
    // Update the bounds with the transform of the scene object
    if (hasComponent(ComponentType::kModel))
    {
        if (hasComponent(ComponentType::kBoneAnimation)) {
            // Use skeleton to set bounding box if there is an animation component
            BoneAnimationComponent* animComp = hasComponent<BoneAnimationComponent>(ComponentType::kBoneAnimation);;
            animComp->updateBounds(transform);
        }
        else {
            // Use model chunks to set bounding boxes if there's no animation component
            ModelComponent* modelComp = hasComponent<ModelComponent>(ComponentType::kModel);
            if (modelComp) {
                modelComp->updateBounds(transform);
            }
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::bindUniforms(DrawCommand& rendercommand)
{
    // Set world matrix uniform 
    rendercommand.addUniform(Shader::s_worldMatrixUniformName, m_transformComponent.worldMatrix());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void SceneObject::setDefaultRenderLayers()
{
    // TODO: Configure these from a file
    Scenario* scenario = scene()->scenario();
    m_renderLayers.emplace_back(scenario->settings().renderLayers().getLayer("skybox")->id());
    m_renderLayers.emplace_back(scenario->settings().renderLayers().getLayer("world")->id());
    m_renderLayers.emplace_back(scenario->settings().renderLayers().getLayer("effects")->id());
    m_renderLayers.emplace_back(scenario->settings().renderLayers().getLayer("ui")->id());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SceneObject::SceneObject():
//    m_soFlags(SceneBehaviorFlag::kIsEnabled)
//{
//    m_components.resize((int)ComponentType::MAX_COMPONENT_TYPE_VALUE);
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneObject::SceneObject(Scene*  scene):
    DagNode(),
    m_soFlags(SceneBehaviorFlag::kEnabled)
{
    if (scene) { 
        m_scene = scene; 
        if(m_scene->scenario()){
            // Check since debug scene has no scenario
            setDefaultRenderLayers();
        }
    }
    else {
#ifdef DEBUG_MODE
        throw("Error, scene object must be instantiated with a scene");
#endif
    }
    m_components.resize((int)ComponentType::MAX_COMPONENT_TYPE_VALUE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing
