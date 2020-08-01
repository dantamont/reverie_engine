#include "GbMainRenderer.h"

#include "../../GbCoreEngine.h"
#include "../../loop/GbSimLoop.h"
#include "../../debugging/GbDebugManager.h"

#include "../../../view/GL/GbGLWidget.h"

#include "../geometry/GbVertexData.h"
#include "../geometry/GbBuffers.h"
#include "../geometry/GbPolygon.h"
#include "../models/GbModel.h"
#include "../shaders/GbShaders.h"

#include "GbRenderers.h"
#include "GbRenderSettings.h"
#include "../../containers/GbSortingLayer.h"

#include "../../components/GbTransformComponent.h"
#include "../../components/GbCamera.h"
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
std::shared_ptr<ShaderProgram> MainRenderer::prepassShader() const
{
    const std::shared_ptr<ResourceHandle>& handle = m_engine->resourceCache()->getHandleWithName("prepass",
        Resource::ResourceType::kShaderProgram);
    return handle->resourceAs<ShaderProgram>();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MainRenderer::initialize()
{
    initializeGlobalSettings();

    // Set up render settings
    RenderSettings::cacheSettings();

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

    // Clear framebuffers
    SimulationLoop::PlayMode mode = m_engine->simulationLoop()->getPlayMode();
    m_renderState.m_playMode = (int)mode;

    // --------------------------------------------------------------
    // Depth pre-pass (can be toggled on and off)
    // --------------------------------------------------------------
    if (m_lightingMode = kForwardDepthPass) {
        // Fill the z-buffer
        m_renderState.m_stage = RenderFrameState::kDepthPrepass;
        
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glColorMask(0, 0, 0, 0); // turn off color rendering

        // Draw commands
        for (std::shared_ptr<RenderCommand>& command : m_renderCommands) {
            command->perform(*this);
        }
    }

    // --------------------------------------------------------------
    // Scene rendering
    // --------------------------------------------------------------
    if (m_lightingMode = kForwardDepthPass) {
        // Restore settings for render pass
        m_renderState.m_camera = nullptr;
        m_renderState.m_stage = RenderFrameState::kRender;
        glDepthMask(GL_FALSE); // don't write to depth buffer
        glDepthFunc(GL_LEQUAL);
        glColorMask(1, 1, 1, 1); // turn on color rendering
    }
    m_renderState.m_stage = RenderFrameState::kRender;

    // Perform draw commands
    for (std::shared_ptr<RenderCommand>& command : m_renderCommands) {
        command->perform(*this);
    }

    // Draw debug visuals if in debug mode
    if (mode == SimulationLoop::kDebug) {
        Camera& debugCamera = m_engine->debugManager()->camera()->camera();
        debugCamera.frameBuffer().bind();
        for (const auto& scenePair : m_engine->scenario()->scenes()) {
            m_engine->debugManager()->draw(scenePair.second);
        }
        debugCamera.frameBuffer().release();
    }

    // Perform second stage draw (render quads to screen)
    glViewport(0, 0, widget()->width(), widget()->height());
    if (mode == SimulationLoop::kStandard) {
        for (const auto& scenePair : m_engine->scenario()->scenes()) {
            for (const auto& cameraComp : scenePair.second->cameras()) {
                cameraComp->camera().drawFrameBufferQuad(*this);
            }
        }
    }
    else {
        Camera& debugCamera = m_engine->debugManager()->camera()->camera();
        if (debugCamera.frameBuffer().isComplete()) {
            m_engine->debugManager()->camera()->camera().drawFrameBufferQuad(*this);
        }
    }

    // Clear render state for next frame
    m_renderState = RenderFrameState();
    m_renderState.m_previousPlayMode = (int)mode;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MainRenderer::processScenes()
{
    // Get scenes
    std::unordered_map<Uuid, std::shared_ptr<Scene>>& scenes = m_engine->scenario()->scenes();
    //if (!scenes.size()) {
    //    // If no scenes, just render static geometry
    //    m_engine->debugManager()->scene()->createDrawCommands(*this);
    //}

    // Iterate through scenes
    for (std::pair<const Uuid, std::shared_ptr<Scene>>& scenePair : scenes) {
        std::shared_ptr<Scene>& scene = scenePair.second;
        scene->createDrawCommands(*this);
    }

    // Grab debug visuals to render
    SimulationLoop::PlayMode mode = m_engine->simulationLoop()->getPlayMode();
    if (mode == SimulationLoop::kDebug) {
        for (const auto& scenePair : m_engine->scenario()->scenes()) {
            m_engine->debugManager()->createDrawCommands(*scenePair.second, *this);
        }
    }

    // Pre-processing before queuing for render
    preprocessCommands();

    QMutexLocker lock(&m_drawMutex);
    m_renderCommands.swap(m_receivedCommands);
    m_receivedCommands.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void MainRenderer::addRenderCommand(const std::shared_ptr<RenderCommand>& command)
{
    command->onAddToQueue();
    m_receivedCommands.push_back(command);
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
    DrawCommand::resetDepths();

    // Sort commands
    std::sort(m_receivedCommands.begin(),
        m_receivedCommands.end(),
        [](const std::shared_ptr<RenderCommand>& c1, std::shared_ptr<RenderCommand>& c2) {
        return c1->sortKey() < c2->sortKey();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////
//void MainRenderer::clearModels()
//{
//    // TODO: Clean things up so this isn't necessary
//    for (auto& scenePair : m_engine->scenario()->scenes()) {
//        std::shared_ptr<Scene>& scene = scenePair.second;
//        const auto& topObjects = scene->topLevelSceneObjects();
//        for (const auto& sceneObject : topObjects) {
//
//            // Clear all model components so no shared pointers to models remain
//            sceneObject->clearModels();
//
//        }
//    }
//
//    m_engine->resourceCache()->clearModels();
//}

/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}