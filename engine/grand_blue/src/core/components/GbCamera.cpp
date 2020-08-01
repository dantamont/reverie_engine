#include "GbCamera.h"
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
#include "../rendering/shaders/GbUniformBufferObject.h"
#include "../rendering/materials/GbMaterial.h"

#include "../loop/GbSimLoop.h"
#include "../debugging/GbDebugManager.h"

#include "../rendering/renderer/GbMainRenderer.h"
#include "../rendering/renderer/GbRenderCommand.h"

#include "../../view/GbWidgetManager.h"
#include "../../view/GL/GbGLWidget.h"
#include "../containers/GbFlags.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Frustum
//////////////////////////////////////////////////////////////////////////////////////////////////
Frustum::Frustum(Camera * camera):
    m_camera(camera)
{
    m_planes.resize(6);
    initialize();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Frustum::~Frustum()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Frustum::initialize()
{
    initialize(m_camera->getViewMatrix(), m_camera->renderProjection().projectionMatrix());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Frustum::normalizePlanes()
{
    for (BoundingPlane& planePair : m_planes) {
        planePair.normalize();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool Frustum::contains(const CollidingGeometry & geometry) const
{
    bool inside = false;
    BoundingPlane::Halfspace result = BoundingPlane::kPositive;
    switch (geometry.geometryType()) {
    case CollidingGeometry::kAABB: {
        const AABB& aabb = static_cast<const AABB&>(geometry);

        for (int i = 0; i < 6; i++) {
            // Return if the aabb is outside of any of the planes
            result = m_planes[i].classifyGeometry(aabb);
            if (result != BoundingPlane::kPositive)
                // Returns if either entirely on wrong side of plane, or if intersects with one
                return result != BoundingPlane::kNegative;
        }
        inside = result != BoundingPlane::kNegative;
        break;
    }
    case CollidingGeometry::kBoundingSphere: {
        const BoundingSphere& sphere = static_cast<const BoundingSphere&>(geometry);
        for (int i = 0; i < 6; i++) {
            // Return if the sphere is outside of any of the planes
            result = m_planes[i].classifyGeometry(sphere);
            if (result != BoundingPlane::kPositive)
                // Returns if either entirely on wrong side of plane, or if intersects with one
                return result != BoundingPlane::kNegative;
        }
        inside = result != BoundingPlane::kNegative;
        break;
    }
    case CollidingGeometry::kPoint: {
        return contains(static_cast<const Vector3g&>(static_cast<const CollidingPoint&>(geometry)));
    }
    default:
        throw("Error, type of geometry not supported");
    }
    return inside;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool Frustum::contains(const Vector3g & point) const
{
    // Returns true if the point is in the positive direction or intersects for all frustum planes
    BoundingPlane::Halfspace result;
    for (int i = 0; i < 6; i++) {
        result = m_planes.at(i).classifyGeometry(point);
        if (result != BoundingPlane::kPositive)
            // Returns if either entirely on wrong side of plane, or if intersects with one
            return result != BoundingPlane::kNegative;
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Frustum::initialize(const Matrix4x4g & viewMatrix, const Matrix4x4g & projectionMatrix)
{
    Matrix4x4g viewProjection = projectionMatrix * viewMatrix;

    // Left clipping plane
    m_planes[kLeft] = BoundingPlane();
    m_planes[kLeft].setA(viewProjection(3, 0) + viewProjection(0, 0));
    m_planes[kLeft].setB(viewProjection(3, 1) + viewProjection(0, 1));
    m_planes[kLeft].setC(viewProjection(3, 2) + viewProjection(0, 2));
    m_planes[kLeft].setD(viewProjection(3, 3) + viewProjection(0, 3));

    // Right clipping plane
    m_planes[kRight] = BoundingPlane();
    m_planes[kRight].setA(viewProjection(3, 0) - viewProjection(0, 0));    
    m_planes[kRight].setB(viewProjection(3, 1) - viewProjection(0, 1));
    m_planes[kRight].setC(viewProjection(3, 2) - viewProjection(0, 2));
    m_planes[kRight].setD(viewProjection(3, 3) - viewProjection(0, 3));

    // Top clipping plane
    m_planes[kTop] = BoundingPlane();
    m_planes[kTop].setA(viewProjection(3, 0) - viewProjection(1, 0));
    m_planes[kTop].setB(viewProjection(3, 1) - viewProjection(1, 1));
    m_planes[kTop].setC(viewProjection(3, 2) - viewProjection(1, 2));
    m_planes[kTop].setD(viewProjection(3, 3) - viewProjection(1, 3));

    // Bottom clipping plane
    m_planes[kBottom] = BoundingPlane();
    m_planes[kBottom].setA(viewProjection(3, 0) + viewProjection(1, 0));
    m_planes[kBottom].setB(viewProjection(3, 1) + viewProjection(1, 1));
    m_planes[kBottom].setC(viewProjection(3, 2) + viewProjection(1, 2));
    m_planes[kBottom].setD(viewProjection(3, 3) + viewProjection(1, 3));

    // Near clipping plane
    m_planes[kNear] = BoundingPlane();
    m_planes[kNear].setA(viewProjection(3, 0) + viewProjection(2, 0));
    m_planes[kNear].setB(viewProjection(3, 1) + viewProjection(2, 1));
    m_planes[kNear].setC(viewProjection(3, 2) + viewProjection(2, 2));
    m_planes[kNear].setD(viewProjection(3, 3) + viewProjection(2, 3));

    // Far clipping plane
    m_planes[kFar] = BoundingPlane();
    m_planes[kFar].setA(viewProjection(3, 0) - viewProjection(2, 0));
    m_planes[kFar].setB(viewProjection(3, 1) - viewProjection(2, 1));
    m_planes[kFar].setC(viewProjection(3, 2) - viewProjection(2, 2));
    m_planes[kFar].setD(viewProjection(3, 3) - viewProjection(2, 3));

    // Normalize planes
    normalizePlanes();

    //for (const std::pair<Plane, BoundingPlane>& planePair : m_planes) {
    //    logInfo(QString::number(int(planePair.first))
    //        + " "+ QString(planePair.second.normal().normalized()));
    //}

    //logInfo(QString::number((int)contains(Vector3(0, 0, 0))));

    // Test box
    //AABB box;
    //box.setMinX(0);
    //box.setMaxX(10);
    //box.setMinY(0);
    //box.setMaxY(10);
    //box.setMinZ(0);
    //box.setMaxZ(10);
    //logInfo(QString::number((int)contains(box)));
    //std::vector<Vector3g> vec = { Vector3g(10, 0, 0), Vector3g(-5, 9, 2), Vector3g(3, -12, -17) };
    //AABB box(vec);
}



//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Viewport
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
void Viewport::setGLViewport(const MainRenderer& r) const
{
    // Changed this, offset is now used for screen quads in shader
    // Remaps the scene AFTER projection
    // See: https://gamedev.stackexchange.com/questions/75499/glviewport-like-camera
    //int xw = (m_xn)*(r.m_widget->width());
    //int yw = (m_yn)*(r.m_widget->height());
    glViewport(0, 0, r.m_widget->width()* m_width, r.m_widget->height()*m_height);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Viewport::resizeFrameBuffer(const MainRenderer& r, FrameBuffer & fbo) const
{
    // Remaps the scene AFTER projection
    // See: https://gamedev.stackexchange.com/questions/75499/glviewport-like-camera
    int w = (m_width)*(r.m_widget->width());
    int h = (m_height)*(r.m_widget->height());
    fbo.reinitialize(w, h);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Camera
//////////////////////////////////////////////////////////////////////////////////////////////////
//Camera::Camera():  
//    m_frustum(this),
//    m_cameraOptions(3)
//{
//    // Add default render layer
//    //m_renderLayers.emplace_back(SortingLayer());
//    setIndex();
//}
//////////////////////////////////////////////////////////////////////////////////////////////////
Camera::Camera(CameraComponent* component):
    m_component(component),
    m_frustum(this),
    m_cameraOptions(3),
    m_renderProjection(component->sceneObject()->engine()->widgetManager()->mainGLWidget()),
    m_frameBuffer(component->sceneObject()->engine()->getGLWidgetContext(),
        InternalBufferFormat::kMSAA,
        FrameBuffer::BufferStorageType::kRBO,
        16)
{
    // Add default render layers
    setDefaultRenderLayers();
    setIndex();
}
//////////////////////////////////////////////////////////////////////////////////////////////////    
Camera::~Camera()
{
    s_deletedIndices.push_back(m_index);
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
float Camera::getDepth(const Vector3g & position)
{
    float depth;

    Vector3g forward = getForwardVec();
    Vector3g distance = position - eye();
    depth = forward.dot(distance);

    return depth;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::drawFrameBufferQuad(MainRenderer & r)
{
    // Obtain quad
    std::shared_ptr<Mesh> quad = m_component->sceneObject()->engine()->resourceCache()->polygonCache()->getSquare();

    // Obtain shader
    auto quadShaderProgram = m_component->sceneObject()->engine()->resourceCache()->getHandleWithName("quad",
        Resource::kShaderProgram)->resourceAs<ShaderProgram>();

    // Bind shader and set uniforms
    quadShaderProgram->bind();
    quadShaderProgram->setUniformValue("offsets", Vector3g(m_viewport.m_xn, m_viewport.m_yn, m_viewport.m_depth), true);
    quadShaderProgram->setUniformValue("scale", Vector2g(m_viewport.m_width, m_viewport.m_height), true);

#ifdef DEBUG_MODE
    bool error = printGLError("Camera::drawFrameBufferQuad:: Error binding shader");
    if (error) {
        logInfo("Error in Camera::drawFrameBufferQuad: " + m_name);
    }
#endif

    // Bind texture
    m_frameBuffer.bindColorAttachment();

#ifdef DEBUG_MODE
    error = printGLError("Camera::drawFrameBufferQuad:: Error binding texture");
    if (error) {
        logInfo("Error in Camera::drawFrameBufferQuad: " + m_name);
    }
#endif

    // Draw the quad
    quad->vertexData().drawGeometry(GL_TRIANGLES);

    // Release shader
    quadShaderProgram->release();
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
void Camera::widgetToRayDirection(const Vector2g & widgetSpace, Vector3g & outRayDirection, const MainRenderer& renderer)
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
void Camera::translate(Vector3g & target, const Vector3g & moveDelta, real_g speedFactor)
{
    Vector3g eye = transform()->getPosition().asReal();
    Vector3g up = getUpVec();
    Vector3g dirX = up;
    Vector3g dirY = up.cross(getForwardVec()).normalized();
    Vector3g dirZ = dirX.cross(dirY).normalized();
       
    Vector3g delta = (moveDelta.x() * dirY + moveDelta.y() * dirX + moveDelta.z() * dirZ) * speedFactor;
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
    updateFrustum();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::setGLViewport(const MainRenderer& r)
{
    // Resize viewport
    m_viewport.setGLViewport(r);
    //m_renderProjection.computeProjectionMatrix();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::resizeFrameBuffer(const MainRenderer& r)
{
    // Resize framebuffer dimensions
    m_viewport.resizeFrameBuffer(r, m_frameBuffer);

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
    
    QJsonArray renderLayers;
    for (const std::shared_ptr<SortingLayer>& layer: getRenderLayers()) {
        renderLayers.push_back(layer->getName());
    }
    object.insert("renderLayers", renderLayers);

    // Camera options
    object.insert("cameraOptions", (int)m_cameraOptions);

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

    m_renderLayers.clear();
    if (object.contains("renderLayers")) {
        for (const auto& layerJson : object["renderLayers"].toArray()) {
            QString layerName = layerJson.toString();
            m_renderLayers.push_back(
                m_component->sceneObject()->scene()->scenario()->settings().renderLayer(layerName));
        }
    }

    // Camera options
    if (object.contains("cameraOptions")) {
        m_cameraOptions = Flags::toFlags<CameraOption>(object["cameraOptions"].toInt());
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
void Camera::updateViewMatrix(const Matrix4x4f& worldMatrix)
{
    m_viewMatrix = worldMatrix.inversed();
    updateFrustum();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::updateFrustum()
{
    m_frustum.initialize();
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
void Camera::setIndex()
{
    if (s_deletedIndices.size()) {
        m_index = s_deletedIndices.back();
        s_deletedIndices.pop_back();
    }
    else {
        m_index = s_cameraCount;
        s_cameraCount++;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::shared_ptr<SortingLayer>> Camera::getRenderLayers() const
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
void Camera::setDefaultRenderLayers()
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
std::vector<std::shared_ptr<SortingLayer>> Camera::renderLayers()
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
std::vector<size_t> Camera::s_deletedIndices = {};

//////////////////////////////////////////////////////////////////////////////////////////////////
size_t Camera::s_cameraCount = 0;





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

    // Return if no movement
    bool mouseMoved = inputHandler().mouseHandler().wasMoved();
    bool scrolled = inputHandler().mouseHandler().wasScrolled();

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
        {
            bool shiftPressed = inputHandler().keyHandler().isHeld(Qt::Key::Key_Shift);
            if (inputHandler().keyHandler().isHeld(Qt::Key::Key_Up)) {
                if (shiftPressed) {
                    cam.translate(m_target, Vector3g(0.0f, 0.0f, 1.0f) * profile.m_translateScaling);
                }
                else {
                    cam.translate(m_target, Vector3g(0.0f, 1.0f, 0.0f) * profile.m_translateScaling);
                }
            }
            if (inputHandler().keyHandler().isHeld(Qt::Key::Key_Down)) {
                if (shiftPressed) {
                    cam.translate(m_target, Vector3g(0.0f, 0.0f, -1.0f) * profile.m_translateScaling);
                }
                else {
                    cam.translate(m_target, Vector3g(0.0f, -1.0f, 0.0f) * profile.m_translateScaling);
                }
            }
            if (inputHandler().keyHandler().isHeld(Qt::Key::Key_Left)) {
                cam.translate(m_target, Vector2g(-1.0f, 0.0f) * profile.m_translateScaling);
            }
            if (inputHandler().keyHandler().isHeld(Qt::Key::Key_Right)) {
                cam.translate(m_target, Vector2g(1.0f, 0.0f) * profile.m_translateScaling);
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
    m_translateScaling = Vector3g(object.value("translateScaling"));
    m_rotateScaling = Vector2g(object.value("rotateScaling"));
    m_panScaling = Vector2g(object.value("panScaling"));
    if(object.contains("tiltScaling"))
        m_tiltScaling = Vector2g(object.value("tiltScaling"));
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
    if (m_camera.cameraOptions().testFlag(Camera::kShowAllRenderLayers)) {
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
    if(!m_cubeMapID.isNull())
        object.insert("cubeMap", m_cubeMapID.asString());
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CameraComponent::loadFromJson(const QJsonValue & json)
{
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
