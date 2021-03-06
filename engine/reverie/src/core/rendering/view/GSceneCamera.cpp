#include "GSceneCamera.h"

#include "../../GCoreEngine.h"
#include "../../scene/GScenario.h"
#include "../../scene/GSceneObject.h"
#include "../../scene/GScene.h"
#include "../../components/GCameraComponent.h"
#include "../../components/GTransformComponent.h"
#include "../../components/GShaderComponent.h"
#include "../../components/GCanvasComponent.h"
#include "../../components/GCubeMapComponent.h"

#include "../../resource/GResourceCache.h"
#include "../models/GModel.h"
#include "../shaders/GShaderProgram.h"
#include "../buffers/GUniformBufferObject.h"
#include "../materials/GMaterial.h"
#include "../postprocessing/GPostProcessingChain.h"
#include "../postprocessing/GPostProcessingEffect.h"

#include "../../loop/GSimLoop.h"
#include "../../debugging/GDebugManager.h"

#include "../renderer/GMainRenderer.h"
#include "../renderer/GRenderCommand.h"

#include "../../../view/GWidgetManager.h"
#include "../../../view/GL/GGLWidget.h"
#include "../../containers/GFlags.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Scene Camera
//////////////////////////////////////////////////////////////////////////////////////////////////
SceneCamera::SceneCamera(CameraComponent* component):
    Camera(&component->sceneObject()->transform(),
        component->sceneObject()->scene()->engine()->getGLWidgetContext(),
        AliasingType::kMSAA,
        FrameBuffer::BufferAttachmentType::kRBO,
        16, // 16 samples for MSAA
        3), // Three color buffers. First for actual color, second for normals (for SSAO) during prepass, 
        // third for draw command IDs
    m_component(component),
    m_cameraOptions(3),
    m_lightClusterGrid(this), 
    m_ssaoFrameBuffer(component->sceneObject()->scene()->engine()->getGLWidgetContext(),
        AliasingType::kDefault,
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
        FBO_SSAO_TEX_FORMAT,
        //TextureFormat::kR16,
        //FBO_DEFAULT_TEX_FORMAT,
        4, // Not using MSAA, so doesn't matter
        1, // One color attachments needed
        false) // No depth
{
    // Add render projection to GL widget
    CoreEngine* core = component->sceneObject()->scene()->engine();
    View::GLWidget* mainGLWidget = core->widgetManager()->mainGLWidget();
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

    auto hdrStage = std::make_shared<PostProcessingEffectStage>((size_t)PostProcessingEffectStage::EffectType::kReinhardToneMapping);
    hdrStage->loadFragmentShader(QStringLiteral(":shaders/hdr.frag"));
    effect->addStage(hdrStage);

    auto contrastStage = std::make_shared<PostProcessingEffectStage>((size_t)PostProcessingEffectStage::EffectType::kContrast);
    contrastStage->loadFragmentShader(QStringLiteral(":shaders/contrast.frag"));
    effect->addStage(contrastStage);

    effect->reinitialize(core);
    effect->setCheckPoint(true);
    m_postProcessingChain->addEffect(effect);

    // Luma filter
    auto luma = std::make_shared<PostProcessingEffect>(
        QStringLiteral("luma")
        );
    auto lumaStage = std::make_shared<PostProcessingEffectStage>((size_t)PostProcessingEffectStage::EffectType::kContrast);
    lumaStage->loadFragmentShader(QStringLiteral(":shaders/luma.frag"));

    luma->addStage(lumaStage);
    luma->reinitialize(core);
    m_postProcessingChain->addEffect(luma);

    // Horizontal blur ---------------------------------
    auto horizontalBlur = std::make_shared<PostProcessingEffect>(
        QStringLiteral("horizontal_blur")
        );
    auto horBlurStage = std::make_shared<PostProcessingEffectStage>((size_t)PostProcessingEffectStage::EffectType::kContrast);
    horBlurStage->loadFragmentShader(QStringLiteral(":shaders/blur.frag"));
    
    horizontalBlur->addStage(horBlurStage);
    horizontalBlur->reinitialize(core);
    horizontalBlur->shaderPreset()->addUniform(Uniform("horizontal", true));
    //horizontalBlur->shaderPreset()->shaderProgram()->addBuffer(
    //    m_lightClusterGrid.screenToViewBuffer().get(),
    //    SCREEN_TO_VIEW_BUFFER_NAME);

    // Vertical blur ------------------------------------
    auto verticalBlur = std::make_shared<PostProcessingEffect>(
        QStringLiteral("vertical_blur")
        );

    auto vertBlurStage = std::make_shared<PostProcessingEffectStage>((size_t)PostProcessingEffectStage::EffectType::kContrast);
    vertBlurStage->loadFragmentShader(QStringLiteral(":shaders/blur.frag"));
    
    verticalBlur->addStage(vertBlurStage);
    verticalBlur->reinitialize(core);
    verticalBlur->shaderPreset()->addUniform(Uniform("horizontal", false));
    
    // Add blur effect 5 times to expand range
    for (int i = 0; i < 5; i++) {
        m_postProcessingChain->addEffect(horizontalBlur);
        m_postProcessingChain->addEffect(verticalBlur);
    }

    // Finally, bloom
    auto bloom = std::make_shared<PostProcessingEffect>(
        QStringLiteral("bloom")
        );

    auto bloomStage = std::make_shared<PostProcessingEffectStage>((size_t)PostProcessingEffectStage::EffectType::kContrast);
    bloomStage->loadFragmentShader(QStringLiteral(":shaders/bloom.frag"));
    bloom->useCheckPoint(true);
    bloom->addStage(bloomStage);
    bloom->reinitialize(core);
    // FIXME: Setting this uniform doesn't seem to work
    bloom->shaderPreset()->addUniform(Uniform("bloomIntensity", (float)1.0));
    m_postProcessingChain->addEffect(bloom);
}
//////////////////////////////////////////////////////////////////////////////////////////////////    
SceneCamera::~SceneCamera()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void SceneCamera::bindTextures()
{
    m_ssaoBlurFrameBuffer.bindColorTexture(3, 0);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void SceneCamera::drawFrameBufferQuad(CoreEngine * core)
{
    // Post-processing
    if (m_cameraOptions.testFlag(CameraOption::kEnablePostProcessing)) {
        postProcessingChain()->postProcess();
    }

    // Draw framebuffer quad
    Camera::drawFrameBufferQuad(core);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void SceneCamera::resizeFrame(size_t width, size_t height)
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
//////////////////////////////////////////////////////////////////////////////////////////////////
void SceneCamera::bindBuffers()
{
    m_lightClusterGrid.bindBufferPoints();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void SceneCamera::updateFrustum(CameraUpdateFlags flags)
{
    Camera::updateFrustum(flags);
    if (flags.testFlag(CameraUpdateFlag::kProjectionUpdated)) {
        m_lightClusterGrid.reconstructGrid();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue SceneCamera::asJson(const SerializationContext& context) const
{
    QJsonObject object = Camera::asJson(context).toObject();

    QJsonArray renderLayers;
    std::vector<SortingLayer*> rLayers = SceneCamera::renderLayers();
    for (SortingLayer* layer: rLayers) {
        renderLayers.push_back(layer->getName().c_str());
    }
    object.insert("renderLayers", renderLayers);

    // Camera options
    object.insert("cameraOptions", (int)m_cameraOptions);

    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void SceneCamera::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    const QJsonObject& object = json.toObject();

    Camera::loadFromJson(json);

    // Render layers
    m_renderLayers.clear();
    ScenarioSettings& settings = m_component->sceneObject()->scene()->scenario()->settings();
    if (object.contains("renderLayers")) {
        for (const auto& layerJson : object["renderLayers"].toArray()) {
            QString layerName = layerJson.toString();
            m_renderLayers.push_back(settings.renderLayers().getLayer(layerName)->id());
        }
    }

    // Camera options
    if (object.contains("cameraOptions")) {
        m_cameraOptions = Flags<CameraOption>::toQFlags(object["cameraOptions"].toInt());
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void SceneCamera::setDefaultRenderLayers()
{
    // TODO: Configure these from a file

    Scenario* scenario = m_component->sceneObject()->scene()->scenario();
    if (scenario) {
        // Normal camera
        m_renderLayers.emplace_back(scenario->settings().renderLayers().getLayer("skybox")->id());
        m_renderLayers.emplace_back(scenario->settings().renderLayers().getLayer("world")->id());
        m_renderLayers.emplace_back(scenario->settings().renderLayers().getLayer("effects")->id());
        m_renderLayers.emplace_back(scenario->settings().renderLayers().getLayer("ui")->id());
    }
    else {
        // Debug camera
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<SortingLayer*> SceneCamera::renderLayers() const
{
    // Get render layers
    std::vector<SortingLayer*> layers;
    for (size_t layerId : m_renderLayers) {
        SortingLayer* layer = m_component->scene()->scenario()->settings().renderLayers().getLayerFromId(layerId);
        layers.push_back(layer);
    }
    return layers;
}



//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}
