//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_CAMERA_COMPONENT_H
#define GB_CAMERA_COMPONENT_H

// QT

// Internal
#include "GComponent.h"
#include "../rendering/view/GSceneCamera.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class TransformComponent;
class SceneObject;
class ShaderProgram;
class MainRenderer;
class InputHandler;
class CubeMapComponent;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
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

        virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;
        virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

        /// @brief Map of enabled movement types
        tsl::robin_map<MovementType, bool> m_movementTypes;

        /// @brief The camera look-at point (if mode sets target)
        //Vector3g m_target;

        real_g m_zoomScaling = real_g(-0.25);
        Vector3 m_translateScaling = Vector3(real_g(1.0), real_g(1.0), real_g(1.0));
        Vector2 m_rotateScaling = Vector2(real_g(1.0), real_g(-1.0));
        Vector2 m_panScaling = Vector2(real_g(-1.0), real_g(1.0));
        Vector2 m_tiltScaling = Vector2(real_g(-1.0), real_g(1.0));
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
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

private:

    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Retrieve the input handler for the main GL widdget
    InputHandler& inputHandler() const;

    /// @brief Retrieve the camera for this controller
    SceneCamera& camera();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The camera component that owns this controller
    CameraComponent* m_component;

    /// @brief The current target of the controller
    Vector3 m_target;

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
    SceneCamera& camera() { return m_camera; }
    const SceneCamera& camera() const { return m_camera; }

    /// @brief Controller
    CameraController& controller() { return m_controller; }

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override { return 1; }

    const Uuid& cubeMapID() const { return m_cubeMapID; }
    void setCubeMapID(const Uuid& cm) { m_cubeMapID = cm; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Draw scene
    void createDrawCommands(Scene& scene, MainRenderer& renderer);
    void createDebugDrawCommands(Scene& scene, MainRenderer& renderer);

    /// @brief Enable this component
    virtual void enable() override;

    /// @brief Disable this component
    virtual void disable() override;
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
    const char* className() const override { return "CameraComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::CameraComponent"; }

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

    SceneCamera m_camera;
    CameraController m_controller;
    Uuid m_cubeMapID = Uuid(false);

    /// @}
};
Q_DECLARE_METATYPE(CameraComponent)


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}


#endif