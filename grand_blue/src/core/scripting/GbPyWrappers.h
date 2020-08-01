/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_COMPONENT_WRAPPERS_H
#define GB_COMPONENT_WRAPPERS_H

// External

// QT

// Internal
#include "GbPythonScript.h"
#include "../geometry/GbEulerAngles.h"
#include "../geometry/GbQuaternion.h"
#include "../geometry/GbMatrix.h"

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
class Scene;
class Scenario;
class ShaderProgram;
class ResourceCache;
struct Uniform;
class CustomEvent;

/////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs and Usings
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class PyEngine
/// @brief Wrapper for the core engine 
class PyEngine : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyEngine();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyEngine"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyEngine"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Get the current scenario from the core engine
    Scenario* scenario(CoreEngine* e) const;

    /// @brief Get the main input handler from the core engine
    InputHandler* input_handler(CoreEngine* e) const;

    /// @brief Get the resource cache
    ResourceCache* resource_cache(CoreEngine* e) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(CoreEngine* e);

    /// @}
private:
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyResourceCache
/// @brief Wrapper for the core engine's resource cache 
class PyResourceCache : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyResourceCache();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyResourceCache"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyResourceCache"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Return shader program from the resource cache
    const ShaderProgram* get_shader(ResourceCache* r, const QString& name) const;

    /// @brief Obtain scene object as JSON
    QJsonValue as_json_str(ResourceCache* r) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(ResourceCache* r);

    /// @}
private:
};


/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyCustomEvent
/// @brief Wrapper for the core engine's resource cache 
class PyCustomEvent : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyCustomEvent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyCustomEvent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyCustomEvent"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Obtain scene object as JSON
    QJsonValue as_json_str(CustomEvent* e) const;

    /// @brief Type of event
    int type(CustomEvent* e) const;

    /// @brief Return data as a QVariantMap
    QVariantMap dataMap(CustomEvent* e) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(CustomEvent* e);

    /// @}
private:
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyVector2
/// @brief Wrapper for a 4-element vector
class PyVector2 : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyVector2();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyVector2"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyVector2"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Construct a new Vector2
    Vector2* new_Vector2();
    Vector2* new_Vector2(double v1, double v2);
    Vector2* new_Vector2(Vector2* o);
    Vector2* new_Vector2(PyObject* o);

    /// @brief Destroy a Vector2
    void delete_Vector2(Vector2* v);


    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps operator==
    bool __eq__(Vector2* v, Vector2* v2);

    /// @brief Wrap arithmetic operators
    Vector2* __add__(Vector2* v, Vector2* v2);
    Vector2* __add__(Vector2* v, double f);
    Vector2* __add__(Vector2* v, int f);
    Vector2* __add__(Vector2* v, PyObject* p2);
    Vector2* __sub__(Vector2* v, Vector2* v2);
    Vector2* __sub__(Vector2* v, PyObject* p2);
    Vector2* __mul__(Vector2* v, Vector2* v2);
    Vector2* __mul__(Vector2* v, double f);
    Vector2* __mul__(Vector2* v, int f);
    Vector2* __mul__(Vector2* v, PyObject* o);

    Vector2* __iadd__(Vector2* v, Vector2* v2);
    Vector2* __isub__(Vector2* v, Vector2* v2);
    Vector2* __imul__(Vector2* v, Vector2* v2);

    /// @brief Dot product
    double dot(Vector2* v1, Vector2* v2);

    /// @brief Converts to a list
    PyObject* to_list(Vector2* v) const;

    /// @brief Return a normalized version of the vector
    Vector2* normalized(Vector2* v) const;
    void normalize(Vector2* v) const;
    double length(Vector2* v) const;

    /// @brief Wraps __str__
    QString py_toString(Vector2* v);

    /// @}
private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyVector3
/// @brief Wrapper for a 4-element vector
class PyVector3 : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyVector3();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyVector3"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyVector3"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Construct a new Vector3
    Vector3* new_Vector3();
    Vector3* new_Vector3(double v1, double v2, double v3);
    Vector3* new_Vector3(Vector3* o);
    Vector3* new_Vector3(PyObject* o);

    /// @brief Destroy a Vector3
    void delete_Vector3(Vector3* v);


    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps operator==
    bool __eq__(Vector3* v, Vector3* v2);

    /// @brief Wrap arithmetic operators
    Vector3* __add__(Vector3* v, Vector3* v2);
    Vector3* __add__(Vector3* v, double f);
    Vector3* __add__(Vector3* v, int f);
    Vector3* __add__(Vector3* v, PyObject* p2);
    Vector3* __sub__(Vector3* v, Vector3* v2);
    Vector3* __sub__(Vector3* v, PyObject* p2);
    Vector3* __mul__(Vector3* v, Vector3* v2);
    Vector3* __mul__(Vector3* v, double f);
    Vector3* __mul__(Vector3* v, int f);
    Vector3* __mul__(Vector3* v, PyObject* o);

    Vector3* __iadd__(Vector3* v, Vector3* v2);
    Vector3* __isub__(Vector3* v, Vector3* v2);
    Vector3* __imul__(Vector3* v, Vector3* v2);

    /// @brief Cross product
    Vector3* cross(Vector3* v1, Vector3* v2);

    /// @brief Dot product
    double dot(Vector3* v1, Vector3* v2);

    /// @brief Converts to a list
    PyObject* to_list(Vector3* v) const;

    /// @brief Return a normalized version of the vector
    Vector3* normalized(Vector3* v) const;
    void normalize(Vector3* v) const;
    double length(Vector3* v) const;

    /// @brief Wraps __str__
    QString py_toString(Vector3* v);

    /// @}
private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyVector4
/// @brief Wrapper for a 4-element vector
class PyVector4 : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyVector4();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyVector4"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyVector4"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Construct a new Vector4
    Vector4* new_Vector4();
    Vector4* new_Vector4(double v1, double v2, double v3, double v4);
    Vector4* new_Vector4(Vector4* o);
    Vector4* new_Vector4(PyObject* o);

    /// @brief Destroy a Vector4
    void delete_Vector4(Vector4* v);


    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps operator==
    bool __eq__(Vector4* v, Vector4* v2);

    /// @brief Wrap arithmetic operators
    Vector4* __add__(Vector4* v, Vector4* v2);
    Vector4* __add__(Vector4* v, PyObject* p2);
    Vector4* __sub__(Vector4* v, Vector4* v2);
    Vector4* __sub__(Vector4* v, PyObject* p2);
    Vector4* __mul__(Vector4* v, Vector4* v2);
    Vector4* __mul__(Vector4* v, double f);
    Vector4* __mul__(Vector4* v, int f);
    Vector4* __mul__(Vector4* v, PyObject* o);

    Vector4* __iadd__(Vector4* v, Vector4* v2);
    Vector4* __isub__(Vector4* v, Vector4* v2);
    Vector4* __imul__(Vector4* v, Vector4* v2);

    /// @brief Converts to a list
    PyObject* to_list(Vector4* v) const;

    /// @brief Normalizes the vector
    Vector4* normalized(Vector4* v) const;
    void normalize(Vector4* v) const;
    double length(Vector4* v) const;


    /// @brief Wraps __str__
    QString py_toString(Vector4* e);

    /// @}
private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyMatrix2x2
/// @brief Wrapper for a 4-element vector
class PyMatrix2x2 : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyMatrix2x2();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyMatrix2x2"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyMatrix2x2"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Construct a new Matrix2x2
    Matrix2x2* new_Matrix2x2();
    Matrix2x2* new_Matrix2x2(Matrix2x2* o);
    Matrix2x2* new_Matrix2x2(PyObject* o);

    /// @brief Destroy a Matrix2x2
    void delete_Matrix2x2(Matrix2x2* m);

    /// @brief Get element at specified row/column
    double at(Matrix2x2* m, int row, int col);

    /// @brief Get column at specified index
    Vector2* column(Matrix2x2* m, int idx);

    /// @brief Get transposed matrix
    Matrix2x2* transposed(Matrix2x2* m);

    /// @brief Set to identity
    void set_to_identity(Matrix2x2* m);

    /// @brief Get diagonal
    Vector2* diagonal(Matrix2x2* m);

    /// @brief Take the transpose of this matrix, then multiply
    Matrix2x2* transpose_multiply(Matrix2x2* m, Matrix2x2* m2);

    /// @brief Get determinant of the matrix
    double determinant(Matrix2x2* m);

    /// @brief Get inverse of the matrix
    Matrix2x2* inversed(Matrix2x2* m);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps operator==
    bool __eq__(Matrix2x2* m, Matrix2x2* v2);

    /// @brief Wrap arithmetic operators
    Matrix2x2* __add__(Matrix2x2* m, Matrix2x2* m2);
    Matrix2x2* __add__(Matrix2x2* m, PyObject* p2);
    Matrix2x2* __sub__(Matrix2x2* m, Matrix2x2* m2);
    Matrix2x2* __sub__(Matrix2x2* m, PyObject* p2);
    Matrix2x2* __mul__(Matrix2x2* m, Matrix2x2* m2);
    Matrix2x2* __mul__(Matrix2x2* m, double f);
    Matrix2x2* __mul__(Matrix2x2* m, int f);
    Matrix2x2* __mul__(Matrix2x2* m, PyObject* o);

    Matrix2x2* __iadd__(Matrix2x2* m, Matrix2x2* m2);
    Matrix2x2* __isub__(Matrix2x2* m, Matrix2x2* m2);
    Matrix2x2* __imul__(Matrix2x2* m, Matrix2x2* m2);

    /// @brief Converts to a list of lists
    PyObject* to_list(Matrix2x2* m) const;

    /// @brief Wraps __str__
    QString py_toString(Matrix2x2* m);

    /// @}
private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyMatrix3x3
/// @brief Wrapper for a 4-element vector
class PyMatrix3x3 : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyMatrix3x3();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyMatrix3x3"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyMatrix3x3"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Construct a new Matrix3x3
    Matrix3x3* new_Matrix3x3();
    Matrix3x3* new_Matrix3x3(Matrix3x3* o);
    Matrix3x3* new_Matrix3x3(PyObject* o);

    /// @brief Destroy a Matrix3x3
    void delete_Matrix3x3(Matrix3x3* m);

    /// @brief Get element at specified row/column
    double at(Matrix3x3* m, int row, int col);

    /// @brief Get column at specified index
    Vector3* column(Matrix3x3* m, int idx);

    /// @brief Get transposed matrix
    Matrix3x3* transposed(Matrix3x3* m);

    /// @brief Set to identity
    void set_to_identity(Matrix3x3* m);

    /// @brief Get diagonal
    Vector3* diagonal(Matrix3x3* m);

    /// @brief Take the transpose of this matrix, then multiply
    Matrix3x3* transpose_multiply(Matrix3x3* m, Matrix3x3* m2);

    /// @brief Get determinant of the matrix
    double determinant(Matrix3x3* m);

    /// @brief Get inverse of the matrix
    Matrix3x3* inversed(Matrix3x3* m);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps operator==
    bool __eq__(Matrix3x3* m, Matrix3x3* v2);

    /// @brief Wrap arithmetic operators
    Matrix3x3* __add__(Matrix3x3* m, Matrix3x3* m2);
    Matrix3x3* __add__(Matrix3x3* m, PyObject* p2);
    Matrix3x3* __sub__(Matrix3x3* m, Matrix3x3* m2);
    Matrix3x3* __sub__(Matrix3x3* m, PyObject* p2);
    Matrix3x3* __mul__(Matrix3x3* m, Matrix3x3* m2);
    Matrix3x3* __mul__(Matrix3x3* m, double f);
    Matrix3x3* __mul__(Matrix3x3* m, int f);
    Matrix3x3* __mul__(Matrix3x3* m, PyObject* o);

    Matrix3x3* __iadd__(Matrix3x3* m, Matrix3x3* m2);
    Matrix3x3* __isub__(Matrix3x3* m, Matrix3x3* m2);
    Matrix3x3* __imul__(Matrix3x3* m, Matrix3x3* m2);

    /// @brief Converts to a list of lists
    PyObject* to_list(Matrix3x3* m) const;

    /// @brief Wraps __str__
    QString py_toString(Matrix3x3* m);

    /// @}
private:

};


/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyMatrix4x4
/// @brief Wrapper for a 4-element vector
class PyMatrix4x4 : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyMatrix4x4();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyMatrix4x4"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyMatrix4x4"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Construct a new Matrix4x4
    Matrix4x4* new_Matrix4x4();
    Matrix4x4* new_Matrix4x4(Matrix4x4* o);
    Matrix4x4* new_Matrix4x4(PyObject* o);

    /// @brief Destroy a Matrix4x4
    void delete_Matrix4x4(Matrix4x4* m);

    /// @brief Get element at specified row/column
    double at(Matrix4x4* m, int row, int col);

    /// @brief Get column at specified index
    Vector4* column(Matrix4x4* m, int idx);

    /// @brief Get transposed matrix
    Matrix4x4* transposed(Matrix4x4* m);

    /// @brief Set to identity
    void set_to_identity(Matrix4x4* m);

    /// @brief Get diagonal
    Vector4* diagonal(Matrix4x4* m);

    /// @brief Take the transpose of this matrix, then multiply
    Matrix4x4* transpose_multiply(Matrix4x4* m, Matrix4x4* m2);

    /// @brief Add a scaling to the matrix
    void add_scale(Matrix4x4* m, Vector3* scale);

    /// @brief Add a rotation to the matrix
    void add_rotation(Matrix4x4* m, Vector3* axis, double angle);

    /// @brief Get and set the translation of the matrix
    Vector3* translation_vec(Matrix4x4* m);
    void set_translation_vec(Matrix4x4* m, Vector3* vec);


    /// @brief Get determinant of the matrix
    double determinant(Matrix4x4* m);

    /// @brief Get inverse of the matrix
    Matrix4x4* inversed(Matrix4x4* m);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps operator==
    bool __eq__(Matrix4x4* m, Matrix4x4* v2);

    /// @brief Wrap arithmetic operators
    Matrix4x4* __add__(Matrix4x4* m, Matrix4x4* m2);
    Matrix4x4* __add__(Matrix4x4* m, PyObject* p2);
    Matrix4x4* __sub__(Matrix4x4* m, Matrix4x4* m2);
    Matrix4x4* __sub__(Matrix4x4* m, PyObject* p2);
    Matrix4x4* __mul__(Matrix4x4* m, Matrix4x4* m2);
    Matrix4x4* __mul__(Matrix4x4* m, double f);
    Matrix4x4* __mul__(Matrix4x4* m, int f);
    Matrix4x4* __mul__(Matrix4x4* m, PyObject* o);

    Matrix4x4* __iadd__(Matrix4x4* m, Matrix4x4* m2);
    Matrix4x4* __isub__(Matrix4x4* m, Matrix4x4* m2);
    Matrix4x4* __imul__(Matrix4x4* m, Matrix4x4* m2);

    /// @brief Converts to a list of lists
    PyObject* to_list(Matrix4x4* m) const;

    /// @brief Wraps __str__
    QString py_toString(Matrix4x4* m);

    /// @}
private:

};



/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyScenario
/// @brief Wrapper for a scenario object
class PyScenario : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyScenario();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyScenario"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyScenario"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{


    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(Scenario* e);

    /// @}
private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyScene
/// @brief Wrapper for a scene object
class PyScene : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyScene();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyScene"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyScene"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{


    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(Scene* e);

    /// @}
private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PySceneObject
/// @brief Wrapper for a scene object
class PySceneObject : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PySceneObject();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PySceneObject"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PySceneObject"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Public Methods
    /// @{

    /// @}


public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

     /// @brief Construct a new scene object
    SceneObject* new_SceneObject(Scene* scene);

    /// @brief Construct a new shared_ptr to a scene object from an existing one
    /// @details 
    /// \param [in] uuidStr String of UUID of wrapped scene object
    SceneObject* new_SceneObject(const QString& uuid);

    /// @brief Destroy shared_ptr to scene object
    void delete_SceneObject(SceneObject* object);

    /// @brief Obtain scene object as JSON
    QString get_name(SceneObject* object) const;

    /// @brief Obtain scene object as JSON
    QJsonValue as_json_str(SceneObject* object) const;

    /// @brief Obtain TransformComponent 
    /// See: https://sourceforge.net/p/pythonqt/discussion/631393/thread/116f83ae/
    TransformComponent* transform(SceneObject* object);

    /// @brief Obtain engine for the scene object
    CoreEngine* engine(SceneObject* object);

    /// @brief Obtain the scene for the scene object
    Scene* scene(SceneObject* object);

    /// @brief Allow the scene object to go out of scope
    void remove(SceneObject* object);

    /// @}
private:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Private members
    /// @details Methods for managing factory
    /// @{

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyLight
/// @brief Wrapper for a Light component
class PyLight : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyLight();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyLight"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyLight"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Add a constructor
    LightComponent* new_LightComponent(SceneObject* object);

    /// @brief Add a destructor
    void delete_LightComponent(LightComponent* l);

    /// @brief Obtain TransformComponent as JSON
    QJsonValue as_json_str(LightComponent* l) const;

    /// @brief Get/set the color of the light
    PyObject* get_diffuse_color(LightComponent* l);
    PyObject* get_ambient_color(LightComponent* l);
    PyObject* get_specular_color(LightComponent* l);
    void set_diffuse_color(LightComponent* l, PyObject* color);
    void set_ambient_color(LightComponent* l, PyObject* color);
    void set_specular_color(LightComponent* l, PyObject* color);
    void set_attenuations(LightComponent* l, PyObject* attenuations);

    /// @brief Get/set the intensity of the light
    double get_intensity(LightComponent* l);
    void set_intensity(LightComponent* l, PyObject* intensity);

    /// @brief Obtain the object for the light
    SceneObject* scene_object(LightComponent* l) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(LightComponent* l);

    /// @}
private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PyMaterial
/// @brief Wrapper for a material component
class PyMaterial : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyMaterial();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyMaterial"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyMaterial"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Add a constructor
    ShaderComponent* new_MaterialComponent(SceneObject* so);

    /// @brief Add a destructor
    void delete_MaterialComponent(ShaderComponent* r);

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
    //-----------------------------------------------------------------------------------------------------------------    
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
    const Vector3* get_position(TranslationComponent* t) const;
    void set_position(TranslationComponent* t, Vector3* pos); // More specific overload must come first
    void set_position(TranslationComponent* t, PyObject* pos);

    ///// @property Velocity
    //const Vector3* get_velocity(TranslationComponent* t) const;
    //void set_velocity(TranslationComponent* t, Vector3* vel);
    //void set_velocity(TranslationComponent* t, PyObject* vel);

    ///// @property Acceleration
    //const Vector3* get_acceleration(TranslationComponent* t) const;
    //void set_acceleration(TranslationComponent* t, Vector3* acc);
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

    Quaternion* __iadd__(Quaternion* q, Quaternion* q2);
    Quaternion* __isub__(Quaternion* q, Quaternion* q2);
    Quaternion* __imul__(Quaternion* q, Quaternion* q2);

    int norm(Quaternion* q) const;


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

    const Quaternion* get_quaternion(RotationComponent* r) const;

    /// @brief Set the rotation
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
/// @class PyShaderProgram
/// @brief Wrapper for a shader program
class PyShaderProgram : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PyShaderProgram();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PyShaderProgram"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PyShaderProgram"; }
    /// @}

public slots:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Slots
    /// @note Only slots are accessible via Python through PythonQt, so must map all desired routines
    /// @{

    /// @brief Add a shader program
    const ShaderProgram* new_ShaderProgram(ResourceCache* cache, const QString& vertPath, const QString& fragPath);

    /// @brief Add a destructor
    void delete_ShaderProgram(ShaderProgram* s);

    /// @brief Get/set uniform value
    PyObject* get_uniform(ShaderProgram* s, const QString& uniformName);
    void set_uniform(ShaderProgram* s, const QString& uniformName, Matrix2x2* value);
    void set_uniform(ShaderProgram* s, const QString& uniformName, Matrix3x3* value);
    void set_uniform(ShaderProgram* s, const QString& uniformName, Matrix4x4* value);
    void set_uniform(ShaderProgram* s, const QString& uniformName, PyObject* value);

    /// @brief Obtain shader program as JSON
    QJsonValue as_json_str(ShaderProgram* s) const;


    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name PythonQt built-in wrappers
    /// @note Standard python operators built by QT
    /// @{

    /// @brief Wraps __str__
    QString py_toString(ShaderProgram* s);

    /// @}
private:

    static PyObject* uniformToPyObject(const Uniform& uniform);
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