/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_COMPONENT_WRAPPERS_H
#define GB_COMPONENT_WRAPPERS_H

// External

// QT

// Internal
#include "../GbPythonScript.h"
#include "../../geometry/GbEulerAngles.h"
#include "../../geometry/GbQuaternion.h"
#include "../../geometry/GbMatrix.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef GB_USE_DOUBLE
typedef double real_g;
#else
typedef float real_g;
#endif

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class InputHandler;
class KeyHandler;
class MouseHandler;
class TransformComponent;
class SceneObject;
class PyTransformComponent;
class TranslationComponent;
class RotationComponent;
class ScaleComponent;
class CameraComponent;
class ShaderComponent;
class CharControlComponent;
class LightComponent;
class AudioSourceComponent;
class BoneAnimationComponent;
class Scene;
class Scenario;
class ShaderProgram;
class ResourceCache;
struct Uniform;
class CustomEvent;
class Transform;

/////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs and Usings
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Function definitions
/////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyShaderComponent
/// @brief Wrapper for a shader component
class PyShaderComponent : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyShaderComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyShaderComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyShaderComponent"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Add a constructor
    ShaderComponent* new_ShaderComponent(SceneObject* so);

    /// @brief Add a destructor
    void delete_ShaderComponent(ShaderComponent* r);

    /// @brief Obtain TransformComponent as JSON
    QJsonValue as_json_str(ShaderComponent* r) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(ShaderComponent* r);

    /// @}
private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyBoneAnimation
/// @brief Wrapper for a bone animation component
class PyBoneAnimation : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyBoneAnimation();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyBoneAnimation"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyBoneAnimation"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Add a constructor
    BoneAnimationComponent* new_BoneAnimationComponent(SceneObject* so);

    /// @brief Add a destructor
    void delete_BoneAnimationComponent(BoneAnimationComponent* c);

    /// @brief Obtain as JSON
    QJsonValue as_json_str(BoneAnimationComponent* bc) const;

    /// @brief Obtain the object for the camera
    SceneObject* scene_object(BoneAnimationComponent* bc) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(BoneAnimationComponent* bc);

    /// @}
private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyCamera
/// @brief Wrapper for a camera component
class PyCamera : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyCamera();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyCamera"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyCamera"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Add a constructor
    CameraComponent* new_CameraComponent(SceneObject* object);

    /// @brief Add a destructor
    void delete_CameraComponent(CameraComponent* c);

    /// @brief Obtain as JSON
    QJsonValue as_json_str(CameraComponent* c) const;

    /// @brief Obtain the object for the camera
    SceneObject* scene_object(CameraComponent* c) const;

    /// @brief Rotate the camera about a point
    void rotate_about_point(CameraComponent* c, Vector3* point, Vector2* mouseDelta, double speedFactor = 1.0);
    //void rotate_about_axis(CameraComponent* c, Vector3* point, Vector3* axis, Vector2* mouseDelta, double speedFactor = 1.0);

    /// @brief Look at the given transform from the camera
    void look_at(CameraComponent* c, Transform* transform, const Vector3* up = nullptr);
    void look_at(CameraComponent* c, Vector3* pos, const Vector3* up = nullptr);

    /// @brief Get camera directional vectors
    Vector3* forward(CameraComponent* c);
    Vector3* right(CameraComponent* c);
    Vector3* up(CameraComponent* c);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Set viewport size
    void set_viewport(CameraComponent* c, double x, double y, double w, double h);

    /// @brief Set viewport order
    void set_depth(CameraComponent* c, int x);

    /// @brief Set camera FOV in degrees
    void set_fov(CameraComponent* c, double fov);

    /// @brief Wraps __str__
    QString py_toString(CameraComponent* c);

    /// @}
private:

};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyCharacterController
/// @brief Wrapper for a camera component
class PyCharacterController : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyCharacterController();
    ~PyCharacterController();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyCharacterController"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyCharacterController"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Add a constructor
    CharControlComponent* new_CharControlComponent(SceneObject* object);

    /// @brief Add a destructor
    void delete_CharControlComponent(CharControlComponent* c);

    /// @brief Move the character by a specified amount
    void move(CharControlComponent * c, const Vector3* disp) const;

    /// @brief Returns radius, which will be negative if this is not a capsule component
    float get_radius(CharControlComponent * c) const;
    bool set_radius(CharControlComponent * c, float rad) const;

    /// @brief Returns height
    float get_height(CharControlComponent * c) const;
    bool set_height(CharControlComponent * c, float height) const;

    void set_height_offset(CharControlComponent * c, float offset) const;
    void set_initial_position(CharControlComponent * c, const Vector3* pos) const;

    /// @brief Obtain as JSON
    QJsonValue as_json_str(CharControlComponent* c) const;

    /// @brief Obtain the object for the camera
    SceneObject* scene_object(CharControlComponent* c) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(CharControlComponent* c);

    /// @}
private:

};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyTransformComponent
/// @brief Wrapper for a TransformComponent component
class PyTransformComponent : public QObject, public Object {
    Q_OBJECT
public:
    //----------------------------------------------------------l-------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyTransformComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyTransformComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyTransformComponent"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Obtain TransformComponent as JSON
    QJsonValue as_json_str(TransformComponent* t) const;

    /// @brief Obtain translation component
    TranslationComponent* translation(TransformComponent* t);

    /// @brief Obtain rotational component
    RotationComponent* rotation(TransformComponent* r);

    /// @brief Obtain scale component
    ScaleComponent* scale(TransformComponent* s);

    /// @brief Obtain the object for the TransformComponent
    SceneObject* scene_object(TransformComponent* t) const;

    /// @brief Convenience method to get position, saves a lot of trouble
    const Vector3* local_pos(TransformComponent* t) const;
    Vector3* pos(TransformComponent* t) const;
    void set_local_pos(TransformComponent* t, Vector3* pos) const;
    void set_pos(TransformComponent* t, Vector3* pos) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(TransformComponent* t);

    /// @}
private:

};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyTranslationComponent
/// @brief Wrapper for a translation component
class PyTranslationComponent : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyTranslationComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyTranslationComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyTranslationComponent"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    ///// @brief Add a constructor
    //TranslationComponent* new_PyTranslationComponent(TransformComponent* t);

    ///// @brief Add a destructor
    //void delete_PyTranslationComponent(TranslationComponent* t);

    /// @brief Obtain translation component as JSON
    QJsonValue as_json_str(TranslationComponent* t) const;

    /// @property Position
    const Vector3* local_pos(TranslationComponent* t) const;
    void set_local_pos(TranslationComponent* t, Vector3* pos); // More specific overload must come first
    void set_local_pos(TranslationComponent* t, PyObject* pos);

    ///// @property Velocity
    //const Vector3g* get_velocity(TranslationComponent* t) const;
    //void set_velocity(TranslationComponent* t, Vector3g* vel);
    //void set_velocity(TranslationComponent* t, PyObject* vel);

    ///// @property Acceleration
    //const Vector3g* get_acceleration(TranslationComponent* t) const;
    //void set_acceleration(TranslationComponent* t, Vector3g* acc);
    //void set_acceleration(TranslationComponent* t, PyObject* acc);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(TranslationComponent* t);

    /// @}
private:

};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyQuaternion
/// @brief Wrapper for a quaternion component
class PyQuaternion : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyQuaternion();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyQuaternion"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyQuaternion"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Add constructors
    /// @note Wrapper does NOT work with macro types, so need to hard-code
    Quaternion* new_Quaternion();
    Quaternion* new_Quaternion(double x, double y, double z, double w);
    //Quaternion* new_Quaternion(double roll, double pitch, double yaw);

    Quaternion* static_Quaternion_look_rotation(const Vector3* dir, const Vector3* up);
    Quaternion* static_Quaternion_slerp(const Quaternion* q1, const Quaternion* q2, double factor);
    Quaternion* static_Quaternion_from_axis_angle(const Vector3* axis, double angle);

    /// @details Use default rotation space, EulerAngles::EulerType = EulerAngles::kZYX, RotationSpace = RotationSpace::kInertial
    Quaternion* static_Quaternion_from_euler_angles(double a1, double a2, double a3);
    Quaternion* static_Quaternion_from_euler_angles(const EulerAngles* angles);
    Quaternion* static_Quaternion_from_rotation_matrix(const Matrix3x3d* mat);
    Quaternion* static_Quaternion_from_rotation_matrix(const Matrix4x4d* mat);
    Quaternion* static_Quaternion_from_direction(const Vector3* dir, const Vector3* up);


    /// @brief Add a destructor
    void delete_Quaternion(Quaternion* q);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(Quaternion* q);

    /// @brief Wraps operator==
    bool __eq__(Quaternion* q, Quaternion* q2);

    /// @brief Wrap arithmetic operators
    Quaternion *__add__(Quaternion* q, Quaternion* q2);
    Quaternion *__sub__(Quaternion* q, Quaternion* q2);
    Quaternion *__mul__(Quaternion* q, Quaternion* q2);
    Vector3 *__mul__(Quaternion* q, Vector3* v);

    Quaternion* __iadd__(Quaternion* q, Quaternion* q2);
    Quaternion* __isub__(Quaternion* q, Quaternion* q2);
    Quaternion* __imul__(Quaternion* q, Quaternion* q2);

    int norm(Quaternion* q) const;

    void normalize(Quaternion* q) const;

    /// @}
private:

};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyEulerAngles
/// @brief Wrapper for an euler angle component
class PyEulerAngles : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyEulerAngles();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyEulerAngles"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyEulerAngles"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Add constructors
    /// @note Wrapper does NOT work with macro types, so need to hard-code
    /// @param [in] rotationOrder list of axes for rotation order- x = 0, y = 1, z = 2
    EulerAngles* new_EulerAngles(float ax, float ay, float az, PyObject* rotationOrder, int rotationType = 0);

    /// @brief Add a destructor
    void delete_EulerAngles(EulerAngles* e);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @}
private:

    EulerAngles::Axes toAxes(PyObject* o);

};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyRotationComponent
/// @brief Wrapper for a rotational component
class PyRotationComponent : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyRotationComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyRotationComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyRotationComponent"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Add a rotation corresponding to the given euler angles to the rotation component
    void add_rotation(RotationComponent* r, const EulerAngles& eulerAngles);

    const Quaternion* quaternion(RotationComponent* r) const;
    const Quaternion* get_quaternion(RotationComponent* r) const;

    /// @brief Set the rotation
    void set(RotationComponent* r, Quaternion* quaternion);
    void set_rotation(RotationComponent* r, Quaternion* quaternion);
    void set_rotation(RotationComponent* r, const EulerAngles& eulerAngles);

    /// @brief Obtain rotation component as JSON
    QJsonValue as_json_str(RotationComponent* r) const;


    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(RotationComponent* r);

    /// @}
private:

};





/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyScaleComponent
/// @brief Wrapper for a scale component
class PyScaleComponent : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyScaleComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyScaleComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyScaleComponent"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Get scale for TransformComponent component
    PyObject* get_scale(ScaleComponent* s) const;

    /// @brief Set scale for TransformComponent component
    void set_scale(ScaleComponent* s, Vector3* scale);
    void set_scale(ScaleComponent* s, PyObject* scale);
    void set_scale(ScaleComponent* s, double x, double y, double z);


    /// @brief Obtain scale component as JSON
    QJsonValue as_json_str(ScaleComponent* s) const;


    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(ScaleComponent* s);

    /// @}
private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyAudioComponent
/// @brief Wrapper for an audio component
class PyAudioComponent : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyAudioComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyAudioComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyAudioComponent"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Obtain translation component as JSON
    QJsonValue as_json_str(AudioSourceComponent* ac) const;

    /// @brief Queue the audio component to play
    void play(AudioSourceComponent* ac);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(AudioSourceComponent* ac);

    /// @}
private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyInputHandler
/// @brief Wrapper for a scene object
class PyInputHandler : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyInputHandler();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyInputHandler"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyInputHandler"; }

    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Key handler
    KeyHandler* key_handler(InputHandler* ih);

    /// @brief Mouse handler
    MouseHandler* mouse_handler(InputHandler* ih);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(InputHandler* ih);

    /// @}
private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyKeyHandler
/// @brief Wrapper for a scene object
class PyKeyHandler : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyKeyHandler();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyKeyHandler"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyKeyHandler"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Whether a key was pressed or released
    bool was_pressed(KeyHandler* kh, int key);
    bool was_pressed(KeyHandler* kh, const QString& key);

    bool was_released(KeyHandler* kh, int key);
    bool was_released(KeyHandler* kh, const QString& key);

    bool was_double_clicked(KeyHandler* kh, int key);
    bool was_double_clicked(KeyHandler* kh, const QString& key);

    /// @brief Whether the given input is held or not
    bool is_held(KeyHandler* kh, int key) const;
    bool is_held(KeyHandler* kh, const QString& key);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(KeyHandler* ih);

    /// @}
private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyInputHandler
/// @brief Wrapper for a scene object
class PyMouseHandler : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyMouseHandler();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyMouseHandler"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyMouseHandler"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Mouse input checks
    bool was_pressed(MouseHandler* mh, int btn);
    bool was_released(MouseHandler* mh, int btn);
    bool was_double_clicked(MouseHandler* mh, int btn);
    bool is_held(MouseHandler* mh, int btn);
    bool moved(MouseHandler* mh);
    bool scrolled(MouseHandler* mh);

    /// @brief Return how much the mouse has moved since the last frame
    const Vector2* mouse_delta(MouseHandler* m);

    /// @brief Return how much the mouse was scrolled
    const Vector2* scroll_delta(MouseHandler* m);

    /// @brief Return screen-space mouse position
    Vector3* screen_pos(MouseHandler* mh);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(MouseHandler* ih);

    /// @}
private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif