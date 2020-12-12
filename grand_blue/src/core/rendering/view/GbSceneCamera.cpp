#include "GbSceneCamera.h"

#include "../../GbCoreEngine.h"
#include "../../scene/GbScenario.h"
#include "../../scene/GbSceneObject.h"
#include "../../scene/GbScene.h"
#include "../../components/GbCameraComponent.h"
#include "../../components/GbTransformComponent.h"
#include "../../components/GbShaderComponent.h"
#include "../../components/GbCanvasComponent.h"
#include "../../components/GbCubeMapComponent.h"

#include "../../resource/GbResourceCache.h"
#include "../models/GbModel.h"
#include "../shaders/GbShaders.h"
#include "../buffers/GbUniformBufferObject.h"
#include "../materials/GbMaterial.h"
#include "../postprocessing/GbPostProcessingChain.h"
#include "../postprocessing/GbPostProcessingEffect.h"

#include "../../loop/GbSimLoop.h"
#include "../../debugging/GbDebugManager.h"

#include "../renderer/GbMainRenderer.h"
#include "../renderer/GbRenderCommand.h"

#include "../../../view/GbWidgetManager.h"
#include "../../../view/GL/GbGLWidget.h"
#include "../../containers/GbFlags.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Scene Camera
//////////////////////////////////////////////////////////////////////////////////////////////////
SceneCamera::SceneCamera(CameraComponent* component):
    Camera(component->sceneObject()->transform().get(),
        component->sceneObject()->engine()->getGLWidgetContext(),
        AliasingType::kMSAA,
        FrameBuffer::BufferAttachmentType::kRBO,
        16, // 16 samples for MSAA
        2), // Two color buffers for SSAO
    m_component(component),
    m_cameraOptions(3),
    m_lightClusterGrid(this), 
    m_ssaoFrameBuffer(component->sceneObject()->engine()->getGLWidgetContext(),
        AliasingType::kDefault,
        FrameBuffer::BufferAttachmentType::kTexture,
        FBO_SSAO_TEX_FORMAT,
        //TextureFormat::kR16,
        //FBO_DEFAULT_TEX_FORMAT,
        4, // Not using MSAA, so doesn't matter
        1, // One color attachments needed
        false), // No depth
    m_ssaoBlurFrameBuffer(
        component->sceneObject()->engine()->getGLWidgetContext(),
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
    View::GLWidget* mainGLWidget = component->sceneObject()->engine()->widgetManager()->mainGLWidget();
    mainGLWidget->addRenderProjection(&m_renderProjection);

    // Initialize shader storage buffers
    m_lightClusterGrid.initializeBuffers(mainGLWidget->renderer()->renderContext());
    m_renderProjection.updateProjection(); // Update again, this time with light cluster grid

    // Add default render layers
    setDefaultRenderLayers();

    // Add default post-processing effects
    m_postProcessingChain = std::make_shared<PostProcessingChain>(this, &mainGLWidget->renderer()->renderContext());


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

    effect->reinitialize();
    effect->setCheckPoint(true);
    m_postProcessingChain->addEffect(effect);

    // Luma filter
    auto luma = std::make_shared<PostProcessingEffect>(
        QStringLiteral("luma")
        );
    auto lumaStage = std::make_shared<PostProcessingEffectStage>((size_t)PostProcessingEffectStage::EffectType::kContrast);
    lumaStage->loadFragmentShader(QStringLiteral(":shaders/luma.frag"));

    luma->addStage(lumaStage);
    luma->reinitialize();
    m_postProcessingChain->addEffect(luma);

    // Horizontal blur ---------------------------------
    auto horizontalBlur = std::make_shared<PostProcessingEffect>(
        QStringLiteral("horizontal_blur")
        );
    auto horBlurStage = std::make_shared<PostProcessingEffectStage>((size_t)PostProcessingEffectStage::EffectType::kContrast);
    horBlurStage->loadFragmentShader(QStringLiteral(":shaders/blur.frag"));
    
    horizontalBlur->addStage(horBlurStage);
    horizontalBlur->reinitialize();
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
    verticalBlur->reinitialize();
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
    bloom->reinitialize();
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
QJsonValue SceneCamera::asJson() const
{
    QJsonObject object = Camera::asJson().toObject();

    QJsonArray renderLayers;
    for (const std::shared_ptr<SortingLayer>& layer: getRenderLayers()) {
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
            m_renderLayers.push_back(settings.renderLayer(layerName));
        }
    }

    // Camera options
    if (object.contains("cameraOptions")) {
        m_cameraOptions = Flags::toFlags<CameraOption>(object["cameraOptions"].toInt());
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::shared_ptr<SortingLayer>> SceneCamera::getRenderLayers() const
{
    std::vector<std::shared_ptr<SortingLayer>> layers;
    for (std::weak_ptr<SortingLayer> weakPtr : m_renderLayers) {
        if (std::shared_ptr<SortingLayer> ptr = weakPtr.lock()) {
            layers.push_back(ptr);
        }
    }
    
    return layers;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void SceneCamera::setDefaultRenderLayers()
{
    // TODO: Configure these from a file

    Scenario* scenario = m_component->sceneObject()->scene()->scenario();
    if (scenario) {
        // Normal camera
        m_renderLayers.emplace_back(scenario->settings().renderLayer("skybox"));
        m_renderLayers.emplace_back(scenario->settings().renderLayer("world"));
        m_renderLayers.emplace_back(scenario->settings().renderLayer("effects"));
        m_renderLayers.emplace_back(scenario->settings().renderLayer("ui"));
    }
    else {
        // Debug camera
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::shared_ptr<SortingLayer>> SceneCamera::renderLayers()
{
    // Get render layers
    std::vector<std::shared_ptr<SortingLayer>> layers = getRenderLayers();

    // Clear any layers that have gone out of scope
    if (layers.size() < m_renderLayers.size()) {
        m_renderLayers.clear();
        for (const std::shared_ptr<SortingLayer>& ptr : layers) {
            m_renderLayers.push_back(ptr);
        }
    }

    return layers;
}



//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}
