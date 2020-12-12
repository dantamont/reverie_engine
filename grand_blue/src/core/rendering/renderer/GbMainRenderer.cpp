#include "GbMainRenderer.h"

#include "../../GbCoreEngine.h"
#include "../../loop/GbSimLoop.h"
#include "../../debugging/GbDebugManager.h"

#include "../../../view/GL/GbGLWidget.h"

#include "../geometry/GbVertexData.h"
#include "../geometry/GbBuffers.h"
#include "../geometry/GbPolygon.h"
#include "../lighting/GbShadowMap.h"
#include "../models/GbModel.h"
#include "../shaders/GbShaders.h"

#include "../postprocessing/GbPostProcessingChain.h"
#include "GbRenderSettings.h"
#include "../../containers/GbSortingLayer.h"

#include "../../components/GbTransformComponent.h"
#include "../../components/GbCameraComponent.h"
#include "../../components/GbLightComponent.h"
#include "../../components/GbScriptComponent.h"
#include "../../components/GbShaderComponent.h"
#include "../../components/GbCanvasComponent.h"

#include "../../scene/GbScenario.h"
#include "../../scene/GbScene.h"
#include "../../scene/GbSceneObject.h"
#include "../../geometry/GbTransformComponents.h"
#include "../../geometry/GbEulerAngles.h"

#include "../../resource/GbResourceCache.h"
#include "../../readers/models/GbOBJReader.h"

#include "../../processes/GbProcessManager.h"
#include "../../utils/GbMemoryManager.h"

#include "GbSortKey.h"
#include "GbRenderCommand.h"

namespace Gb {   

/////////////////////////////////////////////////////////////////////////////////////////////
MainRenderer::MainRenderer(Gb::CoreEngine* engine, View::GLWidget* widget):
    m_engine(engine),
    m_widget(widget),
    m_renderContext(this)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
MainRenderer::~MainRenderer()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
//std::shared_ptr<ShaderProgram> MainRenderer::getDefaultPrepassShader() const
//{
//    const std::shared_ptr<ResourceHandle>& handle = m_engine->resourceCache()->getHandleWithName("prepass_deferred",
//        Resource::ResourceType::kShaderProgram);
//    return handle->resourceAs<ShaderProgram>();
//}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ShaderProgram> MainRenderer::getSSAOShader() const
{
    const std::shared_ptr<ResourceHandle>& handle = m_engine->resourceCache()->getHandleWithName("ssao",
        Resource::ResourceType::kShaderProgram);
    return handle->resourceAs<ShaderProgram>();
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ShaderProgram> MainRenderer::getSSAOBlurShader() const
{
    const std::shared_ptr<ResourceHandle>& handle = m_engine->resourceCache()->getHandleWithName("ssao_blur",
        Resource::ResourceType::kShaderProgram);
    return handle->resourceAs<ShaderProgram>();
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ShaderProgram> MainRenderer::getClusterGridShader() const
{
    const std::shared_ptr<ResourceHandle>& handle = m_engine->resourceCache()->getHandleWithName("light_cluster_grid",
        Resource::ResourceType::kShaderProgram);
    return handle->resourceAs<ShaderProgram>();
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ShaderProgram> MainRenderer::getLightCullingShader() const
{
    const std::shared_ptr<ResourceHandle>& handle = m_engine->resourceCache()->getHandleWithName("light_culling",
        Resource::ResourceType::kShaderProgram);
    return handle->resourceAs<ShaderProgram>();
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ShaderProgram> MainRenderer::getActiveClusterShader() const
{
    const std::shared_ptr<ResourceHandle>& handle = m_engine->resourceCache()->getHandleWithName("active_clusters",
        Resource::ResourceType::kShaderProgram);
    return handle->resourceAs<ShaderProgram>();
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ShaderProgram> MainRenderer::getActiveClusterCompressShader() const
{
    const std::shared_ptr<ResourceHandle>& handle = m_engine->resourceCache()->getHandleWithName("active_clusters_compact",
        Resource::ResourceType::kShaderProgram);
    return handle->resourceAs<ShaderProgram>();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MainRenderer::initialize()
{
    initializeGlobalSettings();

    // Set up render settings
    RenderSettings::CacheSettings(m_renderContext);

    m_timer.start();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MainRenderer::requestResize()
{
    QMutexLocker lock(&m_drawMutex);
    m_renderState.m_actionFlags.setFlag(RenderFrameState::kResize, true);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MainRenderer::render()
{
    QMutexLocker lock(&m_drawMutex);

    // Don't render if screen size is zero
    if (widget()->width() == 0 || widget()->height() == 0) {
        return;
    }

    // Reset between each render pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Set play mode
    SimulationLoop::PlayMode mode = m_engine->simulationLoop()->getPlayMode();
    m_renderState.m_playMode = (int)mode;

    // Flush write data for any buffer queues
    m_renderContext.flushBuffers();

    // Swap read/write buffers
    m_renderContext.swapBuffers();

    // --------------------------------------------------------------
    // Multi-sampled depth pre-pass (can be toggled on and off)
    // TODO: Shorter far plane, could make more performant, and it's not
    // Worth drawing farther objects anyway
    // --------------------------------------------------------------
    if (m_lightingFlags.testFlag(kDepthPrePass)) {
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
            m_renderState.m_camera->releaseFrame();
            m_renderState.m_camera = nullptr;
        }
    
        //if (m_renderCommands.size()) {
        //    auto& framebuffer = static_cast<SceneCamera*>(std::static_pointer_cast<DrawCommand>(m_renderCommands.front())->camera())->frameBuffer();
        //    //framebuffer.saveColorToFile(0, "C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\bufferColor.jpg");
        //    framebuffer.saveColorToFile(1, "C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\bufferNormal.jpg");
        //}
    }

    // --------------------------------------------------------------
    // Dynamic Lighting (Shadow mapping) Pass
    // --------------------------------------------------------------
    if (m_lightingFlags.testFlag(kDynamicShadows)) {
        // REMOVED: Shadow maps are being cleared on camera switch
        // Clear shadow maps (they aren't cleared on camera switch)
        //tsl::robin_map<Uuid, std::shared_ptr<Scene>>& scenes = m_engine->scenario()->scenes();
        //std::vector<ShadowMap*>& shadowMaps = m_renderContext.lightingSettings().shadowMaps();
        //size_t numShadowMaps = shadowMaps.size();
        //for (size_t i = 0; i < numShadowMaps; i++) {
        //    ShadowMap* map = shadowMaps[i];
        //    map->camera()->bindFrame(true);
        //    map->camera()->releaseFrame();
        //}

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
            m_renderState.m_camera->releaseFrame();
            m_renderState.m_camera = nullptr;
        }

        // Return to standard face culling (which is no culling, can cull individual models)
        m_renderContext.flushSetting<CullFaceSetting>(false, CulledFace::kBack);
    }

    //if (m_shadowMapCommands.size()) {
    //    auto& framebuffer = m_shadowMapCommands.front()->camera()->frameBuffer();
    //    framebuffer.saveDepthToFile("C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\depth.jpg");
    //}

    // --------------------------------------------------------------
    // Clustered Lighting pass
    // --------------------------------------------------------------
    // Determined for each camera from render commands

    // Perform light culling on cameras
    if (lightingFlags().testFlag(MainRenderer::kClustered)) {

        if (mode == SimulationLoop::kDebug) {
            // Perform lighting for only debug camera
            SceneCamera& debugCamera = m_engine->debugManager()->camera()->camera();

            // Cull lights by assigning them to each cluster according to their position
            debugCamera.lightClusterGrid().cullLights(*this);
        }
        else {
            std::vector<std::shared_ptr<Scene>>& scenes = m_engine->scenario()->scenes();
            for (const std::shared_ptr<Scene>& scene : scenes) {
                size_t count = 0;
                for (CameraComponent* camComp : scene->cameras()) {
                    SceneCamera& cam = camComp->camera();

                    // Cull lights by assigning them to each cluster according to their position
                    cam.lightClusterGrid().cullLights(*this);

                    count++;
                }
            }
        }

    }

    // --------------------------------------------------------------
    // SSAO pass (rendering SSAO texture)
    // --------------------------------------------------------------
    if (m_lightingFlags.testFlag(kSSAO)) 
    {
        // Don't write to depth buffer
        m_renderContext.flushSetting<DepthSetting>(
            true,
            DepthPassMode::kAlways,
            false);

        // Turn color rendering back on
        glColorMask(1, 1, 1, 1); 

        if (mode == SimulationLoop::kStandard) {
            for (const std::shared_ptr<Scene>& scene : m_engine->scenario()->scenes()) {
                for (const auto& cameraComp : scene->cameras())
                {
                    ssaoPass(cameraComp->camera());
                }
            }
        }
        else {
            SceneCamera& debugCamera = m_engine->debugManager()->camera()->camera();
            if (debugCamera.frameBuffer().isComplete()) {
                ssaoPass(debugCamera);
            }
        }

        //if (m_renderCommands.size()) {
        //    auto& framebuffer = static_cast<SceneCamera*>(std::static_pointer_cast<DrawCommand>(m_renderCommands.front())->camera())->frameBuffer();
        //    //framebuffer.saveColorToFile(0, "C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\bufferColor.jpg");
        //    framebuffer.saveColorToFile(1, "C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\bufferNormal.jpg");
        //}
    }

    // --------------------------------------------------------------
    // Scene rendering
    // --------------------------------------------------------------
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
    size_t numCommands = m_renderCommands.size();
    for (size_t i = 0; i < numCommands; i++) {
        m_renderCommands[i]->perform(*this);
    }

    // Unbind most recently used camera's framebuffer
    if (m_renderState.m_camera) {
        m_renderState.m_camera->releaseFrame();
        m_renderState.m_camera = nullptr;
    }

    // Draw debug visuals if in debug mode
    if (mode == SimulationLoop::kDebug) {
        SceneCamera& debugCamera = m_engine->debugManager()->camera()->camera();
        debugCamera.frameBuffer().bind();
        for (const std::shared_ptr<Scene>& scene : m_engine->scenario()->scenes()) {
            m_engine->debugManager()->draw(scene);
        }
        debugCamera.frameBuffer().release();

        // Save scene to file
        //debugCamera.frameBuffer().saveColorToFile(0, "C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\buffer.jpg");
        //debugCamera.frameBuffer().saveDepthToFile("C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\depth.jpg");
    }

    // --------------------------------------------------------------
    // Quad rendering and Post-Processing
    // --------------------------------------------------------------
    glViewport(0, 0, widget()->width(), widget()->height());

    m_renderContext.flushSetting<DepthSetting>(
        true,
        DepthPassMode::kLessEqual,
        true);

    if (mode == SimulationLoop::kStandard) {
        for (const std::shared_ptr<Scene>& scene : m_engine->scenario()->scenes()) {
            for (const auto& cameraComp : scene->cameras()) 
            {
                cameraComp->camera().drawFrameBufferQuad(m_engine);
                //cameraComp->camera().frameBuffer().saveColorToFile(0, "C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\main.jpg");
            }
        }
    }
    else {
        SceneCamera& debugCamera = m_engine->debugManager()->camera()->camera();
        if (debugCamera.frameBuffer().isComplete()) {
            m_engine->debugManager()->camera()->camera().drawFrameBufferQuad(m_engine);
            //auto quadShaderProgram = m_engine->resourceCache()->getHandleWithName("quad",
                //Resource::kShaderProgram)->resourceAs<ShaderProgram>();
            //debugCamera.ssaoBlurFrameBuffer().drawQuad(debugCamera, *quadShaderProgram);
        }
    }

    // --------------------------------------------------------------
    // Clear render state for the next frame
    // --------------------------------------------------------------
    m_renderState = RenderFrameState();
    m_renderState.m_previousPlayMode = (int)mode;
    m_renderContext.setBoundMaterial(-1);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MainRenderer::processScenes()
{

    // Get scenes
    std::vector<std::shared_ptr<Scene >> &scenes = m_engine->scenario()->scenes();
    //if (!scenes.size()) {
    //    // If no scenes, just render static geometry
    //    m_engine->debugManager()->scene()->createDrawCommands(*this);
    //}

    // Iterate through scenes
    // TODO: Add notion of active scenes
    for(const std::shared_ptr<Scene>& scene: scenes){
        scene->createDrawCommands(*this);
    }

    // Grab debug visuals to render
    SimulationLoop::PlayMode mode = m_engine->simulationLoop()->getPlayMode();
    if (mode == SimulationLoop::kDebug) {
        for (const std::shared_ptr<Scene>& scene : scenes) {
            m_engine->debugManager()->createDrawCommands(*scene, *this);
        }
    }

    // Pre-processing before queuing for render
    preprocessCommands();

    // Lock draw mutex for swap of command vectors
    QMutexLocker lock(&m_drawMutex);

    // Queue up commands to render scene
    m_renderCommands.swap(m_receivedCommands);
    m_receivedCommands.clear();

    // Queue up commands to render shadow maps
    m_shadowMapCommands.swap(m_receivedShadowMapCommands);
    m_receivedShadowMapCommands.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void MainRenderer::addRenderCommand(const std::shared_ptr<RenderCommand>& command)
{
    //QMutexLocker lock(&m_drawMutex);
    command->onAddToQueue();
    m_receivedCommands.push_back(command);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MainRenderer::addShadowMapCommand(const std::shared_ptr<DrawCommand>& command)
{
    //QMutexLocker lock(&m_drawMutex);
    command->onAddToQueue();
    m_receivedShadowMapCommands.push_back(command);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MainRenderer::ssaoPass(SceneCamera& camera)
{
    if (!m_renderContext.isCurrent()) {
        throw("Error, context should be current");
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
    camera.frameBuffer().bindDepthTexture(0);
    //glActiveTexture(GL_TEXTURE0 + 0);
    //size_t depthTex = Texture::TextureBoundToActiveUnit();
    //size_t correctDepth = camera.frameBuffer().blitBuffer()->depthTexture()->getID();

    camera.frameBuffer().bindColorTexture(1, 1);
    //glActiveTexture(GL_TEXTURE0 + 1);
    //size_t normalTex = Texture::TextureBoundToActiveUnit();
    //size_t correctNormal = camera.frameBuffer().blitBuffer()->colorTextures()[1]->getID();

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
    static Color clearColor = Color(Vector4(0.95f, 0.3f, 0.23f, 1.0f));    // blue violet
    ssaoFrameBuffer.clear(clearColor);

    // Bind SSAO shader
    std::shared_ptr<ShaderProgram> ssaoShader = getSSAOShader();
    ssaoShader->bind();

    // Bind kernel buffer
    ls.ssaoBuffer().bindToPoint();

    // Set uniforms
    // TODO: Think up a more precise scaling for bias
    int noiseSize = ls.noiseSize();
    //Vector2 scale = Vector2(camera.viewport().m_width, camera.viewport().m_height);
    Vector2 scale(1.0, 1.0);
    //float bias = ls.ssaoBias();
    //float bias = 0.01;
    //float biasScale = 0.0011; // How bias scales with depth
    //float bias = 0.00;
    //float biasLinear = 0.0000; // How bias scales with depth
    //float biasSquared = 1e-6; // How bias scales with square of depth
    float bias = -0.002f;
    //float biasLinear = 0.0000; // How bias scales with depth
    //float biasSquared = 9.5e-7; // How bias scales with square of depth
    float radius = ls.ssaoRadius();
    int numKernelSamples = ls.numKernelSamples();

    camera.bindUniforms(this);
    ssaoShader->setUniformValue("noiseSize", noiseSize);
    ssaoShader->setUniformValue("offsets", Vector3(0, 0, 0));
    ssaoShader->setUniformValue("scale", scale);
    ssaoShader->setUniformValue("kernelSize", numKernelSamples);
    ssaoShader->setUniformValue("bias", bias);
    //ssaoShader->setUniformValue("biasLinear", biasLinear);
    //ssaoShader->setUniformValue("biasSquared", biasSquared);
    ssaoShader->setUniformValue("radius", radius);
    ssaoShader->updateUniforms();

    // Draw the quad
    std::shared_ptr<Mesh> quad = m_engine->resourceCache()->polygonCache()->getSquare();
    quad->vertexData().drawGeometry(GL_TRIANGLES);

    // Release kernel buffer
    m_renderContext.lightingSettings().ssaoBuffer().releaseFromPoint();

    // Release shader
    ssaoShader->release();

    // Release FBO
    ssaoFrameBuffer.release();

#ifdef DEBUG_MODE
    bool error = printGLError("Error during SSAO pass");
    if (error) {
        logInfo("Error during SSAO pass");
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
    ssaoFrameBuffer.bindColorTexture(3, 0);

    // Bind SSAO blur FBO
    FrameBuffer& ssaoBlurFrameBuffer = camera.ssaoBlurFrameBuffer();
    ssaoBlurFrameBuffer.bind();

    // Set viewport
    camera.viewport().setGLViewport(ssaoBlurFrameBuffer);

    // Clear buffer
    ssaoBlurFrameBuffer.clear(clearColor);

    // Bind shader
    std::shared_ptr<ShaderProgram> ssaoBlurShader = getSSAOBlurShader();
    ssaoBlurShader->bind();
    
    // Set uniforms
    ssaoBlurShader->setUniformValue("offsets", Vector3(0, 0, 0));
    ssaoBlurShader->setUniformValue("scale", scale);
    ssaoBlurShader->updateUniforms();

    // Draw the quad
    quad->vertexData().drawGeometry(GL_TRIANGLES);

    ssaoBlurShader->release();

    // Release FBO
    ssaoBlurFrameBuffer.release();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void MainRenderer::initializeGlobalSettings()
{
    printGLError("Error on GL startup");

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable back-face culling
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    // Set background color
    glClearColor(0.55f, 0.6f, 0.93f, 1.0f); // blue violet
    glEnable(GL_DEPTH_TEST);
    
    // Disable MSAA, since framebuffers use SMAA
    glDisable(GL_MULTISAMPLE);
    //glEnable(GL_MULTISAMPLE); // Enable MSAA

    // Clear all colors on screen from the last frame
    glClear(GL_COLOR_BUFFER_BIT);

    printGLError("Error on GL initialization");
}
///////////////////////////////////////////////////////////////////////////////////////////////
void MainRenderer::preprocessCommands()
{
    // Perform pre-processing of commands
    for (const auto& command : m_receivedCommands) {
        command->preSort();
    }
    for (const auto& command : m_receivedShadowMapCommands) {
        command->preSort();
    }
    //std::pair<float, float> depthRange = DrawCommand::DepthRange();
    //logInfo(QString(Vector2g(depthRange.first, depthRange.second)));

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
}


/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}