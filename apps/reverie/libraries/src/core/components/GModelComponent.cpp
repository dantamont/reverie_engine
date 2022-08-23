#include "core/components/GModelComponent.h"

#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"

#include "core/scene/GScene.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"
#include "core/loop/GSimLoop.h"

#include "core/components/GCameraComponent.h"
#include "core/components/GShaderComponent.h"
#include "core/components/GTransformComponent.h"
#include "core/debugging/GDebugManager.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/shaders/GUniform.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/renderer/GRenderCommand.h"
#include "core/rendering/geometry/GSkeleton.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"

#include "core/components/GLightComponent.h"
#include "core/rendering/lighting/GLight.h"
#include "core/rendering/lighting/GShadowMap.h"
#include "core/rendering/shaders/GUniformContainer.h"

#include "core/components/GAnimationComponent.h"

namespace rev {

ModelComponent::ModelComponent() :
    Component(ComponentType::kModel)
{
}

ModelComponent::ModelComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kModel)
{
    setSceneObject(sceneObject());
    sceneObject()->setComponent(this);
    sceneObject()->updateBounds(sceneObject()->transform());
    sceneObject()->scene()->addModel(this);

    // Initialize uniforms
    RenderContext& context = scene()->engine()->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    m_uniforms.m_false.setValue(false, uc);

    /// @todo Enforce this in widgets
    // These wouldn't have compatible shaders, so there's really no need to allow it
    assert(!sceneObject()->getComponent(ComponentType::kCanvas) && "Error, scene object cannot have both a canvas and model component");
}

rev::ModelComponent::~ModelComponent()
{
    scene()->removeModel(this);
}

void ModelComponent::updateBounds(const TransformInterface& transform)
{
    Model* m = model();
    if (!m) { return; }

    SceneObject& object = *sceneObject();
    object.m_worldBounds.geometry().clear();
    for (ModelChunk& chunk : m->chunks()) {
        chunk.addWorldBounds(transform, object.m_worldBounds);
    }
}

bool ModelComponent::hasDrawCommands() const
{
    return 0 != m_meshDrawCommands.size();
}

std::vector<std::shared_ptr<DrawCommand>> ModelComponent::getMeshCommands(AbstractCamera* camera, Uint32_t renderLayerId) const
{
    std::vector<std::shared_ptr<DrawCommand>> meshCommands;
    meshCommands.reserve(m_meshDrawCommands.size());
    for (const auto& command : m_meshDrawCommands) {
        if (command->camera()->getUuid() == camera->getUuid() &&
            renderLayerId == command->renderLayer().id()) {
            meshCommands.push_back(command);
        }
    }

    return meshCommands;
}

void ModelComponent::clearDrawCommands()
{
    m_meshDrawCommands.clear();
}

void ModelComponent::clearShadowDrawCommands()
{
    m_shadowDrawCommands.clear();
}

void ModelComponent::retrieveDrawCommands(std::vector<std::shared_ptr<DrawCommand>>& outCommands, AbstractCamera* camera, const SortingLayer& currentLayer)
{
    SceneObject& so = *sceneObject();
    std::vector<std::shared_ptr<DrawCommand>> meshDrawCommands = getMeshCommands(camera, currentLayer.id());
    Uint32_t numCommands = meshDrawCommands.size();
    outCommands.reserve(numCommands);
    for (Uint32_t i = 0; i < numCommands; i++) {
        // Update world bounds
        const std::shared_ptr<DrawCommand>& command = meshDrawCommands[i];
        command->setRenderableWorldBounds(so.m_worldBounds.geometry()[i]);

        // Only add if visible to camera
        constexpr size_t numBoxesInChunk = 1;
        if (so.m_worldBounds.inShape(camera->frustum(), i/*boxCount*/, numBoxesInChunk)) {
            outCommands.push_back(command);
        }
    }
}

void ModelComponent::retrieveShadowDrawCommands(std::vector<std::shared_ptr<DrawCommand>>& outCommands)
{
    SceneObject& so = *sceneObject();
    Uint32_t numCommands = m_shadowDrawCommands.size();
    outCommands.reserve(numCommands);
    for (Uint32_t i = 0; i < numCommands; i++) {
        // Update world bounds is unnecessary since shadows are "deferred geometry", a.k.a., the bounds aren't known until draw time, or at all
        const std::shared_ptr<DrawCommand>& command = m_shadowDrawCommands[i];
        //command->setRenderableWorldBounds(so.m_worldBounds.geometry()[i]);

        // Add command
        outCommands.push_back(command);
    }
}

void ModelComponent::onRenderLayerAdded(Uint32_t layerId)
{
    SceneObject& so = *sceneObject();
    ShaderComponent* shaderComp = so.getComponent<ShaderComponent>(ComponentType::kShader);
    if (!shaderComp) {
        return;
    }

    // Regenerate the draw commands for this scene object for all cameras
    std::vector<SortingLayer> sortingLayers = so.renderLayers();
    GSimulationPlayMode playMode = scene()->engine()->simulationLoop()->getPlayMode();

    switch ((ESimulationPlayMode)playMode) {
    case ESimulationPlayMode::eDebug:
        break;
    case ESimulationPlayMode::eStandard:
    {
        for (CameraComponent* camComp : m_scene->cameras()) {
            createDrawCommands(m_meshDrawCommands, camComp->camera(), shaderComp->shaderPreset().get(), layerId);
        }
        break;
    }
    default:
        assert(false && "Invalid play mode");
    }
}

void ModelComponent::onRenderLayerRemoved(Uint32_t layerId)
{
    // Remove the draw commands for this scene object layer
    SceneObject& so = *sceneObject();
    std::vector<SortingLayer> sortingLayers = so.renderLayers();
    GSimulationPlayMode playMode = so.scene()->engine()->simulationLoop()->getPlayMode();

    switch ((ESimulationPlayMode)playMode) {
    case ESimulationPlayMode::eDebug:
        break;
    case ESimulationPlayMode::eStandard:
    {
        std::vector<std::shared_ptr<DrawCommand>> newDrawCommands;
        newDrawCommands.reserve(m_meshDrawCommands.size());

        for (const auto& drawCommand : m_meshDrawCommands) {
            if (drawCommand->renderLayer().id() != layerId) {
                newDrawCommands.push_back(drawCommand);
            }
        }
        newDrawCommands.swap(m_meshDrawCommands);
        break;
    }
    default:
        assert(false && "Invalid play mode");
    }
}

void ModelComponent::createDrawCommands(std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands, AbstractCamera& camera, const ShaderPreset* preset, Int32_t layerId)
{
    if (!isEnabled()) {
        return;
    }

    if (!m_modelHandle) {
        return;
    }

    if (m_modelHandle->isLoading()) {
        return;
    }

    Model* m = model();
    if (!m) {
        return;
    }

    SceneObject& so = *sceneObject();
    so.setMoved(true); // Set the scene object as moved so that world uniform is set

    // Iterate through model chunks to generate draw commands
    std::vector<ModelChunk>& chunks = m->chunks();
    size_t numChunks = chunks.size();

    RenderContext& context = scene()->engine()->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    for (size_t i = 0; i < numChunks; i++) {
        ModelChunk& chunk = chunks[i];

#ifdef DEBUG_MODE
        assert(chunk.isLoaded() && "The chunk needs to be loaded you ding-dong");
#endif
        // Create command, overriding render settings with those belonging to the model component
        auto command = std::make_shared<DrawCommand>(chunk, uc, camera, so.id());
        command->renderSettings().overrideSettings(m_renderSettings);

        // If layer ID given, set it
        if (layerId > -1) {
            const SortingLayers& layers = so.scene()->scenario()->settings().renderLayers();
            if (layerId == m_scene->engine()->debugManager()->debugRenderLayer()->id()) {
                command->setRenderLayer(*m_scene->engine()->debugManager()->debugRenderLayer());
            }
            else {
                command->setRenderLayer(layers.getLayerFromId(layerId));
            }
        }

        // If preset given, set uniforms
        if (preset) {
            const RenderSettings& renderSettings = preset->renderSettings();
            ShaderProgram* shaderProgram = preset->shaderProgram();
            ShaderProgram* presetShaderProgram = preset->prepassShaderProgram();
            command->setShaderPrograms(presetShaderProgram, preset->shaderProgram());
            
            // Set world matrix uniform
            so.applyWorldUniform(*command);

            // Set uniforms and render settings from shader preset
            preset->applyPreset(*command);

            // Set animation-related uniforms
            Int32_t isAnimatedUniformId = shaderProgram->uniformMappings().m_isAnimated;
            if (isAnimatedUniformId != -1) {
                if (!so.getComponent<BoneAnimationComponent>(ComponentType::kBoneAnimation)) {
                    Int32_t isAnimatedUniformPrepassId = presetShaderProgram ? presetShaderProgram->uniformMappings().m_isAnimated : -1;
                    command->setUniform(m_uniforms.m_false, isAnimatedUniformId, isAnimatedUniformPrepassId);
                }
            }
        }

        outDrawCommands.push_back(command);
    }
}

void ModelComponent::createShadowDrawCommands(std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands, ShadowMap& sm)
{
    if (!isEnabled()) {
        return;
    }

    if (!m_modelHandle) {
        return;
    }

    if (m_modelHandle->isLoading()) {
        return;
    }

    Model* m = model();
    if (!m) {
        return;
    }

    SceneObject& so = *sceneObject();
    so.setMoved(true); // Set the scene object as moved so that world uniform is set

    ShaderComponent* sc = so.getComponent<ShaderComponent>(ComponentType::kShader);
    std::shared_ptr<const ShaderPreset> preset = sc->shaderPreset();

    // Iterate through model chunks to generate draw commands
    std::vector<ModelChunk>& chunks = m->chunks();
    size_t numChunks = chunks.size();
    RenderContext& context = scene()->engine()->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();

    for (size_t i = 0; i < numChunks; i++) {
        ModelChunk& chunk = chunks[i];

#ifdef DEBUG_MODE
        assert(chunk.isLoaded() && "The chunk needs to be loaded you ding-dong");
#endif
        // Create command, overriding render settings with those belonging to the model component
        auto command = std::make_shared<DrawCommand>(chunk, uc, *sm.camera(), so.id());
        command->setPassFlags(RenderablePassFlag::kDeferredGeometry);
        command->renderSettings().overrideSettings(m_renderSettings);

        // If preset given, set uniforms
        if (preset) {
            bool isPointLight = sm.lightComponent()->getLightType() == Light::kPoint;

            const RenderSettings& renderSettings = preset->renderSettings();
            ShaderProgram* shaderProgram = preset->shaderProgram();
            ShaderProgram* prepassShader;
            if (isPointLight) {
                // Get cubemap prepass shader for point lights
                prepassShader = ResourceCache::Instance().getHandleWithName("prepass_shadowmap",
                    EResourceType::eShaderProgram)->resourceAs<ShaderProgram>();
            }
            else {
                // Directional and spot lights are relatively straightforward, just one camera
                prepassShader = preset->prepassShaderProgram();
            }
            command->setShaderPrograms(prepassShader, preset->shaderProgram());

            // Set world matrix uniform
            so.applyWorldUniform(*command);

            // Set uniforms and render settings from shader preset
            preset->applyPreset(*command);

            // Set animation-related uniforms
            Int32_t isAnimatedUniformId = shaderProgram->uniformMappings().m_isAnimated;
            if (isAnimatedUniformId != -1) {
                if (!so.getComponent<BoneAnimationComponent>(ComponentType::kBoneAnimation)) {
                    Int32_t isAnimatedUniformPrepassId = prepassShader ? prepassShader->uniformMappings().m_isAnimated : -1;
                    command->setUniform(m_uniforms.m_false, isAnimatedUniformId, isAnimatedUniformPrepassId);
                }
            }

            // For point lights, need instanced rendering for each cubemap face
            if (isPointLight) {
                command->renderSettings().setInstanceCount(6);
            }
        }

        outDrawCommands.push_back(command);
    }
}

void ModelComponent::enable()
{
    Component::enable();
}
 
void ModelComponent::disable()
{
    Component::disable();
}

Model* ModelComponent::model() const
{
    if (m_modelHandle) {
        return m_modelHandle->resourceAs<Model>();
    }
    else {
        return nullptr;
    }
}

void ModelComponent::setModelHandle(const std::shared_ptr<ResourceHandle>& handle)
{
    m_modelHandle = handle;
}

void to_json(json& orJson, const ModelComponent& korObject)
{
    ToJson<Component>(orJson, korObject);
    if (korObject.model()) {
        orJson["model"] = korObject.m_modelHandle->getName().c_str();
    }

    /// @todo Investigate if these should be loaded from JSON
    orJson["renderSettings"] = korObject.m_renderSettings; /// For widgets
}

void from_json(const json& korJson, ModelComponent& orObject)
{
    FromJson<Component>(korJson, orObject);

    // Load model
    if (korJson.contains("model")) {
        GString modelName;
        korJson.at("model").get_to(modelName);
        orObject.m_modelHandle = ResourceCache::Instance().getHandleWithName(modelName, EResourceType::eModel);

        size_t count = 0;
        size_t countMax = 1000;
        while (!orObject.m_modelHandle && count < countMax) {
            orObject.m_modelHandle = ResourceCache::Instance().getHandleWithName(modelName, EResourceType::eModel);
            Logger::LogInfo("Waiting for model " + modelName + " to load");
            count++;

            if (count == countMax) {

                // 9/24/20 Try with MDL extension in case JSON is from when I broke things
                // By changing extensions during binary loading
                GString extension = modelName.split(".").back();
                modelName.replace((const char*)("." + extension), ".mdl");
#ifdef DEBUG_MODE
                Logger::Throw("Error, could not find model");
#endif
                Logger::LogError("Model with name " + modelName + " not found");
                break;
            }
        }
    }

}


} // end namespacing