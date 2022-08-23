#pragma once

// Internal
#include "core/rendering/GGLFunctions.h"
#include "fortress/containers/math/GMatrix.h"
#include "fortress/types/GLoadable.h"

namespace rev {  

class CoreEngine;
class CameraComponent;
class Camera;
class SceneCamera;
class Shape;
class Mesh;
class Model;
class ShaderProgram;
class OpenGlRenderer;
class GLWidget;

/// @class RenderProjection
/// @brief Used to describe view for a camera
/// @details Currently 132 bytes in size
class RenderProjection {
public:
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
    static void Orthographic(Matrix4x4& out, float left, float right, float bottom, float top, float zNear = -1.0, float zFar = 1.0);

    /// @brief Generate a perspective projection
    /// @param[in] fovDeg FOV in degrees
    static Matrix4x4 Perspective(float fovDeg, float aspectRatio, float zNear = -1.0, float zFar = 1.0);
    static void Perspective(Matrix4x4& out, float fovDeg, float aspectRatio, float zNear = -1.0, float zFar = 1.0);

    /// @brief Linearize a depth
    /// @details This converts a depth from [0, 1] to true world-space depth
    static float LinearizedDepth(float depth, float zNear, float zFar);

	/// @}

	/// @name Constructors/Destructor
	/// @{

    RenderProjection(); 
    RenderProjection(const RenderProjection& other);
    RenderProjection(Camera* camera);
	~RenderProjection();

	/// @}

    /// @name Operators
    /// @{

    RenderProjection& operator=(const RenderProjection& other);

    /// @}

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


    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const RenderProjection& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, RenderProjection& orObject);


    /// @}


protected:
    /// @name Friends

    friend class rev::CameraComponent;
    friend class rev::Camera;
    friend class rev::SceneCamera;

    /// @}

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

	/// @name Protected Members
	/// @{

    ProjectionType m_projectionType; ///< Type of mathematical projection
    Camera* m_camera{ nullptr }; ///< Pointer to camera that owns this projection

    double m_fov; ///< Horizontal field of view angle (how wide view is) in degrees
    double m_zNear; ///< near plane distance
    double m_zFar{ -1 }; ///< far plane distance

    /// @brief Dimensions for orthographic projection 
    double m_left{ -1 };
    double m_right{ -1 };
    double m_bottom{ -1 };
    double m_top{ -1 };

    double m_aspectRatio{ 1.0 }; ///< aspect ratio (width/height) of the GL widget
    Matrix4x4 m_projectionMatrix; ///< 64-byte projection matrix
    Matrix4x4 m_inverseProjectionMatrix;

	/// @}
};

    
// End namespaces
}
