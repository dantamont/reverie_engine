#pragma once

// Internal
#include "GComponent.h"
#include "core/rendering/view/GSceneCamera.h"

namespace rev {

class GSimulationPlayMode;
class CoreEngine;
class TransformComponent;
class SceneObject;
class ShaderProgram;
class OpenGlRenderer;
class InputHandler;
class CubeMapComponent;

/// @brief Profile for the controller's behavior
struct ControllerProfile {

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const ControllerProfile& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, ControllerProfile& orObject);

    /// @brief Map of enabled movement types
    tsl::robin_map<int/*MovementType*/, bool> m_movementTypes;

    /// @brief The camera look-at point (if mode sets target)
    //Vector3g m_target;

    Real_t m_zoomScaling = Real_t(-0.25);
    Vector3 m_translateScaling = Vector3(Real_t(1.0), Real_t(1.0), Real_t(1.0));
    Vector2 m_rotateScaling = Vector2(Real_t(1.0), Real_t(-1.0));
    Vector2 m_panScaling = Vector2(Real_t(-1.0), Real_t(1.0));
    Vector2 m_tiltScaling = Vector2(Real_t(-1.0), Real_t(1.0));
};



/// @class CameraController
/// @brief Wrapper to control camera movement
class CameraController {
public:
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

    /// @}

    /// @name Constructors/Destructor
    /// @{

    CameraController(CameraComponent* camera);
    ~CameraController();

    /// @}

    /// @name Properties
    /// @{

    ControllerProfile& profile() { return m_profile; }


    /// @}

    /// @name Public methods
    /// @{

    /// @brief Step the camera controller forward in time
    void step(double deltaSec);

    /// @brief Step the camera controller forward in time, using the given profile
    void step(double deltaSec, const ControllerProfile& profile);


    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const CameraController& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, CameraController& orObject);


    /// @}

private:

    /// @name Protected Methods
    /// @{

    /// @brief Retrieve the input handler for the main GL widdget
    InputHandler& inputHandler() const;

    /// @brief Retrieve the camera for this controller
    SceneCamera& camera();

    /// @}

    /// @name Protected Members
    /// @{

    /// @brief The camera component that owns this controller
    CameraComponent* m_component;

    /// @brief The current target of the controller
    Vector3 m_target;

    ControllerProfile m_profile;

    /// @}

};



/// @class CameraComponent
/// @brief Camera view in GL
// TODO: Add ClearMode for handling different or null skyboxes on clear
class CameraComponent: public Component {
public:
    /// @name Constructors/Destructor
    /// @{

    CameraComponent();
    CameraComponent(const std::shared_ptr<SceneObject>& object);
    ~CameraComponent();

    /// @}

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

    /// @name Public Methods
    /// @{

    /// @brief Draw scene
    void retrieveDrawCommands(Scene& scene, OpenGlRenderer& renderer, const GSimulationPlayMode& playMode);

    /// @brief Enable this component
    virtual void enable() override;

    /// @brief Disable this component
    virtual void disable() override;

    /// @brief Get sorting layers to use for rendering
    std::vector<SortingLayer> getLocalSortingLayers(const GSimulationPlayMode& playMode) const;

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const CameraComponent& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, CameraComponent& orObject);


    /// @}

protected:

    /// @brief Create command to draw skybox
    void createSkyboxDrawCommand(Scene& scene, OpenGlRenderer& renderer, const SortingLayer& currentLayer);

    /// @name Protected Members
    /// @{

    SceneCamera m_camera;
    CameraController m_controller;
    Uuid m_cubeMapID = Uuid(false);

    /// @}
};


    
// End namespaces
}
