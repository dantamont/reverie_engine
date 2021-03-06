/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_RENDER_PROJECTION_H
#define GB_RENDER_PROJECTION_H

// QT

// Internal
#include "../GGLFunctions.h"
#include "../../geometry/GMatrix.h"
#include "../../mixins/GLoadable.h"

namespace rev {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class CameraComponent;
class Camera;
class SceneCamera;
class Shape;
class Mesh;
class Model;
class ShaderProgram;
class MainRenderer;

namespace View {
class GLWidget;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////

/// @class RenderProjection
/// @brief Used to describe view for a camera
/// @details Currently 132 bytes in size
class RenderProjection: public Serializable {
public:
	//---------------------------------------------------------------------------------------
	/// @name Static
	/// @{

    enum ProjectionType {
        kPerspective=0,
        kOrthographic
    };

    /// @brief Generate an orthographic projection
    /// See: http://learnwebgl.brown37.net/08_projections/projections_ortho.html
    /// @details Maps from left-right to [-1, 1] Along the NCD axis
    static Matrix4x4 Orthographic(float left, float right, float bottom, float top, float zNear = -1.0, float zFar = 1.0);

    /// @brief Generate a perspective projection
    /// @param[in] fovDeg FOV in degrees
    static Matrix4x4 Perspective(float fovDeg, float aspectRatio, float zNear = -1.0, float zFar = 1.0);

    /// @brief Linearize a depth
    /// @details This converts a depth from [0, 1] to true world-space depth
    static float LinearizedDepth(float depth, float zNear, float zFar);

	/// @}

	//---------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{

    RenderProjection(); 
    RenderProjection(const RenderProjection& other);
    RenderProjection(Camera* camera);
	~RenderProjection();

	/// @}

    //---------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    RenderProjection& operator=(const RenderProjection& other);

    /// @}

    ////---------------------------------------------------------------------------------------
    ///// @name Non-copyable
    ///// @{
    //RenderProjection& operator=(const RenderProjection& other) = delete;
    //RenderProjection(const RenderProjection&) = delete;
    ///// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    double aspectRatio() const { return m_aspectRatio; }
    void setAspectRatio(double r, bool update = true) { 
        m_aspectRatio = r; 
        if (update) {
            updateProjection();
        }
    }

    /// @brief near plane distance
    double nearClipPlane() const { return m_zNear; }
    void setNearClipPlane(double nearClip) { 
        m_zNear = nearClip; 
        updateProjection();
    }

    /// @brief far plane distance
    double farClipPlane() const { return m_zFar; }
    void setFarClipPlane(double farClip) { 
        m_zFar = farClip; 
        updateProjection();
    }

    /// @property Projection matrix
    const Matrix4x4& projectionMatrix() const { return m_projectionMatrix; }
    Matrix4x4& projectionMatrix() { return m_projectionMatrix; }
    const Matrix4x4& inverseProjectionMatrix() const { return m_inverseProjectionMatrix; }

    /// @property Projection Type
    ProjectionType getProjectionType() const { return m_projectionType; }
    void setProjectionType(ProjectionType type);

    // For orthographic projection
    double leftBound() const { return m_left; }
    void setLeftBound(double left) {
        m_left = left;        
        updateProjection();
    }

    double topBound() const { return m_top; }
    void setTopBound(double top) {
        m_top = top;
        updateProjection();
    }

    double rightBound() const { return m_right; }
    void setRightBound(double right) {
        m_right = right;
        updateProjection();
    }

    double bottomBound() const { return m_bottom; }
    void setBottomBound(double bottom) {
        m_bottom = bottom;
        updateProjection();
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Set to a perspective projection
    void setPerspective(float fov, float aspectRatio, float nearClip, float farClip);

    /// @brief Set to an orthographic projection
    void setOrthographic(float left, float right, float bottom, float top, float zNear = -1.0, float zFar = 1.0);

    /// @brief linearize the given depth value
    float linearizeDepth(float depth) const;

    /// @property FOV
    void setFOV(double fov);
    double fovX() const;
    double fovY() const;
    
    /// @brief Updates aspect ratio and projection matrix
    /// @details Called on resize events
    void resizeProjection(int width, int height);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}


protected:
    //---------------------------------------------------------------------------------------
    /// @name Friends

    friend class rev::CameraComponent;
    friend class rev::Camera;
    friend class rev::SceneCamera;

    /// @}
	//---------------------------------------------------------------------------------------
	/// @name Protected Methods
	/// @{

    /// @brief Update projection-dependent attributes
    void updateProjection();

    /// @brief Compute the projection matrix for this render context
    /// @note See: http://www.songho.ca/opengl/gl_projectionmatrix.html
    /// https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix
    /// https://stackoverflow.com/questions/6060169/is-this-a-correct-perspective-fov-matrix
    void computeProjectionMatrix();

    /// @brief Convert between horizontal and vertical FOV
    static double hfov_to_vfov(double hfov, double aspect);
    static double vfov_to_hfov(double vfov, double aspect);

	/// @}

	//---------------------------------------------------------------------------------------
	/// @name Protected Members
	/// @{

    /// @brief Type of mathematical projection
    ProjectionType m_projectionType;

    /// @brief Pointer to camera that owns this projection
    Camera* m_camera;

    /// @brief Horizontal field of view angle (how wide view is) in degrees
    double m_fov;

    /// @brief near plane distance
    double m_zNear;

    /// @brief far plane distance
    double m_zFar;

    /// @brief Dimensions for orthographic projection 
    double m_left;
    double m_right;
    double m_bottom;
    double m_top;

    /// @brief aspect ratio (width/height) of the GL widget
    double m_aspectRatio;

    /// @brief projection matrix
    /// @details 64 bytes
    Matrix4x4 m_projectionMatrix;
    Matrix4x4 m_inverseProjectionMatrix;

	/// @}
};

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}

#endif