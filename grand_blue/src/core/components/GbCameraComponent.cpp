#include "GbCameraComponent.h"
#include "../GbCoreEngine.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"
#include "../scene/GbScene.h"
#include "../components/GbTransformComponent.h"
#include "../components/GbShaderComponent.h"
#include "../components/GbCanvasComponent.h"
#include "../components/GbCubeMapComponent.h"

#include "../resource/GbResourceCache.h"
#include "../rendering/models/GbModel.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/buffers/GbUniformBufferObject.h"
#include "../rendering/materials/GbMaterial.h"
#include "../rendering/postprocessing/GbPostProcessingChain.h"
#include "../rendering/postprocessing/GbPostProcessingEffect.h"

#include "../loop/GbSimLoop.h"
#include "../debugging/GbDebugManager.h"

#include "../rendering/renderer/GbMainRenderer.h"
#include "../rendering/renderer/GbRenderCommand.h"

#include "../../view/GbWidgetManager.h"
#include "../../view/GL/GbGLWidget.h"
#include "../containers/GbFlags.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////
// CameraController
//////////////////////////////////////////////////////////////////////////////////////////////////
CameraController::CameraController(CameraComponent * camera):
    m_component(camera)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CameraController::~CameraController()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CameraController::step(unsigned long deltaMs)
{
    step(deltaMs, m_profile);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CameraController::step(unsigned long deltaMs, const ControllerProfile & profile)
{
    Q_UNUSED(deltaMs);
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
        logWarning("Warning, camera is on top of target, will not render correctly");
#endif
    }

    for (const std::pair<MovementType, bool>& movePair : profile.m_movementTypes) {
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
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue CameraController::asJson() const
{
    QJsonObject object;
    object.insert("profile", m_profile.asJson());
    object.insert("target", m_target.asJson());
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CameraController::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    QJsonObject object = json.toObject();
    m_profile.loadFromJson(object.value("profile"));
    m_target.loadFromJson(object.value("target"));
}
/////////////////////////////////////////////////////////////////////////////////////////////
InputHandler& CameraController::inputHandler() const
{
    return m_component->sceneObject()->engine()->widgetManager()->mainGLWidget()->inputHandler();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
SceneCamera & CameraController::camera()
{
    return m_component->camera();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue CameraController::ControllerProfile::asJson() const
{
    QJsonObject object;
    QJsonArray movementTypes;
    for (const std::pair<MovementType, bool>& movePair : m_movementTypes) {
        movementTypes.append((int)movePair.first);
    }
    object.insert("movementTypes", movementTypes);

    //object.insert("target", m_target.asJson());
    object.insert("zoomScaling", m_zoomScaling);
    object.insert("translateScaling", m_translateScaling.asJson());
    object.insert("rotateScaling", m_rotateScaling.asJson());
    object.insert("panScaling", m_panScaling.asJson());
    object.insert("tiltScaling", m_tiltScaling.asJson());

    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CameraController::ControllerProfile::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    QJsonObject object = json.toObject();

    QJsonArray movementTypes = object.value("movementTypes").toArray();
    for (auto moveTypeJson : movementTypes) {
        MovementType moveType = (MovementType)moveTypeJson.toInt();
        m_movementTypes[moveType] = true;
    }

    //m_target = Vector3g(object.value("target"));
    m_zoomScaling = real_g(object.value("zoomScaling").toDouble());
    m_translateScaling = Vector3(object.value("translateScaling"));
    m_rotateScaling = Vector2(object.value("rotateScaling"));
    m_panScaling = Vector2(object.value("panScaling"));
    if(object.contains("tiltScaling"))
        m_tiltScaling = Vector2(object.value("tiltScaling"));
}




//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
CameraComponent::CameraComponent() :
    Component(ComponentType::kCamera),
    m_camera(this),
    m_controller(this)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CameraComponent::CameraComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kCamera),
    m_camera(this),
    m_controller(this)
{
    object->setCamera(this);

    // Add camera to scene
    object->scene()->addCamera(this);

    // In case camera is initialized after window has already been resized
    View::GLWidget* widget = m_engine->mainRenderer()->widget();
    m_camera.resizeFrame(widget->width(), widget->height());
    m_camera.lightClusterGrid().onResize();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CameraComponent::~CameraComponent()
{
    // Remove camera from scene;
    if (sceneObject()) {
        std::shared_ptr<Scene> scene = sceneObject()->scene();
        if (scene) {
            scene->removeCamera(this);
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CameraComponent::createDrawCommands(Scene& scene, MainRenderer& renderer)
{    
    // Skip rendering for camera if disabled
    if (!m_isEnabled) return;

    std::vector<std::shared_ptr<SortingLayer>> localLayers;
    std::vector<std::shared_ptr<SortingLayer>>* renderLayers;
    if (m_camera.cameraOptions().testFlag(CameraOption::kShowAllRenderLayers)) {
        // Override layer settings and render everything
        if (sceneObject()->scene()->scenario()) {
            // FIXME: Check since debug camera doesn't like this
            renderLayers = &sceneObject()->scene()->scenario()->settings().renderLayers();
        }
    }
    else {
        // Render only toggled layers
        localLayers = m_camera.renderLayers();
        renderLayers = &localLayers;
    }

    for (const auto& currentLayer : *renderLayers) {
        // Iterate through scene objects
        auto& topLevelObjects = scene.topLevelSceneObjects();
        for (const std::shared_ptr<SceneObject>& sceneObject : topLevelObjects) {
            //sceneObject->draw(SceneObject::kModel);
            sceneObject->createDrawCommands(m_camera, renderer, *currentLayer);
        }

        // Render skybox if there is one
        // Rendered after everything else to discard unnecessary vertices
        CubeMapComponent* skybox = nullptr;
        if (!m_cubeMapID.isNull()) {
            skybox = scene.getCubeMap(m_cubeMapID);
        }
        else if (scene.defaultCubeMap()) {
            skybox = scene.defaultCubeMap();
        }

        if (skybox) {
            bool hasLayer = skybox->sceneObject()->hasRenderLayer(currentLayer->getName());

            if (skybox->isEnabled() && hasLayer) {
                // Create skybox draw command
                auto cubemapShader = m_engine->resourceCache()->getHandleWithName("cubemap",
                    Resource::kShaderProgram)->resourceAs<ShaderProgram>();
                //cubemapShader->bind();
                //skybox->draw(cubemapShader);

                auto drawCommand = std::make_shared<DrawCommand>(
                    *skybox, *cubemapShader, m_camera);
                drawCommand->setUniform(Uniform("worldMatrix", skybox->sceneObject()->transform()->worldMatrix()));
                drawCommand->setRenderLayer(currentLayer.get());
                renderer.addRenderCommand(drawCommand);
            }
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CameraComponent::createDebugDrawCommands(Scene & scene, MainRenderer & renderer)
{
    // Skip rendering for camera if disabled
    if (!m_isEnabled) return;

    // Iterate through scene objects
    std::shared_ptr<SortingLayer> debugLayer = m_engine->debugManager()->debugRenderLayer();
    auto& topLevelObjects = scene.topLevelSceneObjects();
    for (const std::shared_ptr<SceneObject>& sceneObject : topLevelObjects) {
        //sceneObject->draw(SceneObject::kModel);
        sceneObject->createDrawCommands(m_camera, 
            renderer,
            *debugLayer,
            true);
    }

    // Render skybox if there is one
    // Rendered after everything else to discard unnecessary vertices
    CubeMapComponent* skybox = nullptr;
    if (!m_cubeMapID.isNull()) {
        skybox = scene.getCubeMap(m_cubeMapID);
        if (!skybox) {
            // Skybox not found in scene, check if debug skybox
            skybox = m_engine->debugManager()->scene()->getCubeMap(m_cubeMapID);
        }
    }
    else if (scene.defaultCubeMap()) {
        skybox = scene.defaultCubeMap();
    }

    if (skybox) {
        bool hasLayer = skybox->sceneObject()->hasRenderLayer(debugLayer->getName());

        if (skybox->isEnabled() && hasLayer) {
            // Create skybox draw command
            auto cubemapShader = m_engine->resourceCache()->getHandleWithName("cubemap",
                Resource::kShaderProgram)->resourceAs<ShaderProgram>();
            //cubemapShader->bind();
            //skybox->draw(cubemapShader);

            auto drawCommand = std::make_shared<DrawCommand>(
                *skybox, *cubemapShader, m_camera);
            drawCommand->setUniform(Uniform("worldMatrix", skybox->sceneObject()->transform()->worldMatrix()));
            drawCommand->setRenderLayer(debugLayer.get());
            renderer.addRenderCommand(drawCommand);
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue CameraComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    object.insert("camera", m_camera.asJson());
    object.insert("controller", m_controller.asJson());
    if (!m_cubeMapID.isNull()) {
        object.insert("cubeMap", m_cubeMapID.asString().c_str());
    }
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CameraComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    Component::loadFromJson(json);
    QJsonObject object = json.toObject();

    m_camera.loadFromJson(object.value("camera"));
    m_controller.loadFromJson(object.value("controller"));
    if (object.contains("cubeMap")) {
        // Get cubemap for this camera based on UUID
        m_cubeMapID = Uuid(object["cubeMap"].toString());
    }
}




//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}
