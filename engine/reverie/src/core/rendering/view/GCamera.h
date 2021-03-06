//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_CAMERA_H
#define GB_CAMERA_H

// QT

// Internal
#include "GRenderProjection.h"
#include "GFrameBufferQueue.h"
#include "GFrustum.h"
#include "GViewport.h"
#include "../buffers/GShaderStorageBuffer.h"
#include "../lighting/GLightClusterGrid.h"
#include "../../containers/GSortingLayer.h"
#include "../../containers/GColor.h"
#include <core/geometry/GMousePicking.h>

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class Transform;
class SceneObject;
class PostProcessingChain;
class RenderProjection;
class ShaderProgram;
class MainRenderer;
class CubeMapComponent;
class ShaderProgram;
class CollidingGeometry;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Flags for toggling camera settings
enum class CameraOption {
    kFrustumCulling = (1 << 0), // Flag to toggle frustum culling
    kOcclusionCulling = (1 << 1), // Flag to toggle occlusion culling
    kShowAllRenderLayers = (1 << 2), // Flag to force rendering of all render layers, even those unassociated with the camera
    kEnablePostProcessing = (1 << 3), // Flag to enable post-processing effects
    kUseZBufferPass = (1 << 4) // Flag to make a zbuffer pass before rendering scene
};
typedef QFlags<CameraOption> CameraOptions;

//////////////////////////////////////////////////////////////////////////////////////////////////
enum class CameraType {
    kCamera = 0,
    kSceneCamera,
    kSphereCamera
};

//////////////////////////////////////////////////////////////////////////////////////////////////
enum class CameraUpdateFlag {
    kViewUpdated = 1 << 0,
    kProjectionUpdated = 1 << 1
};
typedef QFlags<CameraUpdateFlag> CameraUpdateFlags;

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AbstractCamera
/// @brief Uninstantiable camera class, to provide flexibility in render commands
class AbstractCamera : public Object, public Identifiable {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    AbstractCamera();
    virtual ~AbstractCamera();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    size_t index() const { return m_index; }

    virtual CameraType cameraType() const = 0;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Resize the camera's framebuffer to accomodate the given width and height
    /// @details Width and height may be refer to either a widget or texture's dimensions
    virtual void resizeFrame(size_t width, size_t height) = 0;

    /// @brief Set uniforms related to the camera
    /// @details e.g. View matrix, projection matrix, etc.
    virtual void bindUniforms(const MainRenderer* renderer, ShaderProgram* shaderProgram) = 0;

    /// @brief Bind any textures needed for the camera
    virtual void bindTextures() = 0;

    /// @brief Bind any buffers associated with the camera
    virtual void bindBuffers() = 0;

    /// @brief Bind for drawing
    virtual void bindFrame(FrameBuffer::BindType readOrWrite, bool clear) = 0;

    /// @brief Release from drawing
    virtual void releaseFrame(FrameBuffer::BindType readOrWrite) = 0;

    /// @brief Update the camera's frustum;
    virtual void updateFrustum(CameraUpdateFlags flags) = 0;

    /// @brief The view frustum for the camera
    /// @details The geometry used for culling what is in the camera's view
    virtual const CollidingGeometry& frustum() const = 0;

    /// @brief Get depth of an object (world-space) given its position
    /// @brief Positive is behind camera
    virtual float getDepth(const Vector3& position) = 0;

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    void setIndex();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The index of the camera for use in sort key
    size_t m_index;

    /// @brief IDs to be used again
    static std::vector<size_t> s_deletedIndices;

    /// @brief Camera count
    static size_t s_cameraCount;

    /// @}
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class Camera
/// @brief Base camera class
class Camera : public AbstractCamera, public Serializable {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Calculate look-at matrix given the eye, target, and up vectors
    /// @details Eye is the camera position, target is the point being aimed at, and up is analogous
    /// to the vector from your eyes to the top of your head
    /// See: https://www.3dgep.com/understanding-the-view-matrix/
    static Matrix4x4g LookAtRH(const Vector3& eye, const Vector3& target, const Vector3& up);


    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Camera();
    Camera(const Camera& other);
    Camera(Transform* transform, // Transform that governs the camera
        QOpenGLContext* glContext, 
        AliasingType frameBufferFormat = AliasingType::kMSAA,
        FrameBuffer::BufferAttachmentType framebufferStorageType = FrameBuffer::BufferAttachmentType::kRBO,
        size_t numSamples = 16,
        size_t numColorAttachments = 1);
    virtual ~Camera();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    Camera& operator=(const Camera& other);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    MousePicker& mousePicker() { return m_mousePicker; }

    Color& clearColor() { return m_clearColor; }

    virtual CameraType cameraType() const {
        return CameraType::kCamera;
    }

    /// @brief The view frustum for the camera
    virtual const CollidingGeometry& frustum() const override { return m_frustum; }

    /// @brief The framebuffers that this camera renders onto
    /// @details The queue manages which framebuffers are for read/write every frame
    PingPongFrameBuffers& frameBuffers() { return m_frameBuffers; }
    const PingPongFrameBuffers& frameBuffers() const { return m_frameBuffers; }

    /// @brief Viewport settings
    const Viewport& getViewport() const { return m_viewport; }
    Viewport& viewport() { return m_viewport; }

    /// @brief Render projection for this camera
    RenderProjection& renderProjection() { return m_renderProjection; }
    const RenderProjection& renderProjection() const { return m_renderProjection; }

    /// @brief The view matrix for this camera
    const Matrix4x4& getViewMatrix() const { return m_viewMatrix; }
    void setViewMatrix(const Matrix4x4& viewMatrix)  { 
        m_viewMatrix = viewMatrix; 
    }
    void setViewMatrix(Matrix4x4&& viewMatrix) {
        m_viewMatrix = std::move(viewMatrix);
    }

    /// @brief Get depth of an object (world-space) given its position
    /// @brief Positive is behind camera
    virtual float getDepth(const Vector3& position);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Whether or not the camera can see the specified colliding geometry
    bool canSee(const CollidingGeometry& geometry) const;

    virtual void bindTextures() override {}

    /// @brief Bind for drawing or read
    virtual void bindFrame(FrameBuffer::BindType readOrWrite, bool clear) override;

    /// @brief Release from drawing
    virtual void releaseFrame(FrameBuffer::BindType readOrWrite) override;

    /// @brief Render a framebuffer quad to screen
    virtual void drawFrameBufferQuad(CoreEngine* core);

    /// @brief Conversions between world and widget space
    // TODO: Implement, was wrong
    // See: basic_cluster.frag, has these conversions
    //void widgetToWorldSpace(const Vector2& widgetSpace, Vector4& worldSpace) const;
    //void worldToWidgetSpace(const Vector4& worldSpace, Vector2& widgetSpace) const;

    /// @brief Convert from widget pixels to framebuffer (viewport) pixels
    void widgetToFrame(const Vector2& widgetSpace, const MainRenderer& renderer, real_g& outFrameX, real_g& outFrameY) const;

    /// @brief Convert from clip space to pixel-coordinates in framebuffer
    void clipToFrame(real_g clipX, real_g clipY, real_g& outFrameX, real_g& outFrameY) const;

    /// @brief Screen-space to world space conversion
    /// @details should take in a GL-widget space mouse position (e.g., from widgetMousePosition of input handelr)
    // See: http://antongerdelan.net/opengl/raycasting.html
    void widgetToRayDirection(const Vector2& widgetSpace, Vector3& outRayDirection,
        const MainRenderer& renderer) const;

    /// @brief The position of the camera in world-space
    const Vector3& eye() const;

    /// @brief Get forward (Z), right (X), and up (Y) vectors for the camera
    Vector3 getForwardVec() const;
    Vector3 getRightVec() const;
    Vector3 getUpVec() const;

    /// @brief Zoom to the given target point
    void zoom(const Vector3& target, real_g delta);

    /// @brief Pan the camera, i.e., move the target point by a specified delta in screen-space (NDC coordinates)
    /// @details Note that the specified target will change as a result of this operation
    void pan(Vector3& target, const Vector2& moveDelta);

    /// @brief Tilts the camera, i.e., rotates on its own body axis
    void tilt(Vector3& target, const Vector2& moveDelta);

    /// @brief Translate the camera, given a change in screen-space (NDC coordinates)
    /// @details Note that the specified target will change as a result of this operation
    void translate(Vector3 & target, const Vector3& moveDelta, real_g speedFactor = real_g(1.0));

    /// @brief Rotate about a target point, given a screen-space mouse change in positiion
    void rotateAboutPoint(const Vector3& target, const Vector2& mouseDelta, real_g speedFactor = real_g(1.0));
    
    /// @brief rotate the camera about an axis
    //void rotateAboutAxis(const Vector3& target, const Vector3& axis, const Vector2& mouseDelta, real_g speedFactor = real_g(1.0));

    /// @brief Set transform using a look-at matrix
    void setLookAt(const Vector3& eye, const Vector3& target, const Vector3& up);

    /// @brief Set viewport in GL
    //void setGLViewport(const MainRenderer& r);
    void setGLViewport();

    /// @brief Resize the camera's framebuffer to accomodate the given width and height
    /// @details Width and height may be refer to either a widget or texture's dimensions
    virtual void resizeFrame(size_t width, size_t height);

    /// @brief Set uniforms related to the camera
    /// @details e.g. View matrix, projection matrix, etc.
    virtual void bindUniforms(const MainRenderer* renderer, ShaderProgram* shaderProgram = nullptr) override;

    /// @brief Bind any buffers associated with the camera
    virtual void bindBuffers() {}

    /// @brief Updates the frustum with the latest view/projection matrices from the camera
    virtual void updateFrustum(CameraUpdateFlags flags) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "SceneCamera"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::SceneCamera"; }

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Friends
    /// @{
    friend class Transform;
    friend class TransformComponent;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{


    /// @brief Gets the transform of this camera
    Transform* transform() const;

    /// @brief Update the transform to reflect this camera's view matrix
    void updateTransform();

    /// @brief compute view matrix and frustum given a world matrix
    void updateViewMatrix(const Matrix4x4& worldMatrix);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The mousepicker for the camera 
    MousePicker m_mousePicker;

    /// @brief The color to fill the background of the camera when there are no objects occluding it
    Color m_clearColor;

    /// @brief The transform of the camera
    Transform* m_transform;

    /// @brief The framebuffers that this camera renders onto
    PingPongFrameBuffers m_frameBuffers;

    /// @brief Viewport settings
    /// @details 36 bytes in size
    Viewport m_viewport;

    /// @brief View matrix (64 bytes)
    Matrix4x4 m_viewMatrix;

    /// @brief Holds information about projection
    /// @details 132 bytes in size
    RenderProjection m_renderProjection;

    /// @brief The view frustum for the camera
    Frustum m_frustum;

    /// @}
};


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}


#endif