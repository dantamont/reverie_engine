
// Includes

#include "core/scene/GSceneObject.h"

// Standard Includes

// External

// Project
#include "core/scene/GScene.h"
#include "core/scene/GScenario.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"

#include "core/rendering/view/GPointLightCamera.h"

#include "core/components/GTransformComponent.h"
#include "core/components/GComponent.h"
#include "core/components/GScriptComponent.h"
#include "core/components/GShaderComponent.h"
#include "core/components/GCameraComponent.h"
#include "core/components/GLightComponent.h"
#include "core/components/GModelComponent.h"
#include "core/components/GListenerComponent.h"
#include <core/components/GCharControlComponent.h>
#include <core/components/GRigidBodyComponent.h>
#include "core/components/GCanvasComponent.h"
#include "core/components/GAnimationComponent.h"
#include "core/components/GCubeMapComponent.h"
#include "core/components/GAudioSourceComponent.h"
#include "core/processes/GScriptedProcess.h"

#include "core/loop/GSimLoop.h"

#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/lighting/GShadowMap.h"
#include "core/rendering/renderer/GRenderCommand.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/shaders/GUniformContainer.h"

#include "core/events/GEventManager.h"
#include "fortress/json/GJson.h"

#include <core/scene/GBlueprint.h>
#include <core/debugging/GDebugManager.h>

#include "fortress/system/memory/GPointerTypes.h"


// Namespace Definitions

namespace rev {


// Class Implementations

// Scene Object
std::shared_ptr<SceneObject> SceneObject::Create(Scene* scene, SceneBehaviorFlags flags)
{
    // Create scene object
    std::shared_ptr<SceneObject> sceneObjectPtr = SceneObject::CreateDagNode<SceneObject>(scene);

    // Set flags
    sceneObjectPtr->setFlags(flags);

    // Add to scene
    sceneObjectPtr->setScene(scene);
    scene->addObject(sceneObjectPtr);

    Uint32_t index = scene->m_worldMatrices.size();
    scene->m_worldMatrices.emplace_back();

    // Add transform to scene object
    // TODO: Maybe just cache a raw pointer, since component will never outlive the scene object
    sceneObjectPtr->m_transformComponent = TransformComponent(sceneObjectPtr, scene->m_worldMatrices.container(), index);

    // Set unique name
    static std::atomic<uint32_t> count = 0;
    static GString name("SceneObject_");
    sceneObjectPtr->setName(name + GString::FromNumber(count.load()));
    count++;

    return sceneObjectPtr;
}

std::shared_ptr<SceneObject> SceneObject::Create(CoreEngine * core, const nlohmann::json& json)
{
    // Get scene
    Scene* scene = &core->scenario()->scene();

    // Initialize scene object from scene and load from JSON
    std::shared_ptr<SceneObject> sceneObject = SceneObject::Create(scene);
    json.get_to(*sceneObject);
    return sceneObject;
}

std::shared_ptr<SceneObject> SceneObject::Create(const SceneObject & other)
{
    // Duplicate the specified scene object
    json json = other;
    Scene* scene = other.scene();

    // Initialize scene object from scene and load from JSON
    std::shared_ptr<SceneObject> sceneObject = SceneObject::Create(scene);
    if (other.parent()) {
        sceneObject->setParent(other.parent());
    }
    json.get_to(*sceneObject);
    return sceneObject;
}

std::shared_ptr<SceneObject> SceneObject::Get(uint32_t id)
{
    return DagNode::DagNodes()[id];
}

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

SceneObject::~SceneObject()
{
    scene()->m_worldMatrices.invalidate(m_transformComponent.m_matrices.m_worldMatrixContainer.m_index);
    for (auto& component : m_components) {
        if (component) {
            component->preDestruction(m_scene->engine());
            delete component;
        }
    }
}

Blueprint& SceneObject::createBlueprint() const
{
    std::vector<Blueprint>& blueprints = m_scene->scenario()->blueprints();
    blueprints.emplace_back(*this);
    return blueprints.back();
}

bool SceneObject::canDraw(SceneCamera & camera, const SortingLayer & currentLayer, bool overrideLayer,
    std::shared_ptr<const ShaderPreset>& outPreset) const
{
    // Skip if no shader component or does not contain render layer
    ShaderComponent* sc = getComponent<ShaderComponent>(ComponentType::kShader);
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

void SceneObject::retrieveDrawCommands(SceneCamera& camera, OpenGlRenderer& renderer, const SortingLayer& currentLayer, bool overrideLayer)
{
    // Check if this scene object should be drawn
    std::shared_ptr<const ShaderPreset> shaderPreset;
    bool drawThis = canDraw(camera, currentLayer, overrideLayer, shaderPreset);

    // Generate draw commands
    if(drawThis){
        // Create render commands
        ModelComponent* modelComp = getComponent<ModelComponent>(ComponentType::kModel);
        std::vector<std::shared_ptr<DrawCommand>> renderCommands;
        if (modelComp) {
            modelComp->retrieveDrawCommands(renderCommands, &camera, currentLayer);
        
            BoneAnimationComponent* animComp = getComponent<BoneAnimationComponent>(ComponentType::kBoneAnimation);
            for (const std::shared_ptr<DrawCommand>& command : renderCommands) {
                // Set uniforms for scene object
                if (objectMoved()) {
                    applyWorldUniform(*command);
                }

                // If there are animations, update the corresponding command uniforms
                if (animComp) {
                    animComp->animationController().applyUniforms(*command);
                }
            }

            // Commands have been updated, so unset "moved" flag
            /// @note Flag is not unset if shadows are enabled, since shadows need this flag
            bool shadowsEnabled = renderer.lightingFlags().testFlag(OpenGlRenderer::kDynamicShadows);
            if (0 != renderCommands.size() && !shadowsEnabled) {
                setMoved(false);
            }
        }
        else {
            // Get render settings and shader
            ShaderProgram* shaderProgram = shaderPreset->shaderProgram();
            const RenderSettings& renderSettings = shaderPreset->renderSettings();

            CanvasComponent* canvasComp = getComponent<CanvasComponent>(ComponentType::kCanvas);

            if (canvasComp) {
                canvasComp->createDrawCommands(renderCommands,
                    camera,
                    *shaderProgram,
                    shaderPreset->prepassShaderProgram()); /// @todo Is this correct? Why prepass?

                for (const std::shared_ptr<DrawCommand>& command : renderCommands) {
                    // Set uniforms for scene object
                    applyWorldUniform(*command);

                    // Apply the shader preset
                    shaderPreset->applyPreset(*command);

                    // Set render settings
                    // Note, shader preset overrides any sub-settings
                    command->renderSettings().overrideSettings(renderSettings);

                    // Set render layer of command
                    command->setRenderLayer(currentLayer);
                }
            }
        }

        // Add commands to renderer
        renderer.addRenderCommands(renderCommands);
    }

    // Generate commands for children
    for (const std::shared_ptr<SceneObject>& child : children()) {
        child->retrieveDrawCommands(camera, renderer, currentLayer, overrideLayer);
    }
}

void SceneObject::createModelDrawCommands()
{
    ModelComponent* m = getComponent<ModelComponent>(ComponentType::kModel);
    if (!m) {  
        return; 
    }

    ShaderComponent* shaderComp = getComponent<ShaderComponent>(ComponentType::kShader);
    if (!shaderComp) { 
        return;
    }

    Scene& myScene = *scene();
    const ShaderPreset* preset = shaderComp->shaderPreset().get();

    // Create the draw commands for the loaded model, for each camera and scene object layer permutation
    OpenGlRenderer& renderer = *myScene.engine()->openGlRenderer();
    std::unique_lock lock(renderer.drawMutex()); // Need to lock in case GLWidget::paintGL() gets called asynchronously

    // Clear the previous draw commands
    m->clearDrawCommands();

    std::vector<std::shared_ptr<DrawCommand>>& drawCommands = m->drawCommands();
    GSimulationPlayMode playMode = myScene.engine()->simulationLoop()->getPlayMode();
    switch ((ESimulationPlayMode)playMode) {
    case ESimulationPlayMode::eDebug:
        m->createDrawCommands(drawCommands, myScene.engine()->debugManager()->camera()->camera(), preset, myScene.engine()->debugManager()->debugRenderLayer()->id());
        break;
    case ESimulationPlayMode::eStandard:
    {
        for (CameraComponent* comp : myScene.cameras()) {
            std::vector<SortingLayer> layers = comp->getLocalSortingLayers(playMode);
            for (const SortingLayer& layer : layers) {
                if (m->sceneObject()->hasRenderLayer(layer.id())) {
                    m->createDrawCommands(drawCommands, comp->camera(), preset, layer.id());
                }
            }
        }
        break;
    }
    default:
        assert(false && "Invalid");
    }
}

bool SceneObject::hasRenderLayer(uint32_t layerId) const
{
    //auto layers = renderLayers();
    auto iter = std::find_if(m_renderLayers.begin(), m_renderLayers.end(),
        [&](uint32_t lid) {
        return lid == layerId;
    });

    return  iter != m_renderLayers.end();
}

void SceneObject::addRenderLayer(Uint32_t layerId)
{
    m_renderLayers.push_back(layerId);
    onRenderLayerAdded(layerId);
}

void SceneObject::createShadowDrawCommands()
{
    ModelComponent* m = getComponent<ModelComponent>(ComponentType::kModel);
    if (!m) {
        return;
    }

    Scene& myScene = *scene();
    OpenGlRenderer& renderer = *myScene.engine()->openGlRenderer();
    std::unique_lock lock(renderer.drawMutex()); // Need to lock in case GLWidget::paintGL() gets called asynchronously

    // Clear the previous draw commands
    m->clearShadowDrawCommands();

    ShaderComponent* shaderComp = getComponent<ShaderComponent>(ComponentType::kShader);
    if (!shaderComp) {
        return;
    }

    const ShaderPreset* preset = shaderComp->shaderPreset().get();

    // Create the draw commands for the loaded model for each shadow map
    std::vector<ShadowMap*>& shadowMaps = renderer.renderContext().lightingSettings().shadowMaps();

    std::vector<std::shared_ptr<DrawCommand>>& drawCommands = m->shadowDrawCommands();
    for (ShadowMap* sm : shadowMaps) {
        m->createShadowDrawCommands(drawCommands, *sm);
    }
}

void SceneObject::retrieveShadowDrawCommands(OpenGlRenderer & renderer, ShadowMap* shadowMap)
{
    // Skip if no shader component or does not contain render layer
    ShaderComponent* sc = getComponent<ShaderComponent>(ComponentType::kShader);
    if (sc) {
        if (sc->isEnabled()) {
            // If shader component is not enabled, don't need to draw anything!
            std::shared_ptr<const ShaderPreset> shaderPreset = sc->shaderPreset();
            if (shaderPreset) {
                ShaderProgram* shaderProgram = shaderPreset->shaderProgram();
                if (shaderProgram) {
                    ModelComponent* modelComp = getComponent<ModelComponent>(ComponentType::kModel);
                    if (modelComp) {
                        std::vector<std::shared_ptr<DrawCommand>> renderCommands;

                        modelComp->retrieveShadowDrawCommands(renderCommands, shadowMap);

                        BoneAnimationComponent* animComp = getComponent<BoneAnimationComponent>(ComponentType::kBoneAnimation);
                        for (const std::shared_ptr<DrawCommand>& command : renderCommands) {
                            // Set uniforms for scene object
                            if (objectMoved()) {
                                applyWorldUniform(*command);
                            }

                            // If there are animations, update the corresponding command uniforms
                            if (animComp) {
                                animComp->animationController().applyUniforms(*command);
                            }
                        }

                        renderer.addShadowMapCommands(renderCommands);

                        if (0 != renderCommands.size()) {
                            setMoved(false);
                        }
                    }
                }
            }
        }
    }

    // Generate commands for children
    for (const std::shared_ptr<SceneObject>& child : children()) {
        child->retrieveShadowDrawCommands(renderer, shadowMap);
    }
}


std::vector<SortingLayer> SceneObject::renderLayers() const
{
    // Get render layers
    std::vector<SortingLayer> layers;
    const SortingLayers& sceneLayers = m_scene->engine()->scenario()->settings().renderLayers();
    for (uint32_t layerId : m_renderLayers) {
        SortingLayer layer = sceneLayers.getLayerFromId(layerId);
        if (!layer.isValid()) {
            const auto& debugLayer = m_scene->engine()->debugManager()->debugRenderLayer();
            if (layerId == debugLayer->id()) {
                layer = *debugLayer;
            }
            else {
                Logger::Throw("Error, layer not found");
            }
        }
        else{
            layers.push_back(layer);
        }
    }

    return layers;
}

void SceneObject::removeRenderLayer(uint32_t layerId)
{
    auto iter = std::find_if(m_renderLayers.begin(), m_renderLayers.end(),
        [layerId](uint32_t lid) {return lid == layerId; }
    );
    if (iter != m_renderLayers.end()) {
        m_renderLayers.erase(iter);
    }
    onRenderLayerRemoved(layerId);

    for (const auto& child : m_children) {
        child->removeRenderLayer(layerId);
    }
}

void SceneObject::updatePhysics()
{
    RigidBodyComponent* rigidBodyComp = getComponent<RigidBodyComponent>(ComponentType::kRigidBody);
    if (rigidBodyComp) {
        rigidBodyComp->updateTransformFromPhysics();
    }

    for (const std::shared_ptr<SceneObject>& child : m_children) {
        child->updatePhysics();
    }
}

void SceneObject::setComponent(Component* component)
{
    // Delete component of the given type
    ComponentType type = component->getComponentType();
    Component* comp = getComponent(type);
    if (comp) {
        delete comp;
    }
    // Replace component
    m_components[(int)type] =  component ;
}

bool SceneObject::removeComponent(Component * component)
{
    ComponentType compType = component->getComponentType();
    Component* myComp = getComponent(compType);
    if (myComp) {
        delete myComp;
        m_components[int(compType)] = nullptr;
        return true;
    }
    else {
#ifdef DEBUG_MODE
        Logger::Throw("Error, component does not exist for removal from Scene Object");
#endif
        return false;
    }
}

bool SceneObject::removeComponent(const Uuid& componentId)
{
    for (Component* component : m_components) {
        if (component->getUuid() == componentId) {
            return removeComponent(component);
        }
    }

    return false;
}

bool SceneObject::canAdd(Component * component)
{
    if (component->isSceneComponent()) return false;

    ComponentType type = component->getComponentType();

    // Return false if component is a transform type
    if (type == ComponentType::kTransform) {
#ifdef DEBUG_MODE
        Logger::LogWarning("canAdd:: Warning, cannot add additional transforms to a scene object");
#endif
        return false;
    }

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
        Logger::LogWarning("canAdd:: Warning, cannot add additional transforms to a scene object");
#endif
    default:
#ifdef DEBUG_MODE
        Logger::Throw("canAdd:: Warning, failed to add component to scene object");
#endif
        return false;
    }

    bool hasComponentType = getComponent(type) != nullptr;
    return !hasComponentType;
}

std::shared_ptr<SceneObject> SceneObject::getChild(uint32_t childID) const
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

std::shared_ptr<SceneObject> SceneObject::parent() const
{
    if (hasParents()) {
        for (const auto& parentPair : m_parents) {
            if (std::shared_ptr<DagNode> parent = parentPair.second.lock()) {
                return std::static_pointer_cast<SceneObject>(parent);
            }
            else {
                Logger::Throw("Error, parent pointer no longer valid");
            }
        }
        return nullptr;
    }else{
        return nullptr;
    }
}

void SceneObject::setParent(const std::shared_ptr<SceneObject>& newParent)
{
    if (parent()) {
        removeParent(parent()->id());
    }

    if (hasParents()) {
        Logger::Throw("Error, scene object still has parents");
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

Component* SceneObject::getComponent(ComponentType type) const
{
    if (type == ComponentType::kTransform) {
        // Special case for transform
        return const_cast<TransformComponent*>(&m_transformComponent);
    }
    return m_components[(int)type];
}

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

void to_json(json& orJson, const SceneObject& korObject)
{
    json components = json::array();
    for (const auto& component : korObject.m_components) {
        if (!component) { continue; }

        // Skip if component was created via a script
        if (component->isScriptGenerated()) continue;

        // Append to components JSON
        /// \note Components must be explicitly casted to return correct JSON
        components.push_back(component->toJson());
    }
    orJson["id"] = korObject.id(); // Not used when loading from JSON
    orJson["name"] = korObject.m_name.c_str();
    orJson["components"] = components;
    orJson["transform"] = korObject.m_transformComponent;
    if (korObject.m_scene) {
        orJson["scene"] = korObject.scene()->getName().c_str();
    }

    // Scene object flags
    orJson["soFlags"] = (int)korObject.m_soFlags;

    // Render layers
    ScenarioSettings& settings = korObject.scene()->scenario()->settings();
    json layers = json::array();
    std::vector<SortingLayer>& rLayers = korObject.renderLayers();
    for (const auto& layer : rLayers) {
        layers.push_back(settings.renderLayers().getLayerNameFromId(layer.id()).c_str());
    }
    orJson["renderLayers"] = layers;

    // Add child objects to json
    json kids = json::array();
    for (const auto& child : korObject.children()) {
        kids.push_back(*child);
    }
    orJson["children"] = kids;
}

void from_json(const json& korJson, SceneObject& orObject)
{
    // Set scene object name
    if (korJson.contains("name")) {
        orObject.m_name = korJson.at("name").get_ref<const std::string&>().c_str();
    }

    // Set flags
    if (korJson.contains("soFlags")) {
        orObject.m_soFlags = (SceneBehaviorFlags)korJson.at("soFlags").get<Int32_t>();
    }

    // Add scene object components
    if (korJson.contains("transform")) {
        korJson.at("transform").get_to(orObject.m_transformComponent);
    }

    // Add scene object components
    if (korJson.contains("components")) {
        // Get pointer to this scene object 
        std::shared_ptr<SceneObject> thisSceneObject;
        if (orObject.scene()) {
            thisSceneObject = orObject.scene()->getSceneObject(orObject.id());
        }
        else {
            thisSceneObject = std::static_pointer_cast<SceneObject>(DagNode<SceneObject>::DagNodes().at(orObject.id()));
        }

        for (const json& componentJson : korJson.at("components")) {
            Component::create(thisSceneObject, componentJson);
        }
    }

    // Load render layers
    orObject.m_renderLayers.clear();
    if (korJson.contains("renderLayers")) {
        const json& layers = korJson["renderLayers"];
        SortingLayers& sceneLayers = orObject.m_scene->engine()->scenario()->settings().renderLayers();
        for (const json& layerJson : layers) {
            // Find layer in scene
            GString layerName = layerJson.get_ref<const std::string&>().c_str();
            SortingLayer layer = sceneLayers.getLayer(layerName);
            if (!layer.isValid()) {
                // If layer not found, assumed to be debug layer
                const auto& debugLayer = orObject.m_scene->engine()->debugManager()->debugRenderLayer();
                layer = *debugLayer;
            }

            // Set member using layer ID
            orObject.addRenderLayer(layer.id());
        }
    }

    // Load child scene objects
    if (korJson.contains("children")) {
        for (const json& childJson : korJson.at("children")) {
            // Get pointer to this object's scene and use it to instantiate a child object
            std::shared_ptr<SceneObject> child = SceneObject::Create(orObject.m_scene);
            child->setParent(SceneObject::Get(orObject.id()));
            childJson.get_to(*child);
        }
    }
}

void SceneObject::onAddChild(const std::shared_ptr<SceneObject>& child)
{
    // Don't need to do anything with transforms, since onAddParent covers it
}

void SceneObject::onAddParent(const std::shared_ptr<SceneObject>& parent)
{
    // Set transform's parent
    m_transformComponent.setParent(&parent->transform());

    // Remove from top-level item list if present
    auto thisShared = std::static_pointer_cast<SceneObject>(sharedPtr());
    m_scene->demoteObject(thisShared);
}

void SceneObject::onRemoveChild(const std::shared_ptr<SceneObject>& child)
{
    // Don't need to do anything with transforms, since onRemoveParent covers it
}

void SceneObject::onRemoveParent(const std::shared_ptr<SceneObject>& parent)
{
    m_transformComponent.clearParent(true);
}

void SceneObject::updateBounds(const IndexedTransform& transform)
{
    // Update the bounds with the transform of the scene object
    if (getComponent(ComponentType::kModel))
    {
        if (getComponent(ComponentType::kBoneAnimation)) {
            // Use skeleton to set bounding box if there is an animation component
            BoneAnimationComponent* animComp = getComponent<BoneAnimationComponent>(ComponentType::kBoneAnimation);;
            animComp->updateBounds(transform);
        }
        else {
            // Use model chunks to set bounding boxes if there's no animation component
            ModelComponent* modelComp = getComponent<ModelComponent>(ComponentType::kModel);
            if (modelComp) {
                modelComp->updateBounds(transform);
            }
        }
    }
}

void SceneObject::applyWorldUniform(DrawCommand& renderCommand)
{
    // Set world matrix uniform 
    Int32_t uniformId = renderCommand.shaderProgram()->uniformMappings().m_worldMatrix;
    Int32_t prepassUniformId{ -1 };
    if (renderCommand.prepassShaderProgram()) {
        prepassUniformId = renderCommand.prepassShaderProgram()->uniformMappings().m_worldMatrix;
    }
    renderCommand.setUniform(m_transformComponent.worldMatrixUniform(), uniformId, prepassUniformId);
}

void SceneObject::setDefaultRenderLayers()
{
    // TODO: Configure these from a file
    Scenario* scenario = scene()->scenario();
    addRenderLayer(scenario->settings().renderLayers().getLayer("skybox").id());
    addRenderLayer(scenario->settings().renderLayers().getLayer("world").id());
    addRenderLayer(scenario->settings().renderLayers().getLayer("effects").id());
    addRenderLayer(scenario->settings().renderLayers().getLayer("ui").id());
}

void SceneObject::onRenderLayerAdded(Uint32_t layerId)
{
    ModelComponent* modelComp = getComponent<ModelComponent>(ComponentType::kModel);

    if (modelComp) {
        modelComp->onRenderLayerAdded(layerId);
    }
}

void SceneObject::onRenderLayerRemoved(Uint32_t layerId)
{
    ModelComponent* modelComp = getComponent<ModelComponent>(ComponentType::kModel);
    if (modelComp) {
        modelComp->onRenderLayerRemoved(layerId);
    }

}

SceneObject::SceneObject(Scene* scene):
    DagNode(),
    m_soFlags(SceneBehaviorFlag::kEnabled)
{
    m_components.resize((int)ComponentType::MAX_COMPONENT_TYPE_VALUE);

    if (scene) { 
        m_scene = scene; 
        if(m_scene->scenario()){
            // Check since debug scene has no scenario
            setDefaultRenderLayers();
        }
    }
    else {
#ifdef DEBUG_MODE
        Logger::Throw("Error, scene object must be instantiated with a scene");
#endif
    }
}


} // end namespacing
