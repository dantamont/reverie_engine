
#include "core/components/GCameraComponent.h"
#include "core/GCoreEngine.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/components/GTransformComponent.h"
#include "core/components/GShaderComponent.h"
#include "core/components/GCanvasComponent.h"
#include "core/components/GCubeMapComponent.h"

#include "core/resource/GResourceCache.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/buffers/GUniformBufferObject.h"
#include "core/rendering/materials/GMaterial.h"
#include "core/rendering/postprocessing/GPostProcessingChain.h"
#include "core/rendering/postprocessing/GPostProcessingEffect.h"

#include "core/loop/GSimLoop.h"
#include "core/debugging/GDebugManager.h"

#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/renderer/GRenderCommand.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "fortress/layer/framework/GFlags.h"

#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "core/layer/view/widgets/graphics/GInputHandler.h"

#include "enums/GSimulationPlayModeEnum.h"

namespace rev {


void to_json(nlohmann::json& orJson, const ControllerProfile& korObject)
{
    orJson["movementTypes"] = json::array();
    for (const std::pair<int, bool>& movePair : korObject.m_movementTypes) {
        orJson["movementTypes"].push_back(movePair.first);
    }

    //orJson["target"] = korObject.m_target.asJson();
    orJson["zoomScaling"] = korObject.m_zoomScaling;
    orJson["translateScaling"] = korObject.m_translateScaling;
    orJson["rotateScaling"] = korObject.m_rotateScaling;
    orJson["panScaling"] = korObject.m_panScaling;
    orJson["tiltScaling"] = korObject.m_tiltScaling;
}

void from_json(const nlohmann::json& korJson, ControllerProfile& orObject)
{
    const json& movementTypes = korJson.at("movementTypes");
    for (const auto& moveTypeJson : movementTypes) {
        Int32_t moveType = moveTypeJson.get<Int32_t>();
        orObject.m_movementTypes[moveType] = true;
    }

    //m_target = Vector3g(korJson.at("target"));
    korJson.at("zoomScaling").get_to(orObject.m_zoomScaling);
    korJson.at("translateScaling").get_to(orObject.m_translateScaling);
    korJson.at("rotateScaling").get_to(orObject.m_rotateScaling);
    korJson.at("panScaling").get_to(orObject.m_panScaling);
    if (korJson.contains("tiltScaling")) {
        korJson.at("tiltScaling").get_to(orObject.m_tiltScaling);
    }
}



CameraController::CameraController(CameraComponent * camera):
    m_component(camera)
{
}

CameraController::~CameraController()
{
}

void CameraController::step(double deltaSec)
{
    step(deltaSec, m_profile);
}

void CameraController::step(double deltaSec, const ControllerProfile & profile)
{
    G_UNUSED(deltaSec);
    const Vector2& mouseDelta = inputHandler().mouseHandler().mouseDelta();
    const Vector2& scrollDelta = inputHandler().mouseHandler().scrollDelta();

    // Return if no movement
    bool mouseMoved = inputHandler().mouseHandler().wasMoved();
    bool scrolled = inputHandler().mouseHandler().wasScrolled();

    // Check that camera is not located at target position
    SceneCamera& cam = camera();
    if (cam.eye() == m_target) {
        // Relocate target if camera is on top of it
#ifdef DEBUG_MODE
        Logger::LogWarning("Warning, camera is on top of target, will not render correctly");
#endif
    }

    for (const std::pair<int, bool>& movePair : profile.m_movementTypes) {
        if (!movePair.second) return;
        switch (movePair.first) {
        case kTranslate:
        {
            bool shiftPressed = inputHandler().keyHandler().isHeld(Qt::Key::Key_Shift);
            if (inputHandler().keyHandler().isHeld(Qt::Key::Key_Up)) {
                if (shiftPressed) {
                    cam.translate(m_target, Vector3(0.0f, 0.0f, 1.0f) * profile.m_translateScaling);
                }
                else {
                    cam.translate(m_target, Vector3(0.0f, 1.0f, 0.0f) * profile.m_translateScaling);
                }
            }
            if (inputHandler().keyHandler().isHeld(Qt::Key::Key_Down)) {
                if (shiftPressed) {
                    cam.translate(m_target, Vector3(0.0f, 0.0f, -1.0f) * profile.m_translateScaling);
                }
                else {
                    cam.translate(m_target, Vector3(0.0f, -1.0f, 0.0f) * profile.m_translateScaling);
                }
            }
            if (inputHandler().keyHandler().isHeld(Qt::Key::Key_Left)) {
                cam.translate(m_target, Vector2(-1.0f, 0.0f) * profile.m_translateScaling);
            }
            if (inputHandler().keyHandler().isHeld(Qt::Key::Key_Right)) {
                cam.translate(m_target, Vector2(1.0f, 0.0f) * profile.m_translateScaling);
            }
            break;
        }
        case kTilt:
            if (mouseMoved) {
                if (inputHandler().mouseHandler().isHeld(Qt::MouseButton::MiddleButton)) {
                    cam.tilt(m_target, mouseDelta * profile.m_tiltScaling);
                }
            }
            break;
        case kPan:
            if (mouseMoved) {
                if (inputHandler().mouseHandler().isHeld(Qt::MouseButton::RightButton)) {
                    cam.pan(m_target, mouseDelta * profile.m_panScaling);
                }
            }
            break;
        case kZoom:
            if (scrolled) {
                cam.zoom(m_target, scrollDelta.y() * profile.m_zoomScaling);
            }
            break;
        case kRotate:
            if (mouseMoved) {
                if (inputHandler().mouseHandler().isHeld(Qt::MouseButton::LeftButton)) {
                    cam.rotateAboutPoint(m_target, mouseDelta * profile.m_rotateScaling);
                }
            }
            break;
        }
    }
}

void to_json(json& orJson, const CameraController& korObject)
{
    orJson = { 
        {"profile" ,korObject.m_profile}, 
        {"target", korObject.m_target}
    };
}

void from_json(const json& korJson, CameraController& orObject)
{
    korJson.at("profile").get_to(orObject.m_profile);
    korJson.at("target").get_to(orObject.m_target);
}

InputHandler& CameraController::inputHandler() const
{
    return static_cast<InputHandler&>(m_component->sceneObject()->scene()->engine()->widgetManager()->mainGLWidget()->inputHandler());
}

SceneCamera & CameraController::camera()
{
    return m_component->camera();
}



CameraComponent::CameraComponent() :
    Component(ComponentType::kCamera),
    m_camera(this),
    m_controller(this)
{
}

CameraComponent::CameraComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kCamera),
    m_camera(this),
    m_controller(this)
{
    // Set camera on scene object and update camera's view matrix
    object->setComponent(this);
    object->transform().computeWorldMatrix();

    // Add camera to scene
    object->scene()->addCamera(this);

    // In case camera is initialized after window has already been resized
    // Removed 2/19/21, I don't *think* it's necessary anymore
    // Added back in on 3/12/2021, this seems to keep things happy? Without this,
    // The framebuffer for a second camera is set to 1,1 on scenario switch
    // Note: Must scale dimensions by DPR for true pixel size
    GLWidget* widget = sceneObject()->scene()->engine()->openGlRenderer()->widget();
    m_camera.resizeFrame(widget->pixelWidth(), widget->pixelHeight());
    m_camera.lightClusterGrid().onResize();
}

CameraComponent::~CameraComponent()
{
    // Remove camera from scene
    if (!scene()) {
        Logger::LogError("Error, no scene for camera component");
    }
    scene()->removeCamera(this);
}

void CameraComponent::retrieveDrawCommands(Scene& scene, OpenGlRenderer& renderer, const GSimulationPlayMode& playMode)
{    
    // Skip rendering for camera if disabled
    if (!isEnabled()) return;

    std::vector<SortingLayer> localLayers = getLocalSortingLayers(playMode);
    const bool isDebugMode = ESimulationPlayMode::eDebug == playMode;

    for (const SortingLayer& currentLayer : localLayers) {
        // Iterate through scene objects
        auto& topLevelObjects = scene.topLevelSceneObjects();
        for (const std::shared_ptr<SceneObject>& sceneObject : topLevelObjects) {
            if (sceneObject->isVisible()) {
                sceneObject->retrieveDrawCommands(m_camera, renderer, currentLayer, isDebugMode);
            }
        }

        // Render skybox if there is one
        // Rendered after everything else to discard unnecessary vertices
        createSkyboxDrawCommand(scene, renderer, currentLayer);
    }
}

void CameraComponent::enable()
{
    Component::enable();
}

void CameraComponent::disable()
{
    Component::disable();
}

std::vector<SortingLayer> CameraComponent::getLocalSortingLayers(const GSimulationPlayMode& playMode) const
{
    std::vector<SortingLayer> localLayers;
    const bool isDebugMode = ESimulationPlayMode::eDebug == playMode;
    if (isDebugMode) {
        localLayers = { *sceneObject()->scene()->engine()->debugManager()->debugRenderLayer()};
    }
    else
    {
        if (m_camera.cameraOptions().testFlag(CameraOption::kShowAllRenderLayers)) {
            // Override layer settings and render everything
            if (sceneObject()->scene()->scenario()) {
                // FIXME: Check since debug camera doesn't like this
                std::vector<SortingLayer> rLayers;

                for (const auto& layer : sceneObject()->scene()->scenario()->settings().renderLayers()) {
                    localLayers.emplace_back(layer);
                }
            }
        }
        else {
            // Render only toggled layers
            localLayers = m_camera.renderLayers();
        }
    }
    return localLayers;
}

void CameraComponent::createSkyboxDrawCommand(Scene& scene, OpenGlRenderer& renderer, const SortingLayer& currentLayer)
{
    CubeMapComponent* skybox = nullptr;
    if (!m_cubeMapID.isNull()) {
        skybox = scene.getCubeMap(m_cubeMapID);
    }
    else if (scene.defaultCubeMap()) {
        skybox = scene.defaultCubeMap();
    }

    if (skybox) {
        bool hasLayer = skybox->sceneObject()->hasRenderLayer(currentLayer.id());
        RenderContext& context = scene.engine()->openGlRenderer()->renderContext();
        UniformContainer& uc = context.uniformContainer();
        if (skybox->isEnabled() && hasLayer) {
            // Create skybox draw command
            auto cubemapShader = ResourceCache::Instance().getHandleWithName("cubemap",
                EResourceType::eShaderProgram)->resourceAs<ShaderProgram>();

            auto drawCommand = std::make_shared<DrawCommand>(
                *skybox, *cubemapShader, uc, m_camera, sceneObject()->id());
            drawCommand->addUniform(
                skybox->sceneObject()->transform().worldMatrixUniform(),
                cubemapShader->uniformMappings().m_worldMatrix,
                -1
            );
            drawCommand->setRenderLayer(currentLayer);
            drawCommand->setRenderableWorldBounds(skybox->mesh()->objectBounds()); // FIXME: Set actual world bounds
            renderer.addRenderCommand(drawCommand);
        }
    }
}

void to_json(json& orJson, const CameraComponent& korObject)
{
    ToJson<Component>(orJson, korObject);

    orJson["camera"] = korObject.m_camera;
    orJson["controller"] = korObject.m_controller;

    if (!korObject.m_cubeMapID.isNull()) {
        orJson["cubeMap"] = korObject.m_cubeMapID;
    }
}

void from_json(const json& korJson, CameraComponent& orObject)
{
    from_json(korJson, static_cast<Component&>(orObject));

    korJson.at("camera").get_to(orObject.m_camera);
    korJson.at("controller").get_to(orObject.m_controller);
    if (korJson.contains("cubeMap")) {
        // Get cubemap for this camera based on UUID
        orObject.m_cubeMapID = korJson.at("cubeMap");
    }
}




    
// End namespaces
}
