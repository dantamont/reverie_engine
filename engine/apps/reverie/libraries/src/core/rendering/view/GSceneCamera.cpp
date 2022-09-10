#include "core/rendering/view/GSceneCamera.h"

#include "core/GCoreEngine.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/components/GCameraComponent.h"
#include "core/components/GTransformComponent.h"
#include "core/components/GShaderComponent.h"
#include "core/components/GCanvasComponent.h"
#include "core/components/GCubeMapComponent.h"
#include "core/components/GModelComponent.h"

#include "core/resource/GResourceCache.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/buffers/GUniformBufferObject.h"
#include "core/rendering/materials/GMaterial.h"
#include "core/rendering/postprocessing/GPostProcessingChain.h"
#include "core/rendering/postprocessing/GPostProcessingEffect.h"

#include "core/loop/GSimLoop.h"
#include "core/debugging/GDebugManager.h"

#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/renderer/GRenderCommand.h"
#include "core/rendering/shaders/GUniformContainer.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "fortress/layer/framework/GFlags.h"

namespace rev {



// Scene Camera

SceneCamera::SceneCamera(CameraComponent* component):
    Camera(&component->sceneObject()->transform(),
        &component->sceneObject()->scene()->engine()->openGlRenderer()->renderContext(),
        FrameBufferInfo{ 
            AliasingType::kMSAA, /// @todo MSAA is a huge slowdown. Better to roll my own SMAA!
            FrameBuffer::BufferAttachmentType::kRbo, // Color attachment type
            FrameBuffer::BufferAttachmentType::kRbo, // Depth attachment type
            16, // 16 samples for MSAA
            3 } // Three color buffers. First for actual color, second for normals (for SSAO) during prepass, 
        ), 
        // third for draw command IDs
    m_component(component),
    m_cameraOptions(3),
    m_lightClusterGrid(this), 
    m_ssaoFrameBuffer(component->sceneObject()->scene()->engine()->getGLWidgetContext(),
        AliasingType::kDefault,
        FrameBuffer::BufferAttachmentType::kTexture,
        FrameBuffer::BufferAttachmentType::kTexture,
        FBO_SSAO_TEX_FORMAT,
        //TextureFormat::kR16,
        //FBO_DEFAULT_TEX_FORMAT,
        4, // Not using MSAA, so doesn't matter
        1, // One color attachments needed
        false), // No depth
    m_ssaoBlurFrameBuffer(
        component->sceneObject()->scene()->engine()->getGLWidgetContext(),
        AliasingType::kDefault,
        FrameBuffer::BufferAttachmentType::kTexture,
        FrameBuffer::BufferAttachmentType::kTexture,
        FBO_SSAO_TEX_FORMAT,
        //TextureFormat::kR16,
        //FBO_DEFAULT_TEX_FORMAT,
        4, // Not using MSAA, so doesn't matter
        1, // One color attachments needed
        false) // No depth
{
    // Add render projection to GL widget
    CoreEngine* core = component->sceneObject()->scene()->engine();
    GLWidget* mainGLWidget = static_cast<GLWidget*>(core->widgetManager()->mainGLWidget());
    mainGLWidget->addRenderProjection(&m_renderProjection);

    // Initialize shader storage buffers
    m_lightClusterGrid.initializeBuffers(mainGLWidget->renderer()->renderContext());
    m_renderProjection.updateProjection(); // Update again, this time with light cluster grid

    // Add default render layers
    setDefaultRenderLayers();

    // Add default post-processing effects
    m_postProcessingChain = std::make_shared<PostProcessingChain>(this, &mainGLWidget->renderer()->renderContext());

    // TODO: Initialize post-processing shaders in resource cache, since they are core resources
    // They are core resources because while I may add a control to set custom post-processing shaders,
    // These are going to be configured purely through presets (which may be unique to each effect stage instance)
    
    // TODO: Build a post-processing effect widget
    // HDR and contrast
    auto effect = std::make_shared<PostProcessingEffect>(
        QStringLiteral("test_effect")
        );

    auto hdrStage = std::make_shared<PostProcessingEffectStage>((uint32_t)PostProcessingEffectStage::EffectType::kReinhardToneMapping);
    hdrStage->loadFragmentShader(QStringLiteral(":shaders/hdr.frag"));
    effect->addStage(hdrStage);

    auto contrastStage = std::make_shared<PostProcessingEffectStage>((uint32_t)PostProcessingEffectStage::EffectType::kContrast);
    contrastStage->loadFragmentShader(QStringLiteral(":shaders/contrast.frag"));
    effect->addStage(contrastStage);

    effect->reinitialize(core);
    effect->setCheckPoint(true);
    m_postProcessingChain->addEffect(effect);

    // Luma filter
    auto luma = std::make_shared<PostProcessingEffect>(
        QStringLiteral("luma")
        );
    auto lumaStage = std::make_shared<PostProcessingEffectStage>((uint32_t)PostProcessingEffectStage::EffectType::kContrast);
    lumaStage->loadFragmentShader(QStringLiteral(":shaders/luma.frag"));

    luma->addStage(lumaStage);
    luma->reinitialize(core);
    m_postProcessingChain->addEffect(luma);

    // Horizontal blur ---------------------------------
    auto horizontalBlur = std::make_shared<PostProcessingEffect>(
        QStringLiteral("horizontal_blur")
        );
    auto horBlurStage = std::make_shared<PostProcessingEffectStage>((uint32_t)PostProcessingEffectStage::EffectType::kContrast);
    horBlurStage->loadFragmentShader(QStringLiteral(":shaders/blur.frag"));
    
    horizontalBlur->addStage(horBlurStage);
    horizontalBlur->reinitialize(core);
    horizontalBlur->shaderPreset()->addUniform(horizontalBlur->shaderPreset()->shaderProgram()->getUniformId("horizontal"), true);
    //horizontalBlur->shaderPreset()->shaderProgram()->addBuffer(
    //    m_lightClusterGrid.screenToViewBuffer().get(),
    //    SCREEN_TO_VIEW_BUFFER_NAME);

    // Vertical blur ------------------------------------
    auto verticalBlur = std::make_shared<PostProcessingEffect>(
        QStringLiteral("vertical_blur")
        );

    auto vertBlurStage = std::make_shared<PostProcessingEffectStage>((uint32_t)PostProcessingEffectStage::EffectType::kContrast);
    vertBlurStage->loadFragmentShader(QStringLiteral(":shaders/blur.frag"));
    
    verticalBlur->addStage(vertBlurStage);
    verticalBlur->reinitialize(core);
    verticalBlur->shaderPreset()->addUniform(verticalBlur->shaderPreset()->shaderProgram()->getUniformId("horizontal"), false);
    
    // Add blur effect 5 times to expand range
    for (int i = 0; i < 5; i++) {
        m_postProcessingChain->addEffect(horizontalBlur);
        m_postProcessingChain->addEffect(verticalBlur);
    }

    // Finally, bloom
    auto bloom = std::make_shared<PostProcessingEffect>(
        QStringLiteral("bloom")
        );

    auto bloomStage = std::make_shared<PostProcessingEffectStage>((uint32_t)PostProcessingEffectStage::EffectType::kContrast);
    bloomStage->loadFragmentShader(QStringLiteral(":shaders/bloom.frag"));
    bloom->useCheckPoint(true);
    bloom->addStage(bloomStage);
    bloom->reinitialize(core);
    // FIXME: Setting this uniform doesn't seem to work
    bloom->shaderPreset()->addUniform(bloom->shaderPreset()->shaderProgram()->getUniformId("bloomIntensity"), 1.0F);
    m_postProcessingChain->addEffect(bloom);
}
    
SceneCamera::~SceneCamera()
{
}

std::vector<SortingLayer> SceneCamera::renderLayers() const
{
    ScenarioSettings& settings = m_component->sceneObject()->scene()->scenario()->settings();
    const SortingLayers& sortingLayers = settings.renderLayers();
    std::vector<SortingLayer> layers;
    for (Uint32_t layerId : m_renderLayers) {
        layers.push_back(sortingLayers.getLayerFromId(layerId));
    }
    return layers;
}

const std::vector<Uint32_t>& SceneCamera::renderLayerIds() const
{
    return m_renderLayers;
}

void SceneCamera::addRenderLayer(Uint32_t layerId)
{
    m_renderLayers.push_back(layerId);
    onRenderLayerAdded(layerId);
}

void SceneCamera::removeRenderLayer(Uint32_t layerId)
{
    auto iter = std::find_if(m_renderLayers.begin(), m_renderLayers.end(),
        [layerId](size_t lid) {
            return layerId == lid;
        });
    m_renderLayers.erase(iter);
    onRenderLayerRemoved(layerId);
}

void SceneCamera::bindTextures()
{
    m_ssaoBlurFrameBuffer.bindColorTexture(3, 0);
}

void SceneCamera::drawFrameBufferQuad(CoreEngine * core)
{
    // Post-processing
    if (m_cameraOptions.testFlag(CameraOption::kEnablePostProcessing)) {
        postProcessingChain()->postProcess();
    }

    // Draw framebuffer quad
    Camera::drawFrameBufferQuad(core);
}

void SceneCamera::resizeFrame(uint32_t width, uint32_t height)
{
    Camera::resizeFrame(width, height);

    // Resize SSAO framebuffer
    m_viewport.resizeFrameBuffer(width, height, m_ssaoFrameBuffer);
    m_viewport.resizeFrameBuffer(width, height, m_ssaoBlurFrameBuffer);

    // Resize post-processing framebuffers
    if (m_postProcessingChain) {
        int w = (m_viewport.m_width)*(width);
        int h = (m_viewport.m_height)*(height);
        m_postProcessingChain->resize(w, h);
    }
}

void SceneCamera::bindBuffers()
{
    m_lightClusterGrid.bindBufferPoints();
}

void SceneCamera::updateFrustum(CameraUpdateFlags flags)
{
    Camera::updateFrustum(flags);
    if (flags.testFlag(CameraUpdateFlag::kProjectionUpdated)) {
        m_lightClusterGrid.reconstructGrid();
    }
}

void to_json(json& orJson, const SceneCamera& korObject) 
{
    ToJson<Camera>(orJson, korObject);
    ScenarioSettings& settings = korObject.m_component->sceneObject()->scene()->scenario()->settings();

    json renderLayers = json::array();
    for (Uint32_t layer: korObject.m_renderLayers) {
        renderLayers.push_back(settings.renderLayers().getLayerNameFromId(layer).c_str());
    }
    orJson["renderLayers"] = renderLayers;

    // Camera options
    orJson["cameraOptions"] = (int)korObject.m_cameraOptions;
}

void from_json(const json& korJson, SceneCamera& orObject)
{
    FromJson<Camera>(korJson, orObject);

    // Render layers
    orObject.m_renderLayers.clear();
    ScenarioSettings& settings = orObject.m_component->sceneObject()->scene()->scenario()->settings();
    if (korJson.contains("renderLayers")) {
        for (const json& layerJson : korJson["renderLayers"]) {
            GString layerName = layerJson.get_ref<const std::string&>().c_str();
            orObject.m_renderLayers.push_back(settings.renderLayers().getLayer(layerName).id());
        }
    }

    // Camera options
    if (korJson.contains("cameraOptions")) {
        orObject.m_cameraOptions = Flags<CameraOption>(korJson.at("cameraOptions").get<Int32_t>());
    }
}

void SceneCamera::setDefaultRenderLayers()
{
    // TODO: Configure these from a file

    Scenario* scenario = m_component->sceneObject()->scene()->scenario();
    if (scenario) {
        // Normal camera
        m_renderLayers.emplace_back(scenario->settings().renderLayers().getLayer("skybox").id());
        m_renderLayers.emplace_back(scenario->settings().renderLayers().getLayer("world").id());
        m_renderLayers.emplace_back(scenario->settings().renderLayers().getLayer("effects").id());
        m_renderLayers.emplace_back(scenario->settings().renderLayers().getLayer("ui").id());
    }
    else {
        // Debug camera
    }
}

void SceneCamera::onRenderLayerAdded(Uint32_t layerId)
{
    // Generate the new draw commands for all scene objects for this camera layer
    Scene& mainScene = m_component->scene()->scenario()->scene();
    CoreEngine* engine = mainScene.engine();
    OpenGlRenderer& renderer = *engine->openGlRenderer();
    for (ModelComponent* m: mainScene.models()) 
    {
        if (!m->modelHandle()) { continue; }

        ShaderComponent* shaderComp = m->sceneObject()->getComponent<ShaderComponent>(ComponentType::kShader);
        if (!shaderComp) { continue; }

        const ShaderPreset* preset = shaderComp->shaderPreset().get();

        std::vector<std::shared_ptr<DrawCommand>>& drawCommands = m->drawCommands();
        GSimulationPlayMode playMode = engine->simulationLoop()->getPlayMode();
        switch ((ESimulationPlayMode)playMode) {
        case ESimulationPlayMode::eStandard:
        {
            if (m->sceneObject()->hasRenderLayer(layerId)) {
                m->createDrawCommands(drawCommands, *this, preset, layerId);
            }
            break;
        }
        default:
            assert(false && "Invalid");
        }
    }
}

void SceneCamera::onRenderLayerRemoved(Uint32_t layerId)
{
    // Remove the draw commands for all scene objects for this camera layer
    Scene& mainScene = m_component->scene()->scenario()->scene();
    CoreEngine* engine = mainScene.engine();
    OpenGlRenderer& renderer = *engine->openGlRenderer();
    for (ModelComponent* m : mainScene.models())
    {
        std::vector<std::shared_ptr<DrawCommand>>& drawCommands = m->drawCommands();
        std::vector<std::shared_ptr<DrawCommand>> newDrawCommands;
        newDrawCommands.reserve(drawCommands.size());
        
        GSimulationPlayMode playMode = engine->simulationLoop()->getPlayMode();
        switch ((ESimulationPlayMode)playMode) {
        case ESimulationPlayMode::eStandard:
        {
            // Check if scene object has draw commands for this render layer
            if (m->sceneObject()->hasRenderLayer(layerId)) {
                // Only keep draw commands that do not have this render layer
                for (const auto& command : drawCommands) {
                    if (command->renderLayer().id() != layerId) {
                        newDrawCommands.push_back(command);
                    }
                }
                newDrawCommands.swap(drawCommands);
            }
            break;
        }
        default:
            assert(false && "Invalid");
        }
    }
}

// End namespaces
}
