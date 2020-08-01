//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_CAMERA_H
#define GB_CAMERA_H

// QT

// Internal
#include "GbComponent.h"
#include "../geometry/GbCollisions.h"
#include "../rendering/view/GbRenderProjection.h"
#include "../rendering/view/GbFrameBuffer.h"
#include "../containers/GbSortingLayer.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////
class Camera;
class CoreEngine;
class TransformComponent;
class SceneObject;

class RenderProjection;
class ShaderProgram;
class MainRenderer;
class InputHandler;
class CubeMapComponent;
class FrameBuffer;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Describes the view frustum for a camera
/// @details Currently 36 bytes
class Frustum: public Object {
public:

    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum Plane {
        kLeft,
        kRight,
        kTop,
        kBottom,
        kNear,
        kFar
    };

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    Frustum(Camera* camera);
    ~Frustum();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Initialize the frustum
    void initialize();

    /// @brief Normalize the planes of the frustum
    void normalizePlanes();

    /// @brief Checks whether or not the given geometry is inside the frustum
    bool contains(const CollidingGeometry& geometry) const;
    bool contains(const Vector3g& point) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "Frustum"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::Frustum"; }

    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    void initialize(const Matrix4x4g& viewMatrix, const Matrix4x4g& projectionMatrix);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    Camera* m_camera;

    /// @brief The culling planes for the frustum
    /// @details All planes have normals facing inside the frustum
    std::vector<BoundingPlane> m_planes;

    /// @}
};


//////////////////////////////////////////////////////////////////////////////////////////////////
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
    void setGLViewport(const MainRenderer& r) const;
    void resizeFrameBuffer(const MainRenderer& r, FrameBuffer& fbo) const;

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
class Camera : public Object, public Serializable, private GL::OpenGLFunctions {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Flags for toggling camera settings
    enum CameraOption {
        kFrustumCulling = (1 << 0), // Flag to toggle frustum culling
        kOcclusionCulling = (1 << 1), // Flag to toggle occlusion culling
        kShowAllRenderLayers = (1 << 2), // Flag to force rendering of all render layers, even those unassociated with the camera
        kEnablePostProcessing = (1 << 3), // Flag to enable post-processing effects
        kUseZBufferPass = (1 << 4) // Flag to make a zbuffer pass before rendering scene
    };
    typedef QFlags<CameraOption> CameraOptions;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    //Camera();
    Camera(CameraComponent* component);
    ~Camera();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief The view frustum for the camera
    const Frustum& frustum() const { return m_frustum; }

    /// @brief The framebuffer that this camera renders into
    FrameBuffer& frameBuffer() { return m_frameBuffer; }

    /// @brief Camera options (flags)
    CameraOptions& cameraOptions() { return m_cameraOptions; }

    /// @brief Viewport settings
    const Viewport& getViewport() const { return m_viewport; }
    Viewport& viewport() { return m_viewport; }

    /// @brief Render projection for this camera
    RenderProjection& renderProjection() { return m_renderProjection; }

    /// @brief The view matrix for this camera
    const Matrix4x4f& getViewMatrix() { return m_viewMatrix; }

    size_t index() const { return m_index; }

    /// @brief Get depth of an object (world-space) given its position
    /// @brief Positive is behind camera
    float getDepth(const Vector3g& position);

    std::vector<std::weak_ptr<SortingLayer>>& _renderLayers() { return m_renderLayers; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brie Render a framebuffer quad to screen
    void drawFrameBufferQuad(MainRenderer& r);

    /// @brief Conversions between world and widget space
    void widgetToWorldSpace(const Vector3g& widgetSpace, Vector3g& worldSpace) const;
    void worldToWidgetSpace(const Vector3g& worldSpace, Vector3g& widgetSpace) const;

    /// @brief Screen-space to world space conversion
    /// @details should take in a GL-widget space mouse position (e.g., from widgetMousePosition of input handelr)
    // See: http://antongerdelan.net/opengl/raycasting.html
    void widgetToRayDirection(const Vector2g& widgetSpace, Vector3g& outRayDirection,
        const MainRenderer& renderer);

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
    void translate(Vector3g & target, const Vector3g& moveDelta, real_g speedFactor = real_g(1.0));

    /// @brief Rotate about a target point, given a screen-space mouse change in positiion
    void rotateAboutPoint(const Vector3g& target, const Vector2g& mouseDelta, real_g speedFactor = real_g(1.0));

    /// @brief Set transform using a look-at matrix
    void setLookAt(const Vector3g& eye, const Vector3g& target, const Vector3g& up);

    /// @brief Set viewport in GL
    void setGLViewport(const MainRenderer& r);

    /// @brief Resize the camera's framebuffer to accomodate the given renderer
    void resizeFrameBuffer(const MainRenderer& r);

    /// @brief Set uniforms related to the camera
    /// @details e.g. View matrix, projection matrix, etc.
    void bindUniforms();

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
    friend class CameraComponent;
    //friend class Gb::SceneObject;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Gets the transform of this camera
    TransformComponent* transform() const;

    /// @brief Update the transform to reflect this camera's view matrix
    void updateTransform();

    /// @brief compute view matrix and frustum given a world matrix
    void updateViewMatrix(const Matrix4x4f& worldMatrix);

    /// @brief Updates the frustum with the latest view/projection matrices from the camera
    void updateFrustum();

    /// @brief Calculate look-at matrix given the eye, target, and up vectors
    /// @details Eye is the camera position, target is the point being aimed at, and up is analogous
    /// to the vector from your eyes to the top of your head
    /// See: https://www.3dgep.com/understanding-the-view-matrix/
    static Matrix4x4g lookAtRH(const Vector3g& eye, const Vector3g& target, const Vector3g& up);

    void setIndex();

    std::vector<std::shared_ptr<SortingLayer>> renderLayers();
    std::vector<std::shared_ptr<SortingLayer>> getRenderLayers() const;

    void setDefaultRenderLayers();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The framebuffer that this camera renders onto
    FrameBuffer m_frameBuffer;

    /// @brief Miscellaneous options for the camera
    CameraOptions m_cameraOptions;

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

    /// @brief The view frustum for the camera
    Frustum m_frustum;

    /// @brief The index of the camera for use in sort key
    size_t m_index;

    /// @brief Vector of rendering layers drawn by this camera
    std::vector<std::weak_ptr<SortingLayer>> m_renderLayers;

    /// @brief IDs to be used again
    static std::vector<size_t> s_deletedIndices;

    /// @brief Camera count
    static size_t s_cameraCount;

    /// @}
};
typedef QFlags<Camera::CameraOption> CameraOptions;

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
        Vector3g m_translateScaling = Vector3g(real_g(1.0), real_g(1.0), real_g(1.0));
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

    const UUID& cubeMapID() const { return m_cubeMapID; }
    void setCubeMapID(const UUID& cm) { m_cubeMapID = cm; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Draw scene
    void createDrawCommands(Scene& scene, MainRenderer& renderer);
    void createDebugDrawCommands(Scene& scene, MainRenderer& renderer);

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
    Uuid m_cubeMapID = Uuid(false);

    /// @}
};
Q_DECLARE_METATYPE(CameraComponent)


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}


#endif