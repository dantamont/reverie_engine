#ifndef GB_TRANSFORM_COMPONENTS_H
#define GB_TRANSFORM_COMPONENTS_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External

// Project
#include "../GObject.h"
#include "../geometry/GEulerAngles.h"
#include "../geometry/GQuaternion.h"
#include "../geometry/GMatrix.h"
#include "../time/GDateTime.h"
#include "../geometry/GFrame.h"
#include "../mixins/GLoadable.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Transform;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Enum for transform types
enum class TransformComponentType {
    //kBaseComponent = -2,
    //kAffineComponent = -1,
    kTranslationComponent,
    kRotationComponent,
    kScaleComponent
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ScaleComponent
///    @brief  Represents a transform component that scales an object

class ScaleComponent {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ScaleComponent();
    ScaleComponent(Transform* transform, const Matrix4x4& scaling);
    ScaleComponent(Transform* transform, const Vector3& scaling);
    ~ScaleComponent() {}

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{    

    /// @brief get Matrix
    const Matrix4x4& getMatrix() const { return m_transformMatrix; }

    /// @property Transform type
    /// @brief Get transform type
    static TransformComponentType type() { return TransformComponentType::kScaleComponent; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{    

    bool isIdentity() const {
        return m_scale[0] == 1.0 && m_scale[1] == 1.0 && m_scale[2] == 1.0;
    }

    /// @brief Obtain scaling vector
    const Vector3& getScale() const { return m_scale; }
    Vector3& scale() { return m_scale; }

    /// @brief Set scaling vector and update transform matrix
    void setScale(Transform* transform, const Vector3& scaling, bool updateTransform = true);
    void setScale(Transform* transform, double x, double y, double z, bool updateTransform = true);
    void setScale(Transform* transform, double scaling, bool updateTransform = true);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty());

    /// @}

protected:
    friend class Transform;

    /// @brief Compute scale matrix
    void computeTransformMatrix(Transform* transform, bool updateTransform = true);

    Vector3 m_scale;

    Matrix4x4 m_transformMatrix;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @class RotationComponent
    @brief  Represents a transform component that rotates an object
*/
class RotationComponent {
public:
    typedef Vector<Axis, 3> Axes;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    RotationComponent();
    RotationComponent(Transform* transform, const Quaternion& quaternion);
    RotationComponent(Transform* transform, const EulerAngles& eulerAngles);
    ~RotationComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{    

    /// @property Transform type
    /// @brief Get transform type
    static TransformComponentType type() { return TransformComponentType::kRotationComponent; }


    const Quaternion& getQuaternion() const { return m_quaternion; }

    /// @brief get Matrix
    const Matrix4x4& getMatrix() const { return m_transformMatrix; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{    

    /// @brief Add a rotation corresponding to the given euler angles to this component
    void addRotation(Transform* t, const EulerAngles& eulerAngles, bool updateTransform = true);

    /// @brief Set rotation using quaternion and update rotation matrix
    void setRotation(Transform* t, const Quaternion& quaternion, bool updateTransform = true);

    /// @brief Set rotation using euler angles and update rotation matrix
    void setRotation(Transform* t, const EulerAngles& eulerAngles, bool updateTransform = true);


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty());

    /// @}

protected:
    friend class Transform;

    /// @brief Compute scale matrix
    void computeTransformMatrix(Transform* transform, bool updateTransform = true);
    void computeTransformMatrix(Transform* transform, const EulerAngles& eulerAngles, bool updateTransform = true);

    /// @brief Quaternion representing this rotation
    Quaternion m_quaternion;

    Matrix4x4g m_transformMatrix;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class Translation
///    @brief  Represents a translation
struct Translation: public Serializable {
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{    
    Translation();
    Translation(const QJsonValue& json);
    Translation(const Vector3& position, CoordinateFrame frame = kGLWorld);
    //Translation(const Vector3& position, const Vector3& velocity, const Vector3& acceleration, CoordinateFrame frame = kGLWorld);
    Translation(const Translation& t);
    ~Translation();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{  
    Translation operator+ (const Translation &t)  const;
    Translation operator- (const Translation &t)  const;
    Translation operator* (double c) const;
    Translation operator/ (double c) const;
    friend std::ostream& operator<<(std::ostream& os, const Translation& t);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Methods
    /// @{  

    bool isIdentity() const {
        return m_position.x() == 0 && m_position.y() == 0 && m_position.z() == 0;
    }

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
    /// @name Members
    /// @{  

    //CoordinateFrame m_frame = CoordinateFrame::kGLWorld;
    Vector3 m_position;
    //Vector3 m_velocity;
    //Vector3 m_acceleration;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @class TranslationComponent
*/
class TranslationComponent {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    TranslationComponent();
    TranslationComponent(const Vector3& translation);
    ~TranslationComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{    

    /// @property Transform type
    /// @brief Get transform type
    static TransformComponentType type() { return TransformComponentType::kTranslationComponent; }

    /// @brief get Matrix
    const Matrix4x4& getMatrix() const { return m_transformMatrix; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{    

    bool isIdentity() const {
        return m_translation.isIdentity();
    }

    /// @brief Adds to the position to this component
    //void addTranslation(const Vector3g& position, bool updateTransform = true);

    /// @brief Obtain translation
    const Translation& getTranslation() const { return m_translation; }

    /// @brief Position/velocity/acceleration
    const Vector3& getPosition() const { return m_translation.m_position; }
    Vector3& position() { return m_translation.m_position; }
    void setPosition(Transform* transform, const Vector3& p, bool updateTransform = true);

    //const Vector3& getVelocity() const { return m_translation.m_velocity; }
    //void setVelocity(const Vector3& v, bool updateTransform = true) {
    //    m_translation.m_velocity = v; 
    //    computeTransformMatrix(updateTransform);
    //}

    //const Vector3& getAcceleration() const { return m_translation.m_acceleration; }
    //void setAcceleration(const Vector3& a, bool updateTransform = true) {
    //    m_translation.m_acceleration = a; 
    //    computeTransformMatrix(updateTransform);
    //}

    /// @brief Sets the translation and updates the translation matrix
    void setTranslation(Transform* transform, const Translation& position, bool updateTransform = true);
    void setTranslation(Transform* transform, const Vector3& position, bool updateTransform = true);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty());

    /// @}

protected:
    friend class Transform;

    /// @brief Compute scale matrix
    void computeTransformMatrix(Transform* transform, bool updateTransform = true);

    /// @brief Quaternion representing this rotation
    Translation m_translation;

    Matrix4x4g m_transformMatrix;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
