#pragma once

// Internal
#include "GRenderProjection.h"
#include "GFrameBufferQueue.h"
#include "heave/collisions/GFrustum.h"
#include "GViewport.h"
#include "core/rendering/buffers/GShaderStorageBuffer.h"
#include "core/rendering/lighting/GLightClusterGrid.h"
#include "fortress/containers/GSortingLayer.h"
#include "fortress/containers/GColor.h"
#include <core/geometry/GMousePicking.h>
#include "fortress/layer/framework/GFlags.h"

namespace rev {

class CoreEngine;
class SceneObject;
class PostProcessingChain;
class RenderContext;
class RenderProjection;
class ShaderProgram;
class OpenGlRenderer;
class CubeMapComponent;
class ShaderProgram;
class CollidingGeometry;
class DrawCommand;

template<typename WorldMatrixType>
class TransformTemplate;
typedef TransformTemplate<Matrix4x4> Transform;

enum class CameraType {
    kCamera = 0,
    kSceneCamera,
    kSphereCamera
};


enum class CameraUpdateFlag {
    kViewUpdated = 1 << 0,
    kProjectionUpdated = 1 << 1
};
typedef Flags<CameraUpdateFlag> CameraUpdateFlags;


/// @class AbstractCamera
/// @brief Uninstantiable camera class, to provide flexibility in render commands
class AbstractCamera : public IdentifiableInterface {
public:
    /// @name Constructors/Destructor
    /// @{

    AbstractCamera();
    virtual ~AbstractCamera();

    /// @}

    /// @name Properties
    /// @{

    uint32_t index() const { return m_index; }

    virtual CameraType cameraType() const = 0;

    /// @}

    /// @name Public Methods
    /// @{

    const std::vector<DrawCommand*>& drawCommands() const { return m_drawCommands; }

    /// @brief Resize the camera's framebuffer to accomodate the given width and height
    /// @details Width and height may be refer to either a widget or texture's dimensions
    virtual void resizeFrame(uint32_t width, uint32_t height) = 0;

    /// @brief Set uniforms related to the camera
    /// @details e.g. View matrix, projection matrix, etc.
    virtual void bindUniforms(const OpenGlRenderer* renderer, ShaderProgram* shaderProgram) = 0;

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

    virtual void updateBufferUniforms(RenderContext& renderContext) {}

    /// @}

protected:
    /// @name Protected Methods
    /// @{

    void setIndex();

    /// @}

    /// @name Protected Members
    /// @{

    uint32_t m_index; ///< The index of the camera for use in sort key
    static std::vector<uint32_t> s_deletedIndices; ///< IDs to be used again
    static uint32_t s_cameraCount; ///< Camera count

    std::vector<DrawCommand*> m_drawCommands; ///< The persistent draw commands for the camera

    /// @}
};

/// @class Camera
/// @brief Base camera class
class Camera : public AbstractCamera {
public:

    /// @brief Struct to initialize a framebuffer of the camera
    struct FrameBufferInfo {
        AliasingType m_frameBufferFormat = AliasingType::kMSAA;
        FrameBuffer::BufferAttachmentType m_framebufferColorStorageType = FrameBuffer::BufferAttachmentType::kRbo;
        FrameBuffer::BufferAttachmentType m_framebufferDepthStorageType = FrameBuffer::BufferAttachmentType::kRbo;
        Uint32_t m_numSamples = 16;
        Uint32_t m_numColorAttachments = 1;
    };

    /// @name Static
    /// @{

    /// @brief Calculate look-at matrix given the eye, target, and up vectors
    /// @details Eye is the camera position, target is the point being aimed at, and up is analogous
    /// to the vector from your eyes to the top of your head
    /// See: https://www.3dgep.com/understanding-the-view-matrix/
    static Matrix4x4g LookAtRH(const Vector3& eye, const Vector3& target, const Vector3& up);

    /// @}

    /// @name Constructors/Destructor
    /// @{
    Camera();
    Camera(const Camera& other) = delete;
    Camera(TransformInterface* transform, // Transform that governs the camera
        RenderContext* renderContext, 
        const FrameBufferInfo& info);
    virtual ~Camera();

    /// @}

    /// @name Operators
    /// @{

    Camera& operator=(const Camera& other) = delete;

    /// @}

    /// @name Properties
    /// @{

    MousePicker& mousePicker() { return m_mousePicker; }

    const Color& clearColor() const { return m_clearColor; }
    Color& setClearColor(const Color& color) { return m_clearColor = color; }

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

    /// @brief The world matrix of the camera
    /// @note For cameras, the world matrix is equivalent to the inverse view matrix
    /// @return The world matrix of the camera
    const Matrix4x4& worldMatrix() const;

    /// @brief Get depth of an object (world-space) given its position
    /// @brief Positive is behind camera
    virtual float getDepth(const Vector3& position);

    /// @}

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
    /// @todo Implement, was wrong
    // See: basic_cluster.frag, has these conversions
    //void widgetToWorldSpace(const Vector2& widgetSpace, Vector4& worldSpace) const;
    //void worldToWidgetSpace(const Vector4& worldSpace, Vector2& widgetSpace) const;

    /// @brief Convert from widget pixels to framebuffer (viewport) pixels
    void widgetToFrame(const Vector2& widgetSpace, const OpenGlRenderer& renderer, Real_t& outFrameX, Real_t& outFrameY) const;

    /// @brief Convert from clip space to pixel-coordinates in framebuffer
    void clipToFrame(Real_t clipX, Real_t clipY, Real_t& outFrameX, Real_t& outFrameY) const;

    /// @brief Screen-space to world space conversion
    /// @details should take in a GL-widget space mouse position (e.g., from widgetMousePosition of input handelr)
    // See: http://antongerdelan.net/opengl/raycasting.html
    void widgetToRayDirection(const Vector2& widgetSpace, Vector3& outRayDirection,
        const OpenGlRenderer& renderer) const;

    /// @brief The position of the camera in world-space
    const Vector3& eye() const;

    /// @brief Get forward (Z), right (X), and up (Y) vectors for the camera
    Vector3 getForwardVec() const;
    Vector3 getRightVec() const;
    Vector3 getUpVec() const;

    /// @brief Zoom to the given target point
    void zoom(const Vector3& target, Real_t delta);

    /// @brief Pan the camera, i.e., move the target point by a specified delta in screen-space (NDC coordinates)
    /// @details Note that the specified target will change as a result of this operation
    void pan(Vector3& target, const Vector2& moveDelta);

    /// @brief Tilts the camera, i.e., rotates on its own body axis
    void tilt(Vector3& target, const Vector2& moveDelta);

    /// @brief Translate the camera, given a change in screen-space (NDC coordinates)
    /// @details Note that the specified target will change as a result of this operation
    void translate(Vector3 & target, const Vector3& moveDelta, Real_t speedFactor = Real_t(1.0));

    /// @brief Rotate about a target point, given a screen-space mouse change in positiion
    void rotateAboutPoint(const Vector3& target, const Vector2& mouseDelta, Real_t speedFactor = Real_t(1.0));
    
    /// @brief rotate the camera about an axis
    //void rotateAboutAxis(const Vector3& target, const Vector3& axis, const Vector2& mouseDelta, Real_t speedFactor = Real_t(1.0));

    /// @brief Set transform using a look-at matrix
    void setLookAt(const Vector3& eye, const Vector3& target, const Vector3& up);

    /// @brief Set viewport in GL
    //void setGLViewport(const OpenGlRenderer& r);
    void setGLViewport();

    /// @brief Resize the camera's framebuffer to accomodate the given width and height
    /// @details Width and height may be refer to either a widget or texture's dimensions
    virtual void resizeFrame(uint32_t width, uint32_t height);

    /// @brief Set uniforms related to the camera
    /// @details e.g. View matrix, projection matrix, etc.
    virtual void bindUniforms(const OpenGlRenderer* renderer, ShaderProgram* shaderProgram = nullptr) override;

    /// @brief Bind any buffers associated with the camera
    virtual void bindBuffers() {}

    /// @brief Updates the frustum with the latest view/projection matrices from the camera
    virtual void updateFrustum(CameraUpdateFlags flags) override;

    /// @brief Calculate the points describing the frustum of the camera, in world space
    /// @note Classic OpenGL view and projection matrices are set up so that view space is right-handed, 
    ///   with the camera looking into the -z direction, and the glOrthoand glFrustum functions 
    ///   interpreting nearand far as distances along the view direction
    /// @see https://stackoverflow.com/questions/24085238/how-to-draw-frustum-in-opengl
    /// @see https://stackoverflow.com/questions/44198886/how-to-draw-camera-frustum-using-inverse-view-matrix-in-opengl
    void getWorldFrustumPoints(std::vector<Vector3>& outPoints) const;

    /// @brief Update buffer uniforms with values from this camera
    /// @todo Don't update all of these every time one uniform is changed
    void updateBufferUniforms(RenderContext& renderContext);

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Camera& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Camera& orObject);

    /// @}

protected:
    /// @name Friends
    /// @{
    friend class rev::TransformTemplate<Matrix4x4>;
    friend class TransformComponent;

    /// @}

    /// @name Protected Methods
    /// @{

    /// @brief Update the transform to reflect this camera's view matrix
    void updateTransform();

    /// @brief compute view matrix and frustum given a world matrix
    void updateViewMatrix(const Matrix4x4& worldMatrix);

    /// @}


    /// @name Protected Members
    /// @{

    /// @brief Uniforms to be used for Camera UBO
    struct CameraBufferUniforms {
        StrongTypeUniformData<Matrix4x4> m_viewMatrix; ///< View matrix
        StrongTypeUniformData<Matrix4x4> m_invViewMatrix; ///< Inverse view matrix
        StrongTypeUniformData<Matrix4x4> m_projectionMatrix; ///< Projection matrix
        StrongTypeUniformData<Matrix4x4> m_invProjectionMatrix; ///< Inverse projection matrix
        StrongTypeUniformData<Float32_t> m_zNear; ///< Near clip plane
        StrongTypeUniformData<Float32_t> m_zFar; ///< Far clip plane
        StrongTypeUniformData<Vector2u> m_viewportDimensions; ///< Pixel size dimensions of the viewport
        StrongTypeUniformData<Vector4> m_screenPercentage; ///< Viewport width and height, independent of pixel size
    };
    CameraBufferUniforms m_bufferUniforms; ///< Uniforms to set in the UBO

    RenderContext* m_renderContext{ nullptr }; ///< The render context for the camera
    MousePicker m_mousePicker; ///< The mousepicker for the camera 
    Color m_clearColor; ///< The color to fill the background of the camera when there are no objects occluding it
    TransformInterface* m_transform{ nullptr }; ///< The transform of the camera
    PingPongFrameBuffers m_frameBuffers; ///< The framebuffers that this camera renders onto
    Viewport m_viewport; ///< Viewport settings, 36 bytes in size
    Matrix4x4 m_viewMatrix; ///< View matrix (64 bytes)
    RenderProjection m_renderProjection; ///< Holds information about projection 132 bytes
    Frustum m_frustum; ///< The view frustum for the camera

    /// @}
};

 
// End namespaces
}
