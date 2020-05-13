/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_RENDER_CONTEXT_H
#define GB_RENDER_CONTEXT_H

// QT

// Internal
#include "../GbGLFunctions.h"
#include "../../geometry/GbMatrix.h"
#include "../../mixins/GbLoadable.h"

namespace Gb {  

class CoreEngine;
class CameraComponent;
class Camera;
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
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
    static Matrix4x4f ortho(float left, float right, float bottom, float top, float zNear = -1.0, float zFar = 1.0);

	/// @}

	//---------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{

    RenderProjection(); // For metatype registration
	RenderProjection(View::GLWidget* glWidget);
	~RenderProjection();

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief near plane distance
    double nearClipPlane() const { return m_zNear; }

    /// @brief far plane distance
    double farClipPlane() const { return m_zFar; }

    /// @property Projection matrix
    const Matrix4x4f& projectionMatrix() const { return m_projectionMatrix; }
    const Matrix4x4f& inverseProjectionMatrix() const { return m_projectionMatrix; }

    /// @property Projection Type
    ProjectionType getProjectionType() const { return m_projectionType; }
    void setProjectionType(ProjectionType type);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @property FOV
    void setFOV(double fov);
    double fovX() const;
    double fovY() const;
    
    /// @brief Add a GL widget for this render projection
    void addToGLWidget(View::GLWidget* glWidget);

    /// @brief Updates aspect ratio and projection matrix
    /// @details Called on resize events
    void updateAspectRatio(int width, int height);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}


protected:
    //---------------------------------------------------------------------------------------
    /// @name Friends

    friend class Gb::CameraComponent;
    friend class Gb::Camera;

    /// @}
	//---------------------------------------------------------------------------------------
	/// @name Protected Methods
	/// @{

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
    Matrix4x4f m_projectionMatrix;
    Matrix4x4f m_inverseProjectionMatrix;

	/// @}
};

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}

#endif