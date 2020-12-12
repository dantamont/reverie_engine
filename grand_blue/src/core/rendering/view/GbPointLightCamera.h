//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SPHERE_CAMERA_H
#define GB_SPHERE_CAMERA_H

// QT

// Internal
#include "GbCamera.h"
#include "../../geometry/GbTransform.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////
class RenderContext;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class PointLightCamera
/// @brief Represents a 360 degree capturing camera, used for point light shadow mapping
class PointLightCamera : public AbstractCamera {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    /// @brief Create a sphere camera
    PointLightCamera(Transform* transform, RenderContext* context, size_t cubemapIndex = 0);
    ~PointLightCamera();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    size_t cubemapTextureLayer() const { return m_cubeTextureLayer; }

    virtual CameraType cameraType() const override {
        return CameraType::kSphereCamera;
    }

    const std::array<Camera, 6>& cameras() const { return m_cameras; }
    std::array<Camera, 6>& cameras() { return m_cameras; }

    /// @brief The framebuffer that this camera renders into
    FrameBuffer& frameBuffer() { return *m_frameBuffer; }

    float nearClip() const { return m_nearClip; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Resize the camera's framebuffer to accomodate the given width and height
    /// @details Width and height may be refer to either a widget or texture's dimensions
    virtual void resizeFrame(size_t width, size_t height) override;

    /// @brief Set uniforms related to the camera
    /// @details e.g. View matrix, projection matrix, etc.
    virtual void bindUniforms(const MainRenderer* renderer, ShaderProgram* shaderProgram) override;

    /// @brief Bind any buffers associated with the camera
    virtual void bindBuffers() override;

    virtual void bindTextures() override {}

    /// @brief Bind for drawing
    virtual void bindFrame(bool clear = true) override;

    /// @brief Release from drawing
    virtual void releaseFrame() override;

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
    //---------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    RenderContext* m_context;

    /// @brief the transform associated with the sphere camera and all sub-cameras
    /// @note This differs from the standard camera initialization, which takes a transform pointer that it does not have ownership of
    Transform* m_transform = nullptr;

    /// @brief The framebuffer that this camera renders onto
    //FrameBuffer m_frameBuffer;
    FrameBuffer* m_frameBuffer;

    /// @brief Optional cubemap texture offset for geometry shader
    size_t m_cubeTextureLayer = 0;

    /// @brief The six sub-cameras used to create a cubemap image
    std::array<Camera, 6> m_cameras;

    /// @brief The six light matrices for the camera
    std::array<Matrix4x4g, 6> m_lightMatrices;

    /// @brief The view projection matrices for each of the cameras, used to set uniforms
    //Uniform m_cameraMatrices;

    /// @brief Near plane for the camera
    float m_nearClip;

    /// @brief The frustum for this camera, which is a sphere
    BoundingSphere m_frustum;

    /// @}
};


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}


#endif