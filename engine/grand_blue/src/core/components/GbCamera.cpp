#include "GbCamera.h"
#include "../GbCoreEngine.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"
#include "../scene/GbScene.h"
#include "../components/GbTransformComponent.h"
#include "../components/GbRendererComponent.h"
#include "../components/GbCanvasComponent.h"

#include "../resource/GbResourceCache.h"
#include "../rendering/models/GbModel.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/shaders/GbUniformBufferObject.h"
#include "../rendering/materials/GbMaterial.h"
#include "../rendering/materials/GbCubeMap.h"

#include "../loop/GbSimLoop.h"
#include "../debugging/GbDebugManager.h"

#include "../rendering/renderer/GbMainRenderer.h"
#include "../../view/GbWidgetManager.h"
#include "../../view/GL/GbGLWidget.h"

namespace Gb {
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Using statements
//////////////////////////////////////////////////////////////////////////////////////////////////
   
QJsonValue Viewport::asJson() const
{
    QJsonObject object;
    object.insert("x", m_xn);
    object.insert("y", m_yn);
    object.insert("w", m_width);
    object.insert("h", m_height);
    object.insert("d", m_depth);

    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Viewport::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();
    m_xn = object.value("x").toDouble();
    m_yn = object.value("y").toDouble();
    m_width = object.value("w").toDouble();
    m_height = object.value("h").toDouble();
    m_depth = object.value("d").toDouble();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Viewport::setGLViewport(const GL::MainRenderer& r) const
{
    // Remaps the scene AFTER projection
    // See: https://gamedev.stackexchange.com/questions/75499/glviewport-like-camera
    int xw = (m_xn)*(r.m_widget->width());
    int yw = (m_yn)*(r.m_widget->height());
    glViewport(xw, yw, r.m_widget->width()* m_width, r.m_widget->height()*m_height);
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Camera
//////////////////////////////////////////////////////////////////////////////////////////////////
Camera::Camera() {

}
//////////////////////////////////////////////////////////////////////////////////////////////////
Camera::Camera(CameraComponent* component):
    m_component(component),
    m_renderProjection(component->sceneObject()->engine()->widgetManager()->mainGLWidget())
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////    
Camera::~Camera()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Vector3g Camera::getRightVec() const
{
    // Get first row of rotation matrix
    return Vector3g(m_viewMatrix(0, 0), m_viewMatrix(0, 1), m_viewMatrix(0, 2));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Vector3g Camera::getUpVec() const
{
    // Get second row of rotation matrix
    return Vector3g(m_viewMatrix(1, 0), m_viewMatrix(1, 1), m_viewMatrix(1, 2));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Vector3g Camera::getForwardVec() const
{
    // Get third row of rotation matrix
    return Vector3g(m_viewMatrix(2, 0), m_viewMatrix(2, 1), m_viewMatrix(2, 2));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::widgetToWorldSpace(const Vector3g & widgetSpace, Vector3g & worldSpace) const
{
    const Matrix4x4g& inverseViewMatrix = transform()->worldMatrix();
    worldSpace = inverseViewMatrix * m_renderProjection.m_inverseProjectionMatrix * Vector4g(widgetSpace);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::worldToWidgetSpace(const Vector3g & worldSpace, Vector3g & widgetSpace) const
{
    widgetSpace = m_renderProjection.projectionMatrix() * m_viewMatrix * Vector4g(worldSpace);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Implement object drag
//3D Drag :
//
//void OnMouseDrag()
//{
//    float distance_to_screen = Camera.main.WorldToScreenPoint(gameObject.transform.position).z;
//    transform.position = Camera.main.ScreenToWorldPoint(new Vector3(Input.mousePosition.x, Input.mousePosition.y, distance_to_screen));
//
//}
//
//Plane Drag :
//
//void OnMouseDrag()
//{
//    float distance_to_screen = Camera.main.WorldToScreenPoint(gameObject.transform.position).z;
//    Vector3 pos_move = Camera.main.ScreenToWorldPoint(new Vector3(Input.mousePosition.x, Input.mousePosition.y, distance_to_screen));
//    transform.position = new Vector3(pos_move.x, transform.position.y, pos_move.z);
//
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::widgetToRayDirection(const Vector2g & widgetSpace, Vector3g & outRayDirection, const GL::MainRenderer& renderer)
{
    // Start in widget space; range [0:widget_width, widget_height:0]
    // Convert from widget space to viewport coordinates
    // range [0:viewport_width, viewport_height:0]
    Vector4g coords = Vector4g(widgetSpace.x() - m_viewport.m_xn, 
        widgetSpace.y() - m_viewport.m_yn, 
        -1.0f,  // Setting z for later (want -z direction, so ray points forwards)
        1.0f); // Setting perspective divide for later

    // Convert from viewport to normalized device coordinates
    // range[-1:1, -1 : 1, -1 : 1]
    // Also convert from normalized device coordinates to homogeneous clip coordinates
    // range[-1:1, -1 : 1, -1 : 1, -1 : 1]
    // Note: we do not need to reverse perspective division here because this is a ray with no intrinsic depth.
    real_g width = renderer.widget()->width() * (real_g)m_viewport.m_width;
    real_g height = renderer.widget()->height() * (real_g)m_viewport.m_height;
    coords[0] = 2 * (coords.x() / width) - 1.0;
    coords[1] = 1.0 - 2 * (coords.y() / height); // Flip, since widget-space origin is top-left

    // Convert to eye (camera) coordinates 
    coords = m_renderProjection.m_inverseProjectionMatrix * coords;
    coords[2] = -1.0;
    coords[3] = 0.0;
    
    // Convert to world coordinates
    const Matrix4x4g& inverseView = transform()->worldMatrix();
    outRayDirection = inverseView * coords;
    outRayDirection.normalize();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Vector3g Camera::eye() const
{
    return transform()->getPosition().asReal();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::zoom(const Vector3g & target, real_g delta)
{
    Vector3g eye = transform()->getPosition().asReal();
    eye = target + (eye - target) * (delta + 1.0);

    // Set view/world matrices
    setLookAt(eye, target, getUpVec());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::pan(Vector3g& target, const Vector2g & moveDelta)
{
    // Get necessary parameters
    Vector3g eye = transform()->getPosition().asReal();
    real_g fovY = m_renderProjection.fovY() * Constants::DEG_TO_RAD;
    real_g length = real_g(2.0) * (eye - target).length() * tan(fovY / 2.0);
    Vector3g up = getUpVec();
    Vector3g dirX = up;
    Vector3g dirY = up.cross(target - eye).normalized();
    real_g aspectRatio = real_g(m_renderProjection.m_aspectRatio);

    // Set new target position
    target = target + dirY * moveDelta.x() * length * aspectRatio
        + dirX * moveDelta.y() * length;

    // Set view/world matrices
    setLookAt(eye, target, up);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::tilt(Vector3g & target, const Vector2g & moveDelta)
{
    Vector3g eye = transform()->getPosition().asReal();
    Vector3g up = getUpVec();
    Vector3g forward = getForwardVec();
    Vector3g right = getRightVec();
    up += moveDelta.x() * right;
    up.normalize();

    //forward += moveDelta.y() * up;
    //forward.normalize();
    //real_g dist = (eye - target).length();
    //target = eye + forward * dist;

    // Set view/world matrices
    setLookAt(eye, target, up);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::translate(Vector3g & target, const Vector2g & moveDelta, real_g speedFactor)
{
    Vector3g eye = transform()->getPosition().asReal();
    Vector3g up = getUpVec();
    Vector3g dirX = up;
    Vector3g dirY = up.cross(getForwardVec()).normalized();
       
    Vector3g delta = (moveDelta.x() * dirY + moveDelta.y() * dirX) * speedFactor;
    eye += delta;
    target += delta;

    // Set view/world matrices
    setLookAt(eye, target, up);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::rotateAboutPoint(const Vector3g & target, const Vector2g & mouseDelta, real_g speedFactor)
{
    // Get rotation axis directions
    Vector3g up = getUpVec();
    Vector3g eye = transform()->getPosition().asReal();

    Vector3g dirX = up;
    Vector3g dirY = up.cross(target - eye).normalized();

    // Perform rotation
    eye = Quaternion::rotatePoint(eye, target, dirX, -mouseDelta.x() * M_PI * speedFactor);
    eye = Quaternion::rotatePoint(eye, target, dirY, mouseDelta.y() * M_PI * speedFactor);
    up = Quaternion::rotatePoint(target + up, target, dirY, mouseDelta.y() * M_PI * speedFactor) - target;
    up.normalize();

    // Set view/world matrices
    setLookAt(eye, target, up);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::setLookAt(const Vector3g & eye, const Vector3g & target, const Vector3g & up)
{
    m_viewMatrix = Camera::lookAtRH(eye, target, up);
    updateTransform();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::setGLViewport(const GL::MainRenderer& r)
{
    m_viewport.setGLViewport(r);
    //m_renderProjection.computeProjectionMatrix();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::bindUniforms()
{
    if (UBO::getCameraBuffer()) {
        auto cameraBuffer = UBO::getCameraBuffer();
        cameraBuffer->setUniformValue("viewMatrix", m_viewMatrix);
        cameraBuffer->setUniformValue("projectionMatrix", m_renderProjection.projectionMatrix());
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Camera::asJson() const
{
    QJsonObject object;
    object.insert("projection", m_renderProjection.asJson());
    object.insert("viewMatrix", m_viewMatrix.asJson());
    object.insert("viewport", m_viewport.asJson());
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();
    m_renderProjection.loadFromJson(json["projection"]);
    m_viewMatrix.loadFromJson(json["viewMatrix"]);
    if (object.contains("viewport")) {
        m_viewport.loadFromJson(object["viewport"]);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
TransformComponent * Camera::transform() const
{
    TransformComponent* transform = m_component->sceneObject()->transform().get();
    return transform;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::updateTransform()
{
    Matrix4x4g worldMatrix = m_viewMatrix.inversed();
    transform()->updateWorldMatrix(worldMatrix);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::computeViewMatrix(const Matrix4x4f& worldMatrix)
{
    m_viewMatrix = worldMatrix.inversed();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4g Camera::lookAtRH(const Vector3g & eye, const Vector3g & target, const Vector3g & up)
{
    Vector3g zaxis = (eye - target).normalized();    // The "forward" vector.
    Vector3g xaxis = up.cross(zaxis).normalized();   // The "right" vector.
    Vector3g yaxis = zaxis.cross(xaxis);             // The "up" vector.

    // Create a 4x4 view matrix from the right, up, forward and eye position vectors
    Matrix4x4g viewMatrix = std::vector<Vector4g>{
        Vector4g(xaxis.x(),          yaxis.x(),          zaxis.x(),       0),
        Vector4g(xaxis.y(),          yaxis.y(),          zaxis.y(),       0),
        Vector4g(xaxis.z(),          yaxis.z(),          zaxis.z(),       0),
        Vector4g(-xaxis.dot(eye),   -yaxis.dot(eye),    -zaxis.dot(eye),  1)
    };

    return viewMatrix;
}




//////////////////////////////////////////////////////////////////////////////////////////////////
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
    const Vector2g& mouseDelta = inputHandler().mouseHandler().mouseDelta();
    const Vector2g& scrollDelta = inputHandler().mouseHandler().scrollDelta();

    //if (scrollDelta.y())
    //    logInfo(scrollDelta);

    //if (inputHandler().mouseHandler().isHeld(Qt::MouseButton::LeftButton)) {
    //    // Rotate while left mouse button is clicked
    //}

    // Check that camera is not located at target position
    Camera& cam = camera();
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
            if (inputHandler().keyHandler().isHeld(Qt::Key::Key_Up)) {
                cam.translate(m_target, Vector2g(0.0f, 1.0f) * profile.m_translateScaling);
            }
            if (inputHandler().keyHandler().isHeld(Qt::Key::Key_Down)) {
                cam.translate(m_target, Vector2g(0.0f, -1.0f) * profile.m_translateScaling);
            }
            if (inputHandler().keyHandler().isHeld(Qt::Key::Key_Left)) {
                cam.translate(m_target, Vector2g(-1.0f, 0.0f) * profile.m_translateScaling);
            }
            if (inputHandler().keyHandler().isHeld(Qt::Key::Key_Right)) {
                cam.translate(m_target, Vector2g(1.0f, 0.0f) * profile.m_translateScaling);
            }
            break;
        case kTilt:
            if (inputHandler().mouseHandler().isHeld(Qt::MouseButton::MiddleButton)) {
                cam.tilt(m_target, mouseDelta * profile.m_tiltScaling);
            }
            break;
        case kPan:
            if (inputHandler().mouseHandler().isHeld(Qt::MouseButton::RightButton)) {
                cam.pan(m_target, mouseDelta * profile.m_panScaling);
            }
            break;
        case kZoom:
            cam.zoom(m_target, scrollDelta.y() * profile.m_zoomScaling);
            break;
        case kRotate:
            if (inputHandler().mouseHandler().isHeld(Qt::MouseButton::LeftButton)) {
                cam.rotateAboutPoint(m_target, mouseDelta * profile.m_rotateScaling);
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
void CameraController::loadFromJson(const QJsonValue & json)
{
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
Camera & CameraController::camera()
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
void CameraController::ControllerProfile::loadFromJson(const QJsonValue & json)
{
    QJsonObject object = json.toObject();

    QJsonArray movementTypes = object.value("movementTypes").toArray();
    for (auto moveTypeJson : movementTypes) {
        MovementType moveType = (MovementType)moveTypeJson.toInt();
        m_movementTypes[moveType] = true;
    }

    //m_target = Vector3g(object.value("target"));
    m_zoomScaling = real_g(object.value("zoomScaling").toDouble());
    m_translateScaling = Vector2g(object.value("translateScaling"));
    m_rotateScaling = Vector2g(object.value("rotateScaling"));
    m_panScaling = Vector2g(object.value("panScaling"));
    if(object.contains("tiltScaling"))
        m_tiltScaling = Vector2g(object.value("tiltScaling"));
}




//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
CameraComponent::CameraComponent() :
    Component(kCamera),
    m_camera(this),
    m_controller(this)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CameraComponent::CameraComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, kCamera),
    m_camera(this),
    m_controller(this)
{
    object->setCamera(this);

    // Add camera to scene
    object->scene()->addCamera(this);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CameraComponent::~CameraComponent()
{
    // Remove camera from scene;
    if (sceneObject()) {
        std::shared_ptr<Scene> scene = sceneObject()->scene();
        if (scene)
            scene->removeCamera(this);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CameraComponent::drawScene(std::shared_ptr<Scene> scene, const GL::MainRenderer& renderer)
{    
    // TODO: Add rendering layers

    // Skip rendering for camera if disabled
    if (!m_isEnabled) return;

    SimulationLoop::PlayMode mode = m_engine->simulationLoop()->getPlayMode();

    // Set the viewport size for camera rendering
    m_camera.setGLViewport(renderer);

    // Bind camera uniforms
    m_camera.bindUniforms();

    // Iterate through scene objects
    auto& topLevelObjects = scene->topLevelSceneObjects();
    for (const std::shared_ptr<SceneObject>& sceneObject : topLevelObjects) {

        // Get bind mode of the shader
        // TODO: Implement intelligent binding and unbinding of shaders/materials
        sceneObject->draw(SceneObject::kModel);

    }

    // Render skybox if there is one
    // Rendered after everything else to discard unnecessary vertices
    if (scene->skybox()) {
        // Check if skybox still exists, and delete if not
        if (!m_engine->resourceCache()->hasModel(scene->skybox()->getName())) {
            scene->setSkybox(nullptr);
        }
        else {
            // Render skybox
            auto cubemapShader = m_engine->resourceCache()->getShaderProgramByName("cubemap");
            cubemapShader->bind();
            scene->skybox()->draw(cubemapShader);

        }
    }

    // Render grid and debug renderables if in debug mode
    if (mode == SimulationLoop::kDebug) {
        m_engine->debugManager()->draw(scene);
    }

    // Render canvases
    // TODO: Implement intelligent binding and unbinding of shaders/materials
    std::vector<CanvasComponent*>& canvases = scene->canvases();
    for (CanvasComponent* canvas : canvases) {
        // Render the canvases
        canvas->sceneObject()->draw(SceneObject::kCanvas);
    }

    // TODO: Redo this with framebuffers
    // Clear depth buffer for camera renders, really want to use framebuffers instead
    // See: https://stackoverflow.com/questions/13710791/multiple-viewports-interfering
    glClear(GL_DEPTH_BUFFER_BIT);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue CameraComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    object.insert("camera", m_camera.asJson());
    object.insert("controller", m_controller.asJson());
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CameraComponent::loadFromJson(const QJsonValue & json)
{
    Component::loadFromJson(json);
    QJsonObject object = json.toObject();
    if (object.contains("camera")) {
        m_camera.loadFromJson(object.value("camera"));
        m_controller.loadFromJson(object.value("controller"));
    }
    else {
        // Preserve backwards compatibility
        m_camera.loadFromJson(json);
    }
}



//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}
