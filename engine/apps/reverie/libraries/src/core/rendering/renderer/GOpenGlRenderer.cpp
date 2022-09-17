#include "core/rendering/renderer/GOpenGlRenderer.h"

#include "core/GCoreEngine.h"
#include "core/loop/GSimLoop.h"
#include "core/debugging/GDebugManager.h"

#include "core/layer/view/widgets/graphics/GGLWidget.h"

#include "core/rendering/geometry/GVertexData.h"
#include "core/rendering/buffers/GVertexArrayObject.h"
#include "core/rendering/geometry/GPolygon.h"
#include "core/rendering/lighting/GShadowMap.h"
#include "core/rendering/models/GModel.h"
#include <core/rendering/geometry/GMesh.h>
#include <core/canvas/GGlyph.h>
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/shaders/GUniformContainer.h"

#include "core/rendering/postprocessing/GPostProcessingChain.h"
#include "core/rendering/renderer/GRenderSettings.h"
#include "fortress/containers/GSortingLayer.h"

#include "core/components/GTransformComponent.h"
#include "core/components/GCameraComponent.h"
#include "core/components/GLightComponent.h"
#include "core/components/GScriptComponent.h"
#include "core/components/GShaderComponent.h"
#include "core/components/GCanvasComponent.h"

#include "core/scene/GScenario.h"
#include "core/scene/GScene.h"
#include "core/scene/GSceneObject.h"
#include "heave/kinematics/GTransformComponents.h"
#include "fortress/containers/math/GEulerAngles.h"

#include "core/resource/GResourceCache.h"

#include "core/processes/GProcessManager.h"
#include "fortress/system/memory/GPointerTypes.h"

#include "core/rendering/renderer/GSortKey.h"
#include "core/rendering/renderer/GRenderCommand.h"

namespace rev {   


OpenGlRenderer::OpenGlRenderer(rev::CoreEngine* engine, GLWidget* widget):
    m_engine(engine),
    m_widget(widget),
    m_renderContext(m_widget->context(), m_widget->context()->surface())
{
}

OpenGlRenderer::~OpenGlRenderer()
{
}

bool OpenGlRenderer::raycastDeferredGeometry(const WorldRay & raycast, std::vector<int>& outSceneObjectIds)
{
    // TODO: Implement

    return false;
}

//ShaderProgram* OpenGlRenderer::getDefaultPrepassShader() const
//{
//    const std::shared_ptr<ResourceHandle>& handle = ResourceCache::Instance().getHandleWithName("prepass_deferred",
//        EResourceType::eShaderProgram);
//    return handle->resourceAs<ShaderProgram>();
//}

ShaderProgram* OpenGlRenderer::getSSAOShader() const
{
    const std::shared_ptr<ResourceHandle>& handle = ResourceCache::Instance().getHandleWithName("ssao",
        EResourceType::eShaderProgram);
    return handle->resourceAs<ShaderProgram>();
}

ShaderProgram* OpenGlRenderer::getSSAOBlurShader() const
{
    const std::shared_ptr<ResourceHandle>& handle = ResourceCache::Instance().getHandleWithName("ssao_blur",
        EResourceType::eShaderProgram);
    return handle->resourceAs<ShaderProgram>();
}

ShaderProgram* OpenGlRenderer::getClusterGridShader() const
{
    const std::shared_ptr<ResourceHandle>& handle = ResourceCache::Instance().getHandleWithName("light_cluster_grid",
        EResourceType::eShaderProgram);
    return handle->resourceAs<ShaderProgram>();
}

ShaderProgram* OpenGlRenderer::getLightCullingShader() const
{
    const std::shared_ptr<ResourceHandle>& handle = ResourceCache::Instance().getHandleWithName("light_culling",
        EResourceType::eShaderProgram);
    return handle->resourceAs<ShaderProgram>();
}

ShaderProgram* OpenGlRenderer::getActiveClusterShader() const
{
    const std::shared_ptr<ResourceHandle>& handle = ResourceCache::Instance().getHandleWithName("active_clusters",
        EResourceType::eShaderProgram);
    return handle->resourceAs<ShaderProgram>();
}

ShaderProgram* OpenGlRenderer::getActiveClusterCompressShader() const
{
    const std::shared_ptr<ResourceHandle>& handle = ResourceCache::Instance().getHandleWithName("active_clusters_compact",
        EResourceType::eShaderProgram);
    return handle->resourceAs<ShaderProgram>();
}

void OpenGlRenderer::initialize()
{
    initializeGlobalSettings();

    // Set up render settings
    RenderSettings::CacheSettings(m_renderContext);

    m_timer.start();
}

void OpenGlRenderer::requestResize()
{
    std::unique_lock lock(m_drawMutex);
    m_renderState.m_actionFlags.setFlag(RenderFrameState::kResize, true);
}

void OpenGlRenderer::preDraw()
{
    ESimulationPlayMode mode = m_engine->simulationLoop()->getPlayMode();

    // Update the selected pixel color for each camera
    Vector2 widgetMousePos = m_widget->inputHandler().mouseHandler().widgetMousePosition();
    if (mode == ESimulationPlayMode::eStandard) {
        // Iterate through cameras to draw textured quads
        std::vector<CameraComponent*> cameras = m_engine->scenario()->scene().cameras();
        for (const auto& cameraComp : cameras)
        {
            // Cache the currently selected color
            SceneCamera& camera = cameraComp->camera();
            camera.mousePicker().updateMouseOver(widgetMousePos, camera, *this);
        }
    }
    else {
        SceneCamera& debugCamera = m_engine->debugManager()->camera()->camera();
        if (debugCamera.frameBuffers().writeBuffer().isComplete()) {
            // Cache the currently selected color
            debugCamera.mousePicker().updateMouseOver(widgetMousePos, debugCamera, *this);
        }
    }

    // Reset between each render pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Set play mode
    m_renderState.m_playMode = mode;

    // Flush write data for any buffer queues
    m_renderContext.flushBuffers();

    // Swap read/write buffers
    m_renderContext.swapBuffers();
}

void OpenGlRenderer::depthPrePass()
{
    // Fill the z-buffer
    m_renderState.m_stage = RenderFrameState::kDepthPrepass;

    m_renderContext.flushSetting<DepthSetting>(
        true,
        DepthPassMode::kLess,
        true);

    // Was turning off color rendering, but now rendering normals into FBO for SSAO
    //glColorMask(0, 0, 0, 0); 

    // Draw commands
    size_t numCommands = m_renderCommands.size();
    for (size_t i = 0; i < numCommands; i++) {
        m_renderCommands[i]->depthPrePass(*this);
    }

    // Unbind most recently used camera's framebuffer
    if (m_renderState.m_camera) {
        m_renderState.m_camera->releaseFrame(FrameBuffer::BindType::kWrite);
        m_renderState.m_camera = nullptr;
    }

    //if (m_renderCommands.size()) {
    //    auto& framebuffer = static_cast<SceneCamera*>(std::static_pointer_cast<DrawCommand>(m_renderCommands.front())->camera())->frameBuffer();
    //    //framebuffer.saveColorToFile(0, "C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\bufferColor.jpg");
    //    framebuffer.saveColorToFile(1, "C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\bufferNormal.jpg");
    //}
}

void OpenGlRenderer::shadowMappingPass()
{
    // Clear shadow maps (they aren't cleared on camera switch)
    // Note: I was clearing on camera switch, but if the framebuffers
    // for each light type are cleared, the whole texture is cleared,
    // so all other shadow maps for a light type will be cleared
    Scene& scene = m_engine->scenario()->scene();
    std::vector<ShadowMap*>& shadowMaps = m_renderContext.lightingSettings().shadowMaps();
    size_t numShadowMaps = shadowMaps.size();
    for (size_t i = 0; i < numShadowMaps; i++) {
        ShadowMap* map = shadowMaps[i];
        map->camera()->bindFrame(FrameBuffer::BindType::kWrite, true);
        map->camera()->releaseFrame(FrameBuffer::BindType::kWrite);
    }


    // Fill the z-buffer
    m_renderState.m_stage = RenderFrameState::kShadowMapping;

    m_renderContext.flushSetting<DepthSetting>(
        true,
        DepthPassMode::kLess,
        true);

    // Turn off color rendering for framebuffer
    glColorMask(0, 0, 0, 0);

    // TODO: Cull front-face instead of back-face for only enclosed objects to avoid peter panning
    m_renderContext.flushSetting<CullFaceSetting>(true, CulledFace::kFront);

    // Draw commands
    for (std::shared_ptr<DrawCommand>& command : m_shadowMapCommands) {
        command->shadowPass(*this);
    }

    // Unbind most recently used camera's framebuffer
    if (m_renderState.m_camera) {
        m_renderState.m_camera->releaseFrame(FrameBuffer::BindType::kWrite);
        m_renderState.m_camera = nullptr;
    }

    // Return to standard face culling (which is no culling, can cull individual models)
    m_renderContext.flushSetting<CullFaceSetting>(false, CulledFace::kBack);


    //if (m_shadowMapCommands.size()) {
    //    //auto& framebuffer = m_shadowMapCommands.front()->camera()->frameBuffer();
    //    //framebuffer.saveDepthToFile("C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\depth.jpg");
    //    auto& pointTexture = m_renderContext.lightingSettings().shadowTextures()[0];
    //    pointTexture->getImage(PixelFormat::kDepth, PixelType::kUByte8);
    //}
}

void OpenGlRenderer::render()
{
    std::unique_lock lock(m_drawMutex);

    // Don't render if screen size is zero
    if (widget()->width() == 0 || widget()->height() == 0) {
        return;
    }

    preDraw();

    const ESimulationPlayMode mode = m_engine->simulationLoop()->getPlayMode();

    if (m_lightingFlags.testFlag(kDepthPrePass)) {
        depthPrePass();
    }

    if (m_lightingFlags.testFlag(kDynamicShadows)) {
        shadowMappingPass();
    }

    if (lightingFlags().testFlag(OpenGlRenderer::kClustered)) {
        clusteredLightingPass(mode);
    }

    if (m_lightingFlags.testFlag(kSSAO)) {
        ssaoPass(mode);
    }

    renderPass(mode);

    postProcessingPass(mode);

    // Clear render state for the next frame
    m_renderState = RenderFrameState();
    m_renderState.m_previousPlayMode = mode;
    m_renderContext.setBoundMaterial(-1);
}

void OpenGlRenderer::processScenes()
{
    // Create draw commands for scene
    Scene& scene = m_engine->scenario()->scene();
    scene.retrieveDrawCommands(*this);

    // Grab debug visuals to render
    ESimulationPlayMode mode = m_engine->simulationLoop()->getPlayMode();
    if (mode == ESimulationPlayMode::eDebug) {
        m_engine->debugManager()->createDrawCommands(scene, *this);
    }

    // Pre-processing before queuing for render
    preprocessCommands();

    // Lock draw mutex for swap of command vectors
    std::unique_lock lock(m_drawMutex);

    // Queue up commands to render scene, first caching previous render commands
    m_readRenderCommands.swap(m_renderCommands);
    m_renderCommands.swap(m_receivedCommands);
    m_receivedCommands.clear();

    // Queue up commands to render shadow maps
    m_shadowMapCommands.swap(m_receivedShadowMapCommands);
    m_receivedShadowMapCommands.clear();

    // Reserve a rough estimate of space for received commands
    Uint32_t sceneObjectCount = scene.topLevelSceneObjects().size();
    m_receivedCommands.reserve(sceneObjectCount);
    m_receivedShadowMapCommands.reserve(sceneObjectCount);
}


void OpenGlRenderer::addRenderCommand(const std::shared_ptr<RenderCommand>& command)
{
    //QMutexLocker lock(&m_drawMutex);
    command->onAddToQueue();
    m_receivedCommands.push_back(command);
}

void OpenGlRenderer::addShadowMapCommand(const std::shared_ptr<DrawCommand>& command)
{
    //QMutexLocker lock(&m_drawMutex);
    command->onAddToQueue();
    m_receivedShadowMapCommands.push_back(command);
}

void OpenGlRenderer::clusteredLightingPass(ESimulationPlayMode mode)
{
    if (mode == ESimulationPlayMode::eDebug) {
        // Perform lighting for only debug camera
        SceneCamera& debugCamera = m_engine->debugManager()->camera()->camera();

        // Cull lights by assigning them to each cluster according to their position
        debugCamera.lightClusterGrid().cullLights(*this);
    }
    else {
        Scene& scene = m_engine->scenario()->scene();
        size_t count = 0;
        for (CameraComponent* camComp : scene.cameras()) {
            SceneCamera& cam = camComp->camera();

            // Cull lights by assigning them to each cluster according to their position
            cam.lightClusterGrid().cullLights(*this);

            count++;
        }
    }
}

void OpenGlRenderer::ssaoPass(ESimulationPlayMode mode)
{
    // Don't write to depth buffer
    m_renderContext.flushSetting<DepthSetting>(
        true,
        DepthPassMode::kAlways,
        false);

    // Turn color rendering back on
    glColorMask(1, 1, 1, 1);

    if (mode == ESimulationPlayMode::eStandard) {
        Scene& scene = m_engine->scenario()->scene();
        for (const auto& cameraComp : scene.cameras())
        {
            ssaoCameraPass(cameraComp->camera());
        }
    }
    else {
        SceneCamera& debugCamera = m_engine->debugManager()->camera()->camera();
        if (debugCamera.frameBuffers().writeBuffer().isComplete()) {
            ssaoCameraPass(debugCamera);
        }
    }

    //if (m_renderCommands.size()) {
    //    auto& framebuffer = static_cast<SceneCamera*>(std::static_pointer_cast<DrawCommand>(m_renderCommands.front())->camera())->frameBuffer();
    //    //framebuffer.saveColorToFile(0, "C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\bufferColor.jpg");
    //    framebuffer.saveColorToFile(1, "C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\bufferNormal.jpg");
    //}
}

void OpenGlRenderer::ssaoCameraPass(SceneCamera& camera)
{
    if (!m_renderContext.isCurrent()) {
        Logger::Throw("Error, context should be current");
    }

    // Return if viewport is not full-screen, uv coordinates for ssao texture not scaling correctly
    // FIXME
    //if ((camera.viewport().m_width != 1) || (camera.viewport().m_height != 1)) {
    //    return;
    //}

    // Draw SSAO -----------------------------------------------
    // Bind textures
    // Must be bound before FBO, since FBO gets unbound from this
    LightingSettings& ls = m_renderContext.lightingSettings();

    // Bind depth texture
    // Note, binding from read framebuffer to avoid a pipeline stall (not sure how necessary this is)
    camera.frameBuffers().writeBuffer().bindDepthTexture(0);
    //glActiveTexture(GL_TEXTURE0 + 0);
    //size_t depthTex = Texture::TextureBoundToActiveUnit();
    //size_t correctDepth = camera.frameBuffer().blitBuffer()->depthTexture()->getID();

    // Bind normals texture
    camera.frameBuffers().writeBuffer().bindColorTexture(1, SceneCamera::kNormals); // bind color attachment 1 (normals) to texunit 1
    //glActiveTexture(GL_TEXTURE0 + 1);
    //size_t normalTex = Texture::TextureBoundToActiveUnit();
    //size_t correctNormal = camera.frameBuffer().blitBuffer()->colorTextures()[1]->getID();

    // Bind noise texture
    ls.noiseTexture()->bind(2);
    //glActiveTexture(GL_TEXTURE0 + 2);
    //size_t noiseTex = Texture::TextureBoundToActiveUnit();
    //size_t correctNoise = ls.noiseTexture()->getID();

    // Bind SSAO FBO
    FrameBuffer& ssaoFrameBuffer = camera.ssaoFrameBuffer();
    ssaoFrameBuffer.bind();

    // Set viewport
    camera.viewport().setGLViewport(ssaoFrameBuffer);

    // Clear buffer
    static Color clearColor = Color(Vector4(0.95f, 0.3f, 0.23f, 0.0f));    // blue violet
    ssaoFrameBuffer.clear(clearColor);

    // Bind SSAO shader
    ShaderProgram* ssaoShader = getSSAOShader();
    ssaoShader->bind();

    // Bind kernel buffer
    ls.ssaoBuffer().bindToPoint();

    // Set uniforms
    // TODO: Think up a more precise scaling for bias
    int noiseSize = ls.noiseSize();
    constexpr float bias = -0.002f;
    float radius = ls.ssaoRadius();
    int numKernelSamples = ls.numKernelSamples();

    camera.bindUniforms(this);
    UniformContainer& uc = m_renderContext.uniformContainer();

    /// @todo Set these uniform values only when their underlying values change
    static const Vector2 s_scale(1.0, 1.0);
    static const Vector3 s_offsets = Vector3(0, 0, 0);
    m_ssaoUniforms.m_scale.setValue(s_scale, uc);
    m_ssaoUniforms.m_offsets.setValue(s_offsets, uc);
    m_ssaoUniforms.m_noiseSize.setValue(noiseSize, uc);
    m_ssaoUniforms.m_kernelSize.setValue(numKernelSamples, uc);
    m_ssaoUniforms.m_bias.setValue(bias, uc);
    m_ssaoUniforms.m_radius.setValue(radius, uc);

    ssaoShader->setUniformValue(
        ssaoShader->uniformMappings().m_noiseSize,
        m_ssaoUniforms.m_noiseSize);

    ssaoShader->setUniformValue(
        ssaoShader->uniformMappings().m_offsets, 
        m_ssaoUniforms.m_offsets);

    ssaoShader->setUniformValue(
        ssaoShader->uniformMappings().m_scale,
        m_ssaoUniforms.m_scale);

    ssaoShader->setUniformValue(
        ssaoShader->uniformMappings().m_kernelSize,
        m_ssaoUniforms.m_kernelSize);

    ssaoShader->setUniformValue(
        ssaoShader->uniformMappings().m_bias, 
        m_ssaoUniforms.m_bias);

    ssaoShader->setUniformValue(
        ssaoShader->uniformMappings().m_radius, 
        m_ssaoUniforms.m_radius);

    ssaoShader->updateUniforms(uc);

    // Draw the quad
    Mesh* quad = ResourceCache::Instance().polygonCache()->getSquare();
    quad->vertexData().drawGeometry(PrimitiveMode::kTriangles, 1);

    // Release kernel buffer
    m_renderContext.lightingSettings().ssaoBuffer().releaseFromPoint();

    // Release shader
    ssaoShader->release();

    // Release FBO
    ssaoFrameBuffer.release();

#ifdef DEBUG_MODE
    bool error = printGLError("Error during SSAO pass");
    if (error) {
        Logger::LogInfo("Error during SSAO pass");
    }
#endif

    //if (m_renderCommands.size()) {
    //    ssaoFrameBuffer.saveColorToFile(0, "C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\ssao.jpg");
    //    
    //    //std::vector<int> data;
    //    //data.resize(2000* 2000);
    //    //ssaoFrameBuffer.colorTextures()[0]->getData(data.data(), PixelFormat::kRed, PixelType::kByte8);
    //    //float max = *std::max_element(data.begin(), data.end());
    //}

    // Blur SSAO -----------------------------------------------
    // Set textures
    ssaoFrameBuffer.bindColorTexture(3, 0); // Bind attachment 0 to texunit 3

    // Bind SSAO blur FBO
    FrameBuffer& ssaoBlurFrameBuffer = camera.ssaoBlurFrameBuffer();
    ssaoBlurFrameBuffer.bind();

    // Set viewport
    camera.viewport().setGLViewport(ssaoBlurFrameBuffer);

    // Clear buffer
    ssaoBlurFrameBuffer.clear(clearColor);

    // Bind shader
    ShaderProgram* ssaoBlurShader = getSSAOBlurShader();
    ssaoBlurShader->bind();
    
    // Set uniforms
    ssaoBlurShader->setUniformValue(
        ssaoBlurShader->uniformMappings().m_offsets, 
        m_ssaoUniforms.m_offsets);
    ssaoBlurShader->setUniformValue(
        ssaoBlurShader->uniformMappings().m_scale,
        m_ssaoUniforms.m_scale);
    ssaoBlurShader->updateUniforms(uc);

    // Draw the quad
    quad->vertexData().drawGeometry(PrimitiveMode::kTriangles, 1);

    ssaoBlurShader->release();

    // Release FBO
    ssaoBlurFrameBuffer.release();
}


void OpenGlRenderer::renderPass(ESimulationPlayMode mode)
{
    if (m_lightingFlags.testFlag(kDepthPrePass)) {
        // Restore settings for render pass
        m_renderState.m_camera = nullptr;
        m_renderState.m_stage = RenderFrameState::kRender;

        // Don't write to depth buffer
        m_renderContext.flushSetting<DepthSetting>(
            true,
            DepthPassMode::kLessEqual,
            false);

        // Ensure that color rendering is turned on
        glColorMask(1, 1, 1, 1);
    }

    m_renderState.m_stage = RenderFrameState::kRender;

    // Perform draw commands
    uint32_t numCommands = (uint32_t)m_renderCommands.size();
    for (uint32_t i = 0; i < numCommands; i++) {
        m_renderCommands[i]->perform(*this, i);
    }

    // Unbind most recently used camera's framebuffer
    if (m_renderState.m_camera) {
        m_renderState.m_camera->releaseFrame(FrameBuffer::BindType::kWrite);
        m_renderState.m_camera = nullptr;
    }

    // Draw debug visuals if in debug mode
    if (mode == ESimulationPlayMode::eDebug) {
        SceneCamera& debugCamera = m_engine->debugManager()->camera()->camera();
        debugCamera.frameBuffers().writeBuffer().bind();
        m_engine->debugManager()->draw(&m_engine->scenario()->scene());
        debugCamera.frameBuffers().writeBuffer().release();

        // Save scene to file
        //debugCamera.frameBuffer().saveColorToFile(0, "C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\buffer.jpg");
        //debugCamera.frameBuffer().saveDepthToFile("C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\depth.jpg");
    }
}

void OpenGlRenderer::postProcessingPass(ESimulationPlayMode mode)
{
    glViewport(0, 0, widget()->pixelWidth(), widget()->pixelHeight());

    m_renderContext.flushSetting<DepthSetting>(
        true,
        DepthPassMode::kLessEqual,
        true);

    if (mode == ESimulationPlayMode::eStandard) {
        Scene& scene = m_engine->scenario()->scene();

        // Sort the cameras in the scene from farthest to nearest (for transparency)
        std::vector<CameraComponent*>& cameras = scene.cameras();
        std::sort(cameras.begin(), cameras.end(),
            [](CameraComponent* c1, CameraComponent* c2) {
                return c1->camera().getViewport().getDepth() > c2->camera().getViewport().getDepth();
            });

        // Set blend to enable transparency
        RenderSettings settings = RenderSettings();
        settings.addDefaultBlend();
        settings.bind(m_renderContext);

        // Iterate through cameras to draw textured quads
        for (const auto& cameraComp : scene.cameras())
        {
            if (cameraComp->isEnabled()) {

                // Draw camera output
                cameraComp->camera().drawFrameBufferQuad(m_engine);

                // For debugging, save normals to file
                //cameraComp->camera().frameBuffers().readBuffer().saveColorToFile(1, "C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\main.jpg");

                // Swap read/write FBOs
                cameraComp->camera().frameBuffers().swapBuffers();
            }
        }

        // Restore previous settings
        settings.release(m_renderContext);
    }
    else {
        SceneCamera& debugCamera = m_engine->debugManager()->camera()->camera();
        if (debugCamera.frameBuffers().writeBuffer().isComplete()) {
            // Draw camera output
            m_engine->debugManager()->camera()->camera().drawFrameBufferQuad(m_engine);
            //auto quadShaderProgram = ResourceCache::Instance().getHandleWithName("quad",
                //EResourceType::eShaderProgram)->resourceAs<ShaderProgram>();
            //debugCamera.ssaoBlurFrameBuffer().drawQuad(debugCamera, *quadShaderProgram);

            // Swap read/write FBOs
            m_engine->debugManager()->camera()->camera().frameBuffers().swapBuffers();
        }
    }
}

void OpenGlRenderer::initializeGlobalSettings()
{
    printGLError("Error on GL startup");

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable back-face culling
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    // Set background color
    glClearColor(0.55f, 0.6f, 0.93f, 0.0f); // blue violet
    glEnable(GL_DEPTH_TEST);
    
    // Disable MSAA, since framebuffers use SMAA
    glDisable(GL_MULTISAMPLE);
    //glEnable(GL_MULTISAMPLE); // Enable MSAA

    // Clear all colors on screen from the last frame
    glClear(GL_COLOR_BUFFER_BIT);

    printGLError("Error on GL initialization");
}

void OpenGlRenderer::preprocessCommands()
{
    // Perform pre-processing of commands
    for (const auto& command : m_receivedCommands) {
        command->preSort();
    }
    for (const auto& command : m_receivedShadowMapCommands) {
        command->preSort();
    }

    DrawCommand::ResetDepths();

    // Sort commands
    std::sort(m_receivedCommands.begin(),
        m_receivedCommands.end(),
        [](const std::shared_ptr<RenderCommand>& c1, std::shared_ptr<RenderCommand>& c2) {
        return c1->sortKey() < c2->sortKey();
    });

    std::sort(m_receivedShadowMapCommands.begin(),
        m_receivedShadowMapCommands.end(),
        [](const std::shared_ptr<DrawCommand>& c1, std::shared_ptr<DrawCommand>& c2) {
        return c1->sortKey() < c2->sortKey();
    });

    // For color picking draw pass
    m_commandIndexUniforms.ensureSize(m_receivedCommands.size());
}

// End namespaces
}