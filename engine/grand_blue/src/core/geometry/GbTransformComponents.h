#ifndef GB_TRANSFORM_COMPONENTS_H
#define GB_TRANSFORM_COMPONENTS_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External

// Project
#include "../GbObject.h"
#include "../geometry/GbEulerAngles.h"
#include "../geometry/GbQuaternion.h"
#include "../geometry/GbMatrix.h"
#include "../time/GbDateTime.h"
#include "../geometry/GbFrame.h"
#include "../mixins/GbLoadable.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Transform;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/** @class AbstractTransformComponent
    @brief  An abstract class for a component of a transform
*/
class AbstractTransformComponent: public Object, 
    public Serializable {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    // Enum for transform types
    enum TransformComponentType {
        kBaseComponent = -2,
        kAffineComponent = -1,
        kTranslationComponent,
        kRotationComponent,
        kScaleComponent
    };
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    AbstractTransformComponent(TransformComponentType type);
    virtual ~AbstractTransformComponent() {}

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{    

    /// @property Transform component type
    /// @brief Get transform component type
    TransformComponentType getType() { return m_type; }

    /// @property Transform
    /// @brief Accessors for pointer to parent transform
    void setTransform(Transform* transform);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override { return QJsonObject(); }

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override { Serializable::loadFromJson(json); }

    /// @}

protected:

    /// @Brief Type of component
    TransformComponentType m_type;

    /// @brief Pointer to the transform for this component
    Transform* m_transform;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @class AffineComponent
    @brief  An abstract class for an affine component of a transform
*/
class AffineComponent: public AbstractTransformComponent {
public:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    AffineComponent(TransformComponentType type = kAffineComponent);
    virtual ~AffineComponent() {}

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{    

    /// @brief get Matrix
    const Matrix4x4f& getMatrix() const { return m_transformMatrix; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override { return AbstractTransformComponent::asJson(); }

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override { AbstractTransformComponent::loadFromJson(json); }

    /// @}

protected:
    /// @brief Compute the matrix representing this transform component
    /// @details This method updates the component's transform to reflect changes
    virtual void computeTransformMatrix(bool updateTransform = true);

    /// @brief Matrix representing the transform
    Matrix4x4f m_transformMatrix;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @class ScaleComponent
    @brief  Represents a transform component that scales an object
*/
class ScaleComponent : public AffineComponent {
public:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ScaleComponent();
    ScaleComponent(const Matrix4x4f& scaling);
    ScaleComponent(const  Vector3& scaling);
    ~ScaleComponent() {}

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{    

    /// @property Transform type
    /// @brief Get transform type
    static TransformComponentType type() { return kScaleComponent; }

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
    void setScale(const Vector3& scaling, bool updateTransform = true);
    void setScale(double x, double y, double z, bool updateTransform = true);
    void setScale(double scaling, bool updateTransform = true);

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
    friend class Transform;

    /// @brief Compute scale matrix
    void computeTransformMatrix(bool updateTransform = true) override;

    Vector3 m_scale;
};
Q_DECLARE_METATYPE(ScaleComponent);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @class RotationComponent
    @brief  Represents a transform component that rotates an object
*/
class RotationComponent : public AffineComponent {
public:
    typedef Vector<Axis, 3> Axes;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    RotationComponent();
    RotationComponent(const Quaternion& quaternion);
    RotationComponent(const EulerAngles& eulerAngles);
    ~RotationComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{    

    const Quaternion& getQuaternion() const { return m_quaternion; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{    

    /// @brief Add a rotation corresponding to the given euler angles to this component
    void addRotation(const EulerAngles& eulerAngles, bool updateTransform = true);

    /// @brief Set rotation using quaternion and update rotation matrix
    void setRotation(const Quaternion& quaternion, bool updateTransform = true);

    /// @brief Set rotation using euler angles and update rotation matrix
    void setRotation(const EulerAngles& eulerAngles, bool updateTransform = true);


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
    friend class Transform;

    /// @brief Compute scale matrix
    void computeTransformMatrix(bool updateTransform = true) override;
    void computeTransformMatrix(const EulerAngles& eulerAngles, bool updateTransform = true);

    /// @brief Quaternion representing this rotation
    Quaternion m_quaternion;
};
Q_DECLARE_METATYPE(RotationComponent);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @class Translation
    @brief  Represents a translation
*/
struct Translation: public Serializable {
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{    
    Translation();
    Translation(const QJsonValue& json);
    Translation(const Vector3& position, CoordinateFrame frame = kGLWorld);
    Translation(const Vector3& position, const Vector3& velocity, const Vector3& acceleration, CoordinateFrame frame = kGLWorld);
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
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

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
class TranslationComponent : public AbstractTransformComponent {
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

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{    

    bool isIdentity() const {
        return m_translation.isIdentity();
    }

    /// @brief Adds to the position to this component
    void addTranslation(const Vector3& position, bool updateTransform = true);

    /// @brief Obtain translation
    const Translation& getTranslation() const { return m_translation; }

    /// @brief Position/velocity/acceleration
    const Vector3& getPosition() const { return m_translation.m_position; }
    Vector3& position() { return m_translation.m_position; }
    void setPosition(const Vector3& p, bool updateTransform = true);

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
    void setTranslation(const Translation& position, bool updateTransform = true);
    void setTranslation(const Vector3& position, bool updateTransform = true);

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
    friend class Transform;

    /// @brief Quaternion representing this rotation
    Translation m_translation;
};
Q_DECLARE_METATYPE(TranslationComponent);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
