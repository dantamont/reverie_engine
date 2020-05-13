//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_CAMERA_H
#define GB_CAMERA_H

// QT

// Internal
#include "GbComponent.h"
#include "../geometry/GbMatrix.h"
#include "../rendering/view/GbRenderProjection.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class TransformComponent;
class SceneObject;

class RenderProjection;
class ShaderProgram;
namespace GL {
class MainRenderer;
}
class InputHandler;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Settings for viewport
/// @details Currently 36 bytes
class Viewport: Serializable {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    /// @brief Set viewport in GL
    void setGLViewport(const GL::MainRenderer& r) const;

    /// @brief The order of viewport rendering, larger is rendered later (on top)
    int m_depth = 0;

    /// @brief Normal x-coordinate of camera on screen
    double m_xn = 0;

    /// @brief Normal y-coordinate of camera on screen
    double m_yn = 0;

    /// @brief Percent screen width of camera
    double m_width = 1;

    /// @brief Percent screen height of camera
    double m_height = 1;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class Camera
/// @brief Camera view in GL
class Camera : public Object, public Serializable {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    Camera();
    Camera(CameraComponent* component);
    ~Camera();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Viewport settings
    const Viewport& getViewport() const { return m_viewport; }
    Viewport& viewport() { return m_viewport; }

    /// @brief Render projection for this camera
    RenderProjection& renderProjection() { return m_renderProjection; }

    /// @brief The view matrix for this camera
    const Matrix4x4f& getViewMatrix() { return m_viewMatrix; }


    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Conversions between world and widget space
    void widgetToWorldSpace(const Vector3g& widgetSpace, Vector3g& worldSpace) const;
    void worldToWidgetSpace(const Vector3g& worldSpace, Vector3g& widgetSpace) const;

    /// @brief Screen-space to world space conversion
    /// @details should take in a GL-widget space mouse position (e.g., from widgetMousePosition of input handelr)
    // See: http://antongerdelan.net/opengl/raycasting.html
    void widgetToRayDirection(const Vector2g& widgetSpace, Vector3g& outRayDirection,
        const GL::MainRenderer& renderer);

    /// @brief The position of the camera in world-space
    Vector3g eye() const;

    /// @brief Get forward (Z), right (X), and up (Y) vectors for the camera
    Vector3g getForwardVec() const;
    Vector3g getRightVec() const;
    Vector3g getUpVec() const;

    /// @brief Zoom to the given target point
    void zoom(const Vector3g& target, real_g delta);

    /// @brief Pan the camera, i.e., move the target point by a specified delta in screen-space (NDC coordinates)
    /// @details Note that the specified target will change as a result of this operation
    void pan(Vector3g& target, const Vector2g& moveDelta);

    /// @brief Tilts the camera, i.e., rotates on its own body axis
    void tilt(Vector3g& target, const Vector2g& moveDelta);

    /// @brief Translate the camera, given a change in screen-space (NDC coordinates)
    /// @details Note that the specified target will change as a result of this operation
    void translate(Vector3g & target, const Vector2g& moveDelta, real_g speedFactor = real_g(1.0));

    /// @brief Rotate about a target point, given a screen-space mouse change in positiion
    void rotateAboutPoint(const Vector3g& target, const Vector2g& mouseDelta, real_g speedFactor = real_g(1.0));

    /// @brief Set transform using a look-at matrix
    void setLookAt(const Vector3g& eye, const Vector3g& target, const Vector3g& up);

    /// @brief Set viewport in GL
    void setGLViewport(const GL::MainRenderer& r);

    /// @brief Set uniforms related to the camera
    /// @details e.g. View matrix, projection matrix, etc.
    void bindUniforms();
    //void bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "Camera"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::Camera"; }

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class TransformComponent;
    //friend class Gb::SceneObject;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Gets the transform of this camera
    TransformComponent* transform() const;

    /// @brief Update the transform to reflect this camera's view matrix
    void updateTransform();

    /// @brief compute view matrix given a world matrix
    void computeViewMatrix(const Matrix4x4f& worldMatrix);

    /// @brief Calculate look-at matrix given the eye, target, and up vectors
    /// @details Eye is the camera position, target is the point being aimed at, and up is analogous
    /// to the vector from your eyes to the top of your head
    /// See: https://www.3dgep.com/understanding-the-view-matrix/
    static Matrix4x4g lookAtRH(const Vector3g& eye, const Vector3g& target, const Vector3g& up);


    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The camera component that owns this camera
    CameraComponent* m_component;

    /// @brief Viewport settings
    /// @details 36 bytes in size
    Viewport m_viewport;

    /// @brief View matrix (64 bytes)
    Matrix4x4f m_viewMatrix;

    /// @brief Holds information about projection
    /// @details 132 bytes in size
    RenderProjection m_renderProjection;

    /// @}
};


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CameraController
/// @brief Wrapper to control camera movement
class CameraController : public Object, public Serializable {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Movement types
    enum MovementType {
        kZoom,
        kPan,  // Keep camera in the same place, but move target
        kTilt, // Rotate the camera about it's right vector, i.e., change the up vector
        kTranslate, // Translate the entire camera forwards and backwards (dolly), or left/right (truck)
        kRotate // Rotate the camera about a target point
    };

    /// @brief Movement directions
    enum Direction {
        kLeft,
        kRight,
        kUp,
        kDown,
        kForward,
        kBackwards
    };

    /// @brief Profile for the controller's behavior
    struct ControllerProfile: public Serializable{

        virtual QJsonValue asJson() const override;
        virtual void loadFromJson(const QJsonValue& json) override;

        /// @brief Map of enabled movement types
        std::unordered_map<MovementType, bool> m_movementTypes;

        /// @brief The camera look-at point (if mode sets target)
        //Vector3g m_target;

        real_g m_zoomScaling = real_g(-0.25);
        Vector2g m_translateScaling = Vector2g(real_g(1.0), real_g(1.0));
        Vector2g m_rotateScaling = Vector2g(real_g(1.0), real_g(-1.0));
        Vector2g m_panScaling = Vector2g(real_g(-1.0), real_g(1.0));
        Vector2g m_tiltScaling = Vector2g(real_g(-1.0), real_g(1.0));
    };

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    CameraController(CameraComponent* camera);
    ~CameraController();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    ControllerProfile& profile() { return m_profile; }


    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Step the camera controller forward in time
    void step(unsigned long deltaMs);

    /// @brief Step the camera controller forward in time, using the given profile
    void step(unsigned long deltaMs, const ControllerProfile& profile);


    /// @}

     //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

private:

    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Retrieve the input handler for the main GL widdget
    InputHandler& inputHandler() const;

    /// @brief Retrieve the camera for this controller
    Camera& camera();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The camera component that owns this controller
    CameraComponent* m_component;

    /// @brief The current target of the controller
    Vector3g m_target;

    ControllerProfile m_profile;

    /// @}

};



//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CameraComponent
/// @brief Camera view in GL
// TODO: Add ClearMode for handling different or null skyboxes on clear
class CameraComponent: public Component {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    CameraComponent();
    CameraComponent(const std::shared_ptr<SceneObject>& object);
    ~CameraComponent();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Camera
    Camera& camera() { return m_camera; }
    const Camera& camera() const { return m_camera; }

    /// @brief Controller
    CameraController& controller() { return m_controller; }

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override { return 1; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Draw scene
    void drawScene(std::shared_ptr<Scene> scene, const GL::MainRenderer& renderer);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{

    /// @property className
    const char* className() const override { return "CameraComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::CameraComponent"; }

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

    Camera m_camera;
    CameraController m_controller;

    /// @}
};
Q_DECLARE_METATYPE(CameraComponent)


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}


#endif