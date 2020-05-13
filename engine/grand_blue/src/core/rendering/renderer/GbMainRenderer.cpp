#include "GbMainRenderer.h"

#include "../../GbCoreEngine.h"

#include "../../../view/GL/GbGLWidget.h"

#include "../geometry/GbVertexData.h"
#include "../geometry/GbBuffers.h"
#include "../geometry/GbPolygon.h"
#include "../models/GbModel.h"
#include "../shaders/GbShaders.h"
#include "../materials/GbMaterial.h"
#include "../materials/GbCubeMap.h"

#include "GbRenderers.h"
#include "GbRenderSettings.h"
#include "../../containers/GbSortingLayer.h"

#include "../../components/GbTransformComponent.h"
#include "../../components/GbCamera.h"
#include "../../components/GbLight.h"
#include "../../components/GbScriptComponent.h"
#include "../../components/GbRendererComponent.h"

#include "../../scene/GbScenario.h"
#include "../../scene/GbScene.h"
#include "../../scene/GbSceneObject.h"
#include "../../geometry/GbTransformComponents.h"
#include "../../geometry/GbEulerAngles.h"

#include "../../resource/GbResourceCache.h"
#include "../../readers/models/GbOBJReader.h"

#include "../../processes/GbProcessManager.h"

#include "../../loop/GbSimLoop.h"
#include "../../debugging/GbDebugManager.h"

namespace Gb {   
namespace GL {

/////////////////////////////////////////////////////////////////////////////////////////////
MainRenderer::MainRenderer(Gb::CoreEngine* engine, View::GLWidget* widget):
    m_engine(engine),
    m_widget(widget)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
MainRenderer::~MainRenderer()
{
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
void MainRenderer::render()
{
    // Reset between each render pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

    // TODO: Rethink this loop, maybe abstract out the idea of a RenderCommand, which
    // Would set and restore the render state for draw calls
    SimulationLoop::PlayMode mode = m_engine->simulationLoop()->getPlayMode();

    // Iterate through scenes
    std::unordered_map<Uuid, std::shared_ptr<Scene>>& scenes = m_engine->scenario()->scenes();
    for (std::pair<const Uuid, std::shared_ptr<Scene>>& scenePair : scenes) {
        std::shared_ptr<Scene>& scene = scenePair.second;

        // Iterate through cameras
        if (mode == SimulationLoop::kStandard) {
            for (CameraComponent*& camera : scene->cameras()) {
                camera->drawScene(scene, *this);
            }
        }
        else if (mode == SimulationLoop::kDebug) {
            // Debug mode is enabled
            m_engine->debugManager()->camera()->drawScene(scene, *this);
        }

    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MainRenderer::clear()
{
    m_engine->scenario()->clearSceneObjects();
    clearModels();
    m_engine->scenario()->removeScenes();
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

    // Set background color
    //glEnable(GL_LIGHTING); // legacy GL, incompatible with current profile
    //glEnable(GL_LIGHT0); // legacy GL, incompatible with current profile
    glClearColor(0, 0, 1, 1);
    glEnable(GL_DEPTH_TEST);

    // Clear all colors on screen from the last frame
    glClear(GL_COLOR_BUFFER_BIT);

    printGLError("Error on GL initialization");
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MainRenderer::clearModels()
{
    // TODO: Clean things up so this isn't necessary
    for (auto& scenePair : m_engine->scenario()->scenes()) {
        std::shared_ptr<Scene>& scene = scenePair.second;
        const auto& topObjects = scene->topLevelSceneObjects();
        for (const auto& sceneObject : topObjects) {
            if (!Map::HasKey(sceneObject->components(), Component::kModel)) break;

            // Clear all model components so no shared pointers to models remain
            sceneObject->clearModels();

        }
    }

    m_engine->resourceCache()->clearModels();
}

/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
}