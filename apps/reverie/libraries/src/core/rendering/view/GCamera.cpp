#include "core/rendering/view/GCamera.h"

#include "core/GCoreEngine.h"
#include "fortress/containers/GColor.h"
#include "core/components/GTransformComponent.h"

#include "core/resource/GResourceCache.h"
#include "core/scene/GSceneObject.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/buffers/GUniformBufferObject.h"

#include "core/loop/GSimLoop.h"
#include "core/debugging/GDebugManager.h"

#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/renderer/GRenderCommand.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "fortress/layer/framework/GFlags.h"
#include "fortress/math/GMath.h"

namespace rev {



// Abstract Camera


AbstractCamera::AbstractCamera()
{    
    // Set camera index
    setIndex();
}

AbstractCamera::~AbstractCamera()
{
    s_deletedIndices.push_back(m_index);
}

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

std::vector<uint32_t> AbstractCamera::s_deletedIndices = {};


uint32_t AbstractCamera::s_cameraCount = 0;






// Camera

Camera::Camera()
{
}

Camera::Camera(const Camera & other) :
    m_frameBuffers(other.m_frameBuffers),
    m_transform(other.m_transform),
    m_viewport(other.m_viewport),
    m_viewMatrix(other.m_viewMatrix),
    m_frustum(other.m_frustum),
    m_renderContext(other.m_renderContext),
    m_clearColor(Vector4(0.55f, 0.6f, 0.93f, 0.0f))
{
    m_renderProjection = other.m_renderProjection;
    m_renderProjection.m_camera = this;

    // Initialize render projection
    m_renderProjection.updateProjection();
    updateBufferUniforms(*m_renderContext);
}

Camera::Camera(TransformInterface* transform,
    RenderContext* renderContext, AliasingType frameBufferFormat,
    FrameBuffer::BufferAttachmentType framebufferStorageType, uint32_t numSamples,
    uint32_t numColorAttachments) :
    m_frustum(m_viewMatrix, m_renderProjection.projectionMatrix()),
    m_renderContext(renderContext),
    m_frameBuffers(renderContext->context(),
        frameBufferFormat,
        framebufferStorageType,
        FBO_FLOATING_POINT_TEX_FORMAT,
        numSamples,
        numColorAttachments),
    m_transform(transform),
    m_renderProjection(this),
    m_clearColor(Vector4(0.55f, 0.6f, 0.93f, 0.0f))
{
    // Initialize render projection
    m_renderProjection.updateProjection();
    updateBufferUniforms(*m_renderContext);
}

Camera::~Camera()
{
}

Camera & Camera::operator=(const Camera & other)
{
    m_transform = other.m_transform;
    m_frameBuffers = other.m_frameBuffers;
    m_viewport = other.m_viewport;
    m_viewMatrix = other.m_viewMatrix;
    m_renderProjection = RenderProjection(this);
    m_frustum = other.m_frustum;
    updateBufferUniforms(*m_renderContext);
    return *this;
}

Vector3 Camera::getRightVec() const
{
    // Get first row of rotation matrix
    return Vector3(m_viewMatrix(0, 0), m_viewMatrix(0, 1), m_viewMatrix(0, 2));
}

Vector3 Camera::getUpVec() const
{
    // Get second row of rotation matrix
    return Vector3(m_viewMatrix(1, 0), m_viewMatrix(1, 1), m_viewMatrix(1, 2));
}

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

float Camera::getDepth(const Vector3 & position)
{
    float depth;

    Vector3 forward = getForwardVec();
    Vector3 distance = position - eye();
    depth = forward.dot(distance);

    return depth;
}

bool Camera::canSee(const CollidingGeometry & geometry) const
{
    return m_frustum.intersects(geometry);
}

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
            Logger::Throw("Error, cannot clear read buffer");
        }
    }
}

void Camera::releaseFrame(FrameBuffer::BindType readOrWrite)
{
    if (readOrWrite == FrameBuffer::BindType::kWrite) {
        m_frameBuffers.writeBuffer().release();
    }
    else {
        m_frameBuffers.readBuffer().release();
    }
}

void Camera::drawFrameBufferQuad(CoreEngine* core)
{
    // Draw Quad
    auto quadShaderProgram = ResourceCache::Instance().getHandleWithName("quad",
        EResourceType::eShaderProgram)->resourceAs<ShaderProgram>();
    m_frameBuffers.writeBuffer().drawQuad(*this, *quadShaderProgram);
}

void Camera::widgetToFrame(const Vector2 & widgetSpace, const OpenGlRenderer & renderer, Real_t & outFrameX, Real_t & outFrameY) const
{
    // Convert to clip coordinates
    m_viewport.widgetToClip(widgetSpace, renderer, outFrameX, outFrameY);

    // Convert to frame coordinates
    clipToFrame(outFrameX, outFrameY, outFrameX, outFrameY);
}

void Camera::clipToFrame(Real_t clipX, Real_t clipY, Real_t & outFrameX, Real_t & outFrameY) const
{
    // Convert to [0, 1], then scale up based on framebuffer dimensions
    // Could also multiple by m_viewport.m_width * widget.width(), since framebuffer always matches viewport size
    outFrameX = (outFrameX + 1.0) / 2.0;
    outFrameX *= m_frameBuffers.readBuffer().width();

    outFrameY = (outFrameY + 1.0) / 2.0;
    outFrameY *= m_frameBuffers.readBuffer().height();
}

//void Camera::widgetToWorldSpace(const Vector2 & widgetSpace, Vector4 & worldSpace) const
//{
//    /// \see widgetToRayDirection
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
//    const Matrix4x4g& inverseViewMatrix = m_transform->worldMatrix();
//    worldSpace = inverseViewMatrix * m_renderProjection.m_inverseProjectionMatrix * Vector4(widgetSpace);
//}

//void Camera::worldToWidgetSpace(const Vector4 & worldSpace, Vector2 & widgetSpace) const
//{
//    widgetSpace = m_renderProjection.projectionMatrix() * m_viewMatrix * Vector4(worldSpace);
//}

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

void Camera::widgetToRayDirection(const Vector2 & widgetSpace, Vector3 & outRayDirection, const OpenGlRenderer& renderer) const
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
    const Matrix4x4g& inverseView = m_transform->worldMatrix();
    outRayDirection = inverseView * coords;
    outRayDirection.normalize();
}

const Vector3& Camera::eye() const
{
    return m_transform->getPosition();
}

void Camera::zoom(const Vector3 & target, Real_t delta)
{
    const Vector3& oldEye = m_transform->getPosition();
    Vector3 eye = target + (oldEye - target) * (delta + 1.0);

    // Set view/world matrices
    setLookAt(eye, target, getUpVec());
}

void Camera::pan(Vector3& target, const Vector2 & moveDelta)
{
    // Get necessary parameters
    const Vector3& eye = m_transform->getPosition();
    Real_t fovY = m_renderProjection.fovY() * Constants::DegToRad;
    Real_t length = Real_t(2.0) * (eye - target).length() * tan(fovY / 2.0);
    Vector3 up = getUpVec();
    Vector3 dirX = up;
    Vector3 dirY = up.cross(target - eye).normalized();
    Real_t aspectRatio = Real_t(m_renderProjection.m_aspectRatio);

    // Set new target position
    target = target + dirY * moveDelta.x() * length * aspectRatio
        + dirX * moveDelta.y() * length;

    // Set view/world matrices
    setLookAt(eye, target, up);
}

void Camera::tilt(Vector3 & target, const Vector2 & moveDelta)
{
    const Vector3& eye = m_transform->getPosition();
    Vector3 up = getUpVec();
    Vector3 forward = getForwardVec();
    Vector3 right = getRightVec();
    up += moveDelta.x() * right;
    up.normalize();

    //forward += moveDelta.y() * up;
    //forward.normalize();
    //Real_t dist = (eye - target).length();
    //target = eye + forward * dist;

    // Set view/world matrices
    setLookAt(eye, target, up);
}

void Camera::translate(Vector3 & target, const Vector3 & moveDelta, Real_t speedFactor)
{
    Vector3 eye = m_transform->getPosition();
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

void Camera::rotateAboutPoint(const Vector3 & target, const Vector2 & mouseDelta, Real_t speedFactor)
{
    // Get rotation axis directions
    Vector3 up = getUpVec();
    Vector3 eye = m_transform->getPosition();

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

//void Camera::rotateAboutAxis(const Vector3 & target, const Vector3 & axis, const Vector2 & mouseDelta, Real_t speedFactor)
//{
//    Logger::Throw("unimplemented");
//}

void Camera::setLookAt(const Vector3 & eye, const Vector3 & target, const Vector3 & up)
{
    // TODO: Wrap up in Transform class, would be convenient
    m_viewMatrix = Camera::LookAtRH(eye, target, up);
    updateTransform();
    updateFrustum(CameraUpdateFlag::kViewUpdated);
    updateBufferUniforms(*m_renderContext);
}

//void Camera::setGLViewport(const OpenGlRenderer& r)
//{
//    // Resize viewport
//    m_viewport.setGLViewport(r);
//    //m_renderProjection.computeProjectionMatrix();
//}

void Camera::setGLViewport()
{
    // Resize viewport
    for (FrameBuffer& fbo : m_frameBuffers.frameBuffers()) {
        m_viewport.setGLViewport(fbo);
    }
    //m_renderProjection.computeProjectionMatrix();
}

void Camera::resizeFrame(uint32_t width, uint32_t height)
{
    // Resize projection
    m_renderProjection.resizeProjection(width, height);

    // Resize framebuffer dimensions
    for (FrameBuffer& fbo : m_frameBuffers.frameBuffers()) {
        m_viewport.resizeFrameBuffer(width, height, fbo);
    }

    updateBufferUniforms(*m_renderContext);
}

void Camera::bindUniforms(const OpenGlRenderer* renderer, ShaderProgram*)
{
    if (Ubo::GetCameraBuffer()) {
        auto cameraBuffer = Ubo::GetCameraBuffer();
        cameraBuffer->setBufferUniformValue<ECameraBufferUniformName::eViewMatrix>(m_bufferUniforms.m_viewMatrix.getData());
        cameraBuffer->setBufferUniformValue<ECameraBufferUniformName::eProjectionMatrix>(m_bufferUniforms.m_projectionMatrix.getData());
        cameraBuffer->setBufferUniformValue<ECameraBufferUniformName::eZNear>(m_bufferUniforms.m_zNear.getData());
        cameraBuffer->setBufferUniformValue<ECameraBufferUniformName::eZFar>(m_bufferUniforms.m_zFar.getData());
        cameraBuffer->setBufferUniformValue<ECameraBufferUniformName::eInvViewMatrix>(m_bufferUniforms.m_invViewMatrix.getData());
        cameraBuffer->setBufferUniformValue<ECameraBufferUniformName::eInvProjectionMatrix>(m_bufferUniforms.m_invProjectionMatrix.getData());
        
        if (renderer) {
            cameraBuffer->setBufferUniformValue<ECameraBufferUniformName::eViewportDimensions>(m_bufferUniforms.m_viewportDimensions.getData());
        }

        cameraBuffer->setBufferUniformValue<ECameraBufferUniformName::eScreenPercentage>(m_bufferUniforms.m_screenPercentage.getData());
    }
}

void to_json(json& orJson, const Camera& korObject)
{
    orJson["projection"] = korObject.m_renderProjection;
    orJson["viewMatrix"] = korObject.m_viewMatrix;
    orJson["viewport"] = korObject.m_viewport;
    orJson["clearColor"] = korObject.m_clearColor.toVector<Real_t, 4>();
}

void from_json(const json& korJson, Camera& orObject)
{
    // Projection and view matrix
    korJson["projection"].get_to(orObject.m_renderProjection);
    korJson["viewMatrix"].get_to(orObject.m_viewMatrix);
    if (korJson.contains("viewport")) {
        korJson["viewport"].get_to(orObject.m_viewport);
    }

    // Clear color
    if (korJson.contains("clearColor")) {
        orObject.m_clearColor = Color(Vector4(korJson.at("clearColor")));
    }

    orObject.updateFrustum((Uint32_t)CameraUpdateFlag::kProjectionUpdated | (Uint32_t)CameraUpdateFlag::kViewUpdated);
    orObject.updateBufferUniforms(*orObject.m_renderContext);
}

void Camera::updateTransform()
{
    Matrix4x4g worldMatrix = m_viewMatrix.inversed();
    m_transform->updateWorldMatrixWithLocal(worldMatrix);
    updateBufferUniforms(*m_renderContext);
}

void Camera::updateViewMatrix(const Matrix4x4& worldMatrix)
{
    m_viewMatrix = worldMatrix.inversed();
    updateFrustum(CameraUpdateFlag::kViewUpdated);
    updateBufferUniforms(*m_renderContext);
}

void Camera::updateBufferUniforms(RenderContext& context)
{
    UniformContainer& uc = context.uniformContainer();
    m_bufferUniforms.m_viewMatrix.setValue(m_viewMatrix, uc);
    m_bufferUniforms.m_projectionMatrix.setValue(m_renderProjection.m_projectionMatrix, uc);
    m_bufferUniforms.m_invViewMatrix.setValue(m_transform->worldMatrix(), uc);
    m_bufferUniforms.m_invProjectionMatrix.setValue(m_renderProjection.m_inverseProjectionMatrix, uc);
    m_bufferUniforms.m_zNear.setValue(m_renderProjection.m_zNear, uc);
    m_bufferUniforms.m_zFar.setValue(m_renderProjection.m_zFar, uc);

    Vector2u viewportDimensions(m_frameBuffers.writeBuffer().width(), m_frameBuffers.writeBuffer().height());
    m_bufferUniforms.m_viewportDimensions.setValue(viewportDimensions, uc);
    m_bufferUniforms.m_screenPercentage.setValue(Vector4(m_viewport.m_width, m_viewport.m_height, 0, 0), uc);
}

void Camera::updateFrustum(CameraUpdateFlags flags)
{
    Q_UNUSED(flags);
    m_frustum.initialize(m_viewMatrix, m_renderProjection.projectionMatrix());
}

void Camera::getWorldFrustumPoints(std::vector<Vector3>& outPoints) const
{
    // The points of the corners of the frustum cube in normalized device coordinates
    static constexpr Uint32_t s_numPoints = 8;
    static const std::array<Vector4, s_numPoints> ndcPoints =
    {{
        // near face
        { 1.F,  1.F, -1.F, 1.F},
        {-1.F,  1.F, -1.F, 1.F},
        { 1.F, -1.F, -1.F, 1.F},
        {-1.F, -1.F, -1.F, 1.F},

        // far face
        { 1.F,  1.F, 1.F, 1.F},
        {-1.F,  1.F, 1.F, 1.F},
        { 1.F, -1.F, 1.F, 1.F},
        {-1.F, -1.F, 1.F, 1.F},
    }};

    const Matrix4x4& inverseViewMatrix = m_transform->worldMatrix();
    const Matrix4x4g inverseProjView = inverseViewMatrix * m_renderProjection.m_inverseProjectionMatrix;

    // Detrmine world space coordinates
    outPoints.reserve(s_numPoints);
    for (int i = 0; i < s_numPoints; i++)
    {
        Vector4 myVec = inverseProjView * ndcPoints[i];
        myVec /= myVec.w();
        outPoints.emplace_back(myVec); // Converts to Vector3
    }
}

Matrix4x4g Camera::LookAtRH(const Vector3 & eye, const Vector3 & target, const Vector3 & upIn)
{
    // Normalize up vector, in case it is not already
    Vector3 up = upIn.normalized();

    /// \see https://www.3dgep.com/understanding-the-view-matrix/
    // https://gamedev.stackexchange.com/questions/104862/how-to-find-the-up-direction-of-the-view-matrix-with-glm
    Vector3 zaxis = (eye - target).normalized();    // The "forward" vector, negated direction that camera is looking at.

    // Modify up vector if it is in the same direction as the zaxis
    Vector3 xaxisUnscaled = up.cross(zaxis);

    Matrix4x4g viewMatrix = Matrix4x4g::EmptyMatrix();
    if (rev::math::fuzzyIsNull(xaxisUnscaled.lengthSquared())) {
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




    
// End namespaces
}
