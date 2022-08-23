#pragma once

// Internal
#include "GCamera.h"
#include "fortress/containers/math/GTransform.h"

namespace rev {

class RenderContext;

/// @class PointLightCamera
/// @brief Represents a 360 degree capturing camera, used for point light shadow mapping
class PointLightCamera : public AbstractCamera {
public:

    /// @brief Container for view-projection matricies used by the point light camera
    struct ViewProjectionMatrices {
        /// @brief Update the view matrices with a new position
        void updateViewMatrices(const Vector3& cameraPosition);

        /// @brief Update the view projection matrices from the view and projection matrices;
        void updateViewProjectionMatrices();

        std::array<Matrix4x4g, 6> m_viewMatrices;
        std::array<Matrix4x4g, 6> m_projectionMatrices;
        std::array<Matrix4x4g, 6> m_viewProjectionMatrices; ///< The six light view projection matrices for the camera

    protected:
        static const std::array<Vector3, 6> s_lookAtDirs; ///< @todo Make constexpr, Vector3 needs a constexpr constructor
        static const std::array<Vector3, 6> s_upDirs;  ///< @todo Make constexpr, Vector3 needs a constexpr constructor
    };

public:
    /// @name Constructors/Destructor
    /// @{

    /// @brief Create a sphere camera
    PointLightCamera(TransformInterface* transform, RenderContext* context, uint32_t pointLightIndex = 0);
    ~PointLightCamera();

    /// @}

    /// @name Properties
    /// @{

    uint32_t pointLightIndex() const { return m_pointLightIndex; }

    virtual CameraType cameraType() const override {
        return CameraType::kSphereCamera;
    }

    /// @brief The framebuffer that this camera renders into
    FrameBuffer& frameBuffer() { return *m_frameBuffer; }

    ViewProjectionMatrices& viewProjectionMatrices() { return m_lightMatrices; }

    float nearClip() const { return m_nearClip; }

    void setFarClip(Float32_t farClip) { m_frustum.setRadius(farClip); }

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Resize the camera's framebuffer to accomodate the given width and height
    /// @details Width and height may be refer to either a widget or texture's dimensions
    virtual void resizeFrame(uint32_t width, uint32_t height) override;

    /// @brief Set uniforms related to the camera
    /// @details e.g. View matrix, projection matrix, etc.
    virtual void bindUniforms(const OpenGlRenderer* renderer, ShaderProgram* shaderProgram) override;

    /// @brief Bind any buffers associated with the camera
    virtual void bindBuffers() override;

    virtual void bindTextures() override {}

    /// @brief Bind for drawing
    virtual void bindFrame(FrameBuffer::BindType readOrWrite = FrameBuffer::BindType::kWrite, bool clear = true) override;

    /// @brief Release from drawing
    virtual void releaseFrame(FrameBuffer::BindType readOrWrite = FrameBuffer::BindType::kWrite) override;

    /// @brief The view frustum for the camera, which is a sphere
    /// @details The geometry used for culling what is in the camera's view
    virtual const CollidingGeometry& frustum() const override {
        return m_frustum;
    }

    /// @brief Updates the frustum with the latest view/projection matrices from the camera
    virtual void updateFrustum(CameraUpdateFlags flags) override;

    /// @brief Get depth of an object (world-space) given its position
    /// @brief Will always be negative, since always in front of camera
    virtual float getDepth(const Vector3& position) override;

    /// @}

protected:
    /// @name Protected Members
    /// @{

    struct PointLightUniforms {
        UniformData m_pointLightIndex;
    };
    PointLightUniforms m_uniforms;

    RenderContext* m_context{ nullptr }; ///< The render context used by the camera
    TransformInterface* m_transform{ nullptr }; ///< the transform associated with the sphere camera and all sub-cameras
    FrameBuffer* m_frameBuffer{ nullptr }; ///< The framebuffer that this camera renders onto
    ViewProjectionMatrices m_lightMatrices; ///< The matricies used by this camera
    Uint32_t m_pointLightIndex{ 0 }; ///< Cubemap texture offset for shader buffer access
    Float32_t m_nearClip{ -1.f }; ///< Near plane for the camera
    BoundingSphere m_frustum; ///< The frustum for this camera, which is a sphere

    /// @}
};

    
// End namespaces
}
