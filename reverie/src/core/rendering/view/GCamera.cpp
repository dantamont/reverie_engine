#include "GCamera.h"
#include "GCamera.h"

#include "../../GCoreEngine.h"
#include "../../containers/GColor.h"
#include "../../components/GTransformComponent.h"

#include "../../resource/GResourceCache.h"
#include "../../scene/GSceneObject.h"
#include "../models/GModel.h"
#include "../shaders/GShaderProgram.h"
#include "../buffers/GUniformBufferObject.h"

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
// Abstract Camera
//////////////////////////////////////////////////////////////////////////////////////////////////

AbstractCamera::AbstractCamera()
{    
    // Set camera index
    setIndex();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
AbstractCamera::~AbstractCamera()
{
    s_deletedIndices.push_back(m_index);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractCamera::setIndex()
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
std::vector<size_t> AbstractCamera::s_deletedIndices = {};

//////////////////////////////////////////////////////////////////////////////////////////////////
size_t AbstractCamera::s_cameraCount = 0;




//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Camera
//////////////////////////////////////////////////////////////////////////////////////////////////
Camera::Camera()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Camera::Camera(const Camera & other) :
    m_frameBuffers(other.m_frameBuffers),
    m_transform(other.m_transform),
    m_viewport(other.m_viewport),
    m_viewMatrix(other.m_viewMatrix),
    m_frustum(other.m_frustum),
    m_clearColor(Vector4(0.55f, 0.6f, 0.93f, 1.0f))
{
    m_renderProjection = other.m_renderProjection;
    m_renderProjection.m_camera = this;

    // Initialize render projection
    m_renderProjection.updateProjection();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Camera::Camera(Transform* transform,
    QOpenGLContext* glContext, AliasingType frameBufferFormat,
    FrameBuffer::BufferAttachmentType framebufferStorageType, size_t numSamples,
    size_t numColorAttachments) :
    m_frustum(m_viewMatrix, m_renderProjection.projectionMatrix()),
    m_frameBuffers(glContext,
        frameBufferFormat,
        framebufferStorageType,
        FBO_FLOATING_POINT_TEX_FORMAT,
        numSamples,
        numColorAttachments),
    m_transform(transform),
    m_renderProjection(this),
    m_clearColor(Vector4(0.55f, 0.6f, 0.93f, 1.0f))
{
    // Initialize render projection
    m_renderProjection.updateProjection();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Camera::~Camera()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Camera & Camera::operator=(const Camera & other)
{
    m_transform = other.m_transform;
    m_frameBuffers = other.m_frameBuffers;
    m_viewport = other.m_viewport;
    m_viewMatrix = other.m_viewMatrix;
    m_renderProjection = RenderProjection(this);
    m_frustum = other.m_frustum;
    return *this;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Vector3 Camera::getRightVec() const
{
    // Get first row of rotation matrix
    return Vector3(m_viewMatrix(0, 0), m_viewMatrix(0, 1), m_viewMatrix(0, 2));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Vector3 Camera::getUpVec() const
{
    // Get second row of rotation matrix
    return Vector3(m_viewMatrix(1, 0), m_viewMatrix(1, 1), m_viewMatrix(1, 2));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Vector3 Camera::getForwardVec() const
{
    // Get third row of rotation matrix, see:
    // Forward vec is the OPPOSITE of the camera's look at direction
    // https://gamedev.stackexchange.com/questions/104862/how-to-find-the-up-direction-of-the-view-matrix-with-glm
    //| ux vx - nx - eyex | column 0
    //| uy vy - ny - eyey | column 1
    //| uz vz - nz - eyez | column 2
    //| 0  0    0     1   | column 3
    // View matrix, where u is up vector, n is direction camera is looking, and v is perpenduicular to both n and u
    
    return Vector3(m_viewMatrix(2, 0), m_viewMatrix(2, 1), m_viewMatrix(2, 2));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
float Camera::getDepth(const Vector3 & position)
{
    float depth;

    Vector3 forward = getForwardVec();
    Vector3 distance = position - eye();
    depth = forward.dot(distance);

    return depth;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool Camera::canSee(const CollidingGeometry & geometry) const
{
    return m_frustum.intersects(geometry);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::bindFrame(FrameBuffer::BindType readOrWrite, bool clear)
{
    // Set viewport
    setGLViewport();

    // Bind framebuffer
    if (readOrWrite == FrameBuffer::BindType::kWrite) {
        m_frameBuffers.writeBuffer().bind();
        if (clear) {
            m_frameBuffers.writeBuffer().clear(m_clearColor);
        }
    }
    else {
        m_frameBuffers.readBuffer().bind();
        if (clear) {
            throw("Error, cannot clear read buffer");
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::releaseFrame(FrameBuffer::BindType readOrWrite)
{
    if (readOrWrite == FrameBuffer::BindType::kWrite) {
        m_frameBuffers.writeBuffer().release();
    }
    else {
        m_frameBuffers.readBuffer().release();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::drawFrameBufferQuad(CoreEngine* core)
{
    // Draw Quad
    auto quadShaderProgram = core->resourceCache()->getHandleWithName("quad",
        ResourceType::kShaderProgram)->resourceAs<ShaderProgram>();
    m_frameBuffers.writeBuffer().drawQuad(*this, *quadShaderProgram);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::widgetToFrame(const Vector2 & widgetSpace, const MainRenderer & renderer, real_g & outFrameX, real_g & outFrameY) const
{
    // Convert to clip coordinates
    m_viewport.widgetToClip(widgetSpace, renderer, outFrameX, outFrameY);

    // Convert to frame coordinates
    clipToFrame(outFrameX, outFrameY, outFrameX, outFrameY);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::clipToFrame(real_g clipX, real_g clipY, real_g & outFrameX, real_g & outFrameY) const
{
    // Convert to [0, 1], then scale up based on framebuffer dimensions
    // Could also multiple by m_viewport.m_width * widget.width(), since framebuffer always matches viewport size
    outFrameX = (outFrameX + 1.0) / 2.0;
    outFrameX *= m_frameBuffers.readBuffer().width();

    outFrameY = (outFrameY + 1.0) / 2.0;
    outFrameY *= m_frameBuffers.readBuffer().height();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//void Camera::widgetToWorldSpace(const Vector2 & widgetSpace, Vector4 & worldSpace) const
//{
//    // See: widgetToRayDirection
//    //    1. Obtain your mouse coordinates within the client area
//    //    2. Get your Projection matrix and View matrix if no Model matrix required.
//    //    3. Multiply Projection * View
//    //    4. Inverse the results of multiplication
//    //
//    //    5. Construct a vector4 consisting of
//    //
//    //      x = mouseposition.x within a range of window x
//    //
//    //          transform to values between - 1 and 1
//    //
//    //      y = mouseposition.y within a range of window y
//    //
//    //          transform to values between - 1 and 1
//    //          remember to invert mouseposition.y if needed
//    //
//    //      z = the depth value(this can be obtained with glReadPixel)
//    //
//    //          you can manually go from - 1 to 1 (zNear, zFar), (2.0 * depth - 1.0 ??)
//    //
//    //      w = 1.0
//    //
//    //    Multiply the vector by inversed matrix created before
//    //
//    //    Divide result vector by it's w component after matrix multiplication ( perspective division )
//    const Matrix4x4g& inverseViewMatrix = transform()->worldMatrix();
//    worldSpace = inverseViewMatrix * m_renderProjection.m_inverseProjectionMatrix * Vector4(widgetSpace);
//}
////////////////////////////////////////////////////////////////////////////////////////////////////
//void Camera::worldToWidgetSpace(const Vector4 & worldSpace, Vector2 & widgetSpace) const
//{
//    widgetSpace = m_renderProjection.projectionMatrix() * m_viewMatrix * Vector4(worldSpace);
//}
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
void Camera::widgetToRayDirection(const Vector2 & widgetSpace, Vector3 & outRayDirection, const MainRenderer& renderer) const
{
    // Get clip coordinates
    Vector4 coords = Vector4(0, 0,
        -1.0f,  // Setting z for later (want -z direction, so ray points forwards)
        1.0f); // Setting perspective divide for later
    m_viewport.widgetToClip(widgetSpace, renderer, coords[0], coords[1]);

    // Convert to eye (camera) coordinates 
    // Note: we do not need to reverse perspective division here because this is a ray (direction) with no intrinsic depth.
    coords = m_renderProjection.m_inverseProjectionMatrix * coords;
    coords[2] = -1.0;
    coords[3] = 0.0;

    // Convert to world coordinates
    const Matrix4x4g& inverseView = transform()->worldMatrix();
    outRayDirection = inverseView * coords;
    outRayDirection.normalize();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
const Vector3& Camera::eye() const
{
    return transform()->getPosition();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::zoom(const Vector3 & target, real_g delta)
{
    const Vector3& oldEye = transform()->getPosition();
    Vector3 eye = target + (oldEye - target) * (delta + 1.0);

    // Set view/world matrices
    setLookAt(eye, target, getUpVec());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::pan(Vector3& target, const Vector2 & moveDelta)
{
    // Get necessary parameters
    const Vector3& eye = transform()->getPosition();
    real_g fovY = m_renderProjection.fovY() * Constants::DEG_TO_RAD;
    real_g length = real_g(2.0) * (eye - target).length() * tan(fovY / 2.0);
    Vector3 up = getUpVec();
    Vector3 dirX = up;
    Vector3 dirY = up.cross(target - eye).normalized();
    real_g aspectRatio = real_g(m_renderProjection.m_aspectRatio);

    // Set new target position
    target = target + dirY * moveDelta.x() * length * aspectRatio
        + dirX * moveDelta.y() * length;

    // Set view/world matrices
    setLookAt(eye, target, up);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::tilt(Vector3 & target, const Vector2 & moveDelta)
{
    const Vector3& eye = transform()->getPosition();
    Vector3 up = getUpVec();
    Vector3 forward = getForwardVec();
    Vector3 right = getRightVec();
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
void Camera::translate(Vector3 & target, const Vector3 & moveDelta, real_g speedFactor)
{
    Vector3 eye = transform()->getPosition();
    Vector3 up = getUpVec();
    Vector3 dirX = up;
    Vector3 dirY = up.cross(getForwardVec()).normalized();
    Vector3 dirZ = dirX.cross(dirY).normalized();

    Vector3 delta = (moveDelta.x() * dirY + moveDelta.y() * dirX + moveDelta.z() * dirZ) * speedFactor;
    eye += delta;
    target += delta;

    // Set view/world matrices
    setLookAt(eye, target, up);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::rotateAboutPoint(const Vector3 & target, const Vector2 & mouseDelta, real_g speedFactor)
{
    // Get rotation axis directions
    Vector3 up = getUpVec();
    Vector3 eye = transform()->getPosition();

    Vector3 dirX = up;
    Vector3 dirY = up.cross(target - eye).normalized();

    // Perform rotation
    eye = Quaternion::rotatePoint(eye, target, dirX, -mouseDelta.x() * M_PI * speedFactor);
    eye = Quaternion::rotatePoint(eye, target, dirY, mouseDelta.y() * M_PI * speedFactor);
    up = Quaternion::rotatePoint(target + up, target, dirY, mouseDelta.y() * M_PI * speedFactor) - target;
    up.normalize();

    // Set view/world matrices
    setLookAt(eye, target, up);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//void Camera::rotateAboutAxis(const Vector3 & target, const Vector3 & axis, const Vector2 & mouseDelta, real_g speedFactor)
//{
//    throw("unimplemented");
//}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::setLookAt(const Vector3 & eye, const Vector3 & target, const Vector3 & up)
{
    // TODO: Wrap up in Transform class, would be convenient
    m_viewMatrix = Camera::LookAtRH(eye, target, up);
    updateTransform();
    updateFrustum(CameraUpdateFlag::kViewUpdated);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//void Camera::setGLViewport(const MainRenderer& r)
//{
//    // Resize viewport
//    m_viewport.setGLViewport(r);
//    //m_renderProjection.computeProjectionMatrix();
//}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::setGLViewport()
{
    // Resize viewport
    for (FrameBuffer& fbo : m_frameBuffers.frameBuffers()) {
        m_viewport.setGLViewport(fbo);
    }
    //m_renderProjection.computeProjectionMatrix();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::resizeFrame(size_t width, size_t height)
{
    // Resize projection
    m_renderProjection.resizeProjection(width, height);

    // Resize framebuffer dimensions
    for (FrameBuffer& fbo : m_frameBuffers.frameBuffers()) {
        m_viewport.resizeFrameBuffer(width, height, fbo);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::bindUniforms(const MainRenderer* renderer, ShaderProgram* shaderProgram)
{
    Q_UNUSED(shaderProgram);

    if (UBO::getCameraBuffer()) {
        auto cameraBuffer = UBO::getCameraBuffer();
        cameraBuffer->setUniformValue(Uniform("viewMatrix", m_viewMatrix));
        cameraBuffer->setUniformValue(Uniform("projectionMatrix", m_renderProjection.projectionMatrix()));
        cameraBuffer->setUniformValue(Uniform("zNear", (real_g)m_renderProjection.nearClipPlane()));
        cameraBuffer->setUniformValue(Uniform("zFar", (real_g)m_renderProjection.farClipPlane()));
        cameraBuffer->setUniformValue(Uniform("invViewMatrix", m_transform->worldMatrix()));
        cameraBuffer->setUniformValue(Uniform("invProjectionMatrix", m_renderProjection.inverseProjectionMatrix()));
        
        if (renderer) {
            Vector2u viewportDimensions(m_frameBuffers.writeBuffer().width(), m_frameBuffers.writeBuffer().height());
            //float vw = m_viewport.m_width;
            //float vh = m_viewport.m_height;
            //const View::GLWidget* widget = renderer->widget();
            //viewportDimensions[0] = unsigned int(widget->width() * vw);
            //viewportDimensions[1] = unsigned int(widget->height() * vh);
            cameraBuffer->setUniformValue(Uniform("viewportDimensions", viewportDimensions));
        }
        //float zNear = cameraBuffer->getUniformBufferValue("zNear").get<float>();
        //float zFar = cameraBuffer->getUniformBufferValue("zFar").get<float>();

        cameraBuffer->setUniformValue("screenPercentage", Vector4(m_viewport.m_width, m_viewport.m_height, 0, 0));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Camera::asJson(const SerializationContext& context) const
{
    QJsonObject object;
    object.insert("projection", m_renderProjection.asJson());
    object.insert("viewMatrix", m_viewMatrix.asJson());
    object.insert("viewport", m_viewport.asJson());
    object.insert("clearColor", m_clearColor.toVector4g().asJson());
    return object;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    Q_UNUSED(context);

    const QJsonObject& object = json.toObject();

    // Projection and view matrix
    m_renderProjection.loadFromJson(json["projection"]);
    m_viewMatrix.loadFromJson(json["viewMatrix"]);
    if (object.contains("viewport")) {
        m_viewport.loadFromJson(object["viewport"]);
    }

    // Clear color
    if (object.contains("clearColor")) {
        m_clearColor = Color(Vector4(object["clearcolor"]));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Transform * Camera::transform() const
{
    //Transform* transform = m_component->sceneObject()->transform().get();
    return m_transform;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::updateTransform()
{
    Matrix4x4g worldMatrix = m_viewMatrix.inversed();
    transform()->updateWorldMatrix(worldMatrix);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::updateViewMatrix(const Matrix4x4& worldMatrix)
{
    m_viewMatrix = worldMatrix.inversed();
    updateFrustum(CameraUpdateFlag::kViewUpdated);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::updateFrustum(CameraUpdateFlags flags)
{
    Q_UNUSED(flags);
    m_frustum.initialize(m_viewMatrix, m_renderProjection.projectionMatrix());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4g Camera::LookAtRH(const Vector3 & eye, const Vector3 & target, const Vector3 & upIn)
{
    // Normalize up vector, in case it is not already
    Vector3 up = upIn.normalized();

    // See: https://www.3dgep.com/understanding-the-view-matrix/
    // https://gamedev.stackexchange.com/questions/104862/how-to-find-the-up-direction-of-the-view-matrix-with-glm
    Vector3 zaxis = (eye - target).normalized();    // The "forward" vector, negated direction that camera is looking at.

    // Modify up vector if it is in the same direction as the zaxis
    Vector3 xaxisUnscaled = up.cross(zaxis);

    Matrix4x4g viewMatrix = Matrix4x4g::EmptyMatrix();
    if (qFuzzyIsNull(xaxisUnscaled.lengthSquared())) {
        // Use quaternion to avoid gimbal lock and get rotation
        viewMatrix = Quaternion::fromDirection(zaxis, up).toRotationMatrix();
        
        // Get x and y axes from rotation
        Vector3 xaxis = viewMatrix.getRow(0);
        Vector3 yaxis = viewMatrix.getRow(1);

        // Set translation using the "right" vector
        viewMatrix(0, 3) = -xaxis.dot(eye);
        viewMatrix(1, 3) = -yaxis.dot(eye);
        viewMatrix(2, 3) = -zaxis.dot(eye);
        viewMatrix(3, 3) = 1;
    }
    else {
        Vector3 xaxis = xaxisUnscaled.normalized(); // The "right" vector.
        Vector3 yaxis = zaxis.cross(xaxis);         // The "up" vector.

        // Create a 4x4 view matrix from the right, up, forward and eye position vectors
        viewMatrix = std::array<float, 16>{
            xaxis.x(), yaxis.x(), zaxis.x(), 0,
            xaxis.y(), yaxis.y(), zaxis.y(), 0,
            xaxis.z(), yaxis.z(), zaxis.z(), 0,
            -xaxis.dot(eye), -yaxis.dot(eye), -zaxis.dot(eye), 1
        };
    }
        
    return viewMatrix;
}




//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}
