#pragma once

// Project
#include "fortress/containers/math/GEulerAngles.h"
#include "fortress/containers/math/GMatrix.h"

namespace rev {

template <typename WorldMatrixType>
class TransformTemplate;

/// @class ScaleComponent
/// @brief Represents a transform component that scales an object
template <typename WorldMatrixType>
class ScaleComponent {
public:
    /// @name Constructors and Destructors
    /// @{
    ScaleComponent() = default;
    ScaleComponent(TransformTemplate<WorldMatrixType>* transform, const Matrix4x4& scaling) {
        computeTransformMatrix(transform);
    }

    ScaleComponent(TransformTemplate<WorldMatrixType>* transform, const Vector3& scaling) :
        m_scale(scaling)
    {
        computeTransformMatrix(transform);
    }
    ~ScaleComponent() = default;

    /// @}

    /// @name Public Methods
    /// @{    

    bool isIdentity() const {
        if (m_scale[0] == 1.0) {
            if (m_scale[1] == 1.0) {
                if (m_scale[2] == 1.0) {
                    return true;
                }
            }
        }
        return false;
    }

    /// @brief Obtain scaling vector
    const Vector3& getScale() const { return m_scale; }
    Vector3& scale() { return m_scale; }

    /// @brief Set scaling vector and update transform matrix
    void setScale(TransformTemplate<WorldMatrixType>* transform, const Vector3& scaling, bool updateTransform = true) {
        m_scale = scaling;
        computeTransformMatrix(transform, updateTransform);
    }

    void setScale(TransformTemplate<WorldMatrixType>* transform, double x, double y, double z, bool updateTransform = true) {
        m_scale = { float(x), float(y), float(z) };
        computeTransformMatrix(transform, updateTransform);
    }

    void setScale(TransformTemplate<WorldMatrixType>* transform, double scaling, bool updateTransform = true) {
        m_scale = rev::Vector3(1, 1, 1) * scaling;
        computeTransformMatrix(transform, updateTransform);
    }

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const ScaleComponent& korObject) {
        orJson = { {"scaling", korObject.m_scale} };
    }

    /// @}

protected:
    friend class TransformTemplate<WorldMatrixType>;

    /// @brief Compute scale matrix
    void computeTransformMatrix(TransformTemplate<WorldMatrixType>* transform, bool updateTransform = true) {
        if (transform && updateTransform) {
            transform->computeWorldMatrix();
        }
    }

    Vector3 m_scale{1, 1, 1}; ///< The scale
};


/// @class RotationComponent
/// @brief  Represents a transform component that rotates an object
template <typename WorldMatrixType>
class RotationComponent {
public:
    typedef Vector<Axis, 3> Axes;

    /// @name Constructors and Destructors
    /// @{
    RotationComponent() = default;

    RotationComponent(TransformTemplate<WorldMatrixType>* transform, const Quaternion& quaternion) :
        m_quaternion(quaternion)
    {
        computeTransformMatrix(transform);
    }

    RotationComponent(TransformTemplate<WorldMatrixType>* transform, const EulerAngles& eulerAngles) {
        computeTransformMatrix(transform, eulerAngles);
    }

    ~RotationComponent() = default;

    /// @}

    /// @name Public Methods
    /// @{    

    const Quaternion& getQuaternion() const { return m_quaternion; }

    /// @brief get Matrix
    const Matrix4x4& getMatrix() const { return m_transformMatrix; }

    /// @brief Add a rotation corresponding to the given euler angles to this component
    void addRotation(TransformTemplate<WorldMatrixType>* transform, const EulerAngles& eulerAngles, bool updateTransform = true) {
        Matrix4x4 addedRotation = eulerAngles.toRotationMatrixF();
        m_transformMatrix = addedRotation * m_transformMatrix;
        m_quaternion = Quaternion::fromRotationMatrix(m_transformMatrix);
        if (transform && updateTransform) {
            transform->computeWorldMatrix();
        }
    }

    /// @brief Set rotation using quaternion and update rotation matrix
    void setRotation(TransformTemplate<WorldMatrixType>* t, const Quaternion& quaternion, bool updateTransform = true) {
        m_quaternion = quaternion;
        computeTransformMatrix(t, updateTransform);
    }

    /// @brief Set rotation using euler angles and update rotation matrix
    void setRotation(TransformTemplate<WorldMatrixType>* t, const EulerAngles& eulerAngles, bool updateTransform = true) {
        computeTransformMatrix(t, eulerAngles);
    }


    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const RotationComponent& korObject) {
        orJson["quaternion"] = korObject.m_quaternion;
    }

    /// @}

protected:
    friend class TransformTemplate<WorldMatrixType>;

    /// @brief Compute scale matrix
    void computeTransformMatrix(TransformTemplate<WorldMatrixType>* transform, bool updateTransform = true) {
        m_quaternion.toRotationMatrix(m_transformMatrix);
        if (transform && updateTransform) {
            transform->computeWorldMatrix();
        }
    }

    void computeTransformMatrix(TransformTemplate<WorldMatrixType>* transform, const EulerAngles& eulerAngles, bool updateTransform = true) {
        m_transformMatrix = eulerAngles.toRotationMatrixF();
        setRotation(transform, Quaternion::fromRotationMatrix(m_transformMatrix));
        if (transform && updateTransform) {
            transform->computeWorldMatrix();
        }
    }

    Quaternion m_quaternion{0, 0, 0, 1}; ///< Quaternion representing this rotation
    Matrix4x4 m_transformMatrix; ///< The matrix representing the rotation
};


/// @class Translation
/// @brief  Represents a translation
class Translation {
public:
    /// @name Constructors and Destructors
    /// @{    
    Translation() = default;

    Translation(const json& json) {
        json.get_to(*this);
    }

    Translation(const Vector3& position) :
        m_position(position)
    {
    }

    Translation(const Translation& t) :
        m_position(t.m_position)
    {
    }

    ~Translation() = default;

    /// @}

    /// @name Operators
    /// @{  
    Translation operator+ (const Translation &t) const {
        return Translation(m_position + t.m_position);
    }

    Translation operator- (const Translation &t) const {
        return Translation(m_position - t.m_position);
    }

    Translation operator* (double c) const {
        return Translation(m_position * c);
    }

    Translation operator/ (double c) const {
        double factor = 1.0 / c;
        return Translation(m_position * factor);
    }

    friend std::ostream& operator<<(std::ostream& os, const Translation& t) {
        return os << "Translation("
            << "position: " << t.m_position
            << ")";
    }

    /// @}

    /// @name Methods
    /// @{  

    bool isIdentity() const {
        if (m_position.x() == 0) {
            if (m_position.y() == 0) {
                if (m_position.z() == 0) {
                    return true;
                }
            }
        }
        return false;
    }

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Translation& korObject) {
        orJson = { { "position", korObject.m_position } };
    }

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Translation& orObject) {
        orObject.m_position = korJson.at("position").get<Vector3>();
    }

    /// @}

    /// @name Members
    /// @{  

    Vector3 m_position{0, 0, 0};

    /// @}
};

/// @class TranslationComponent
template <typename WorldMatrixType>
class TranslationComponent {
public:
    /// @name Constructors and Destructors
    /// @{
    /// 
    TranslationComponent() = default;

    TranslationComponent(const Vector3& translation) :
        m_translation(translation)
    {
    }

    ~TranslationComponent() = default;

    /// @}

    /// @name Public Methods
    /// @{    

    bool isIdentity() const {
        return m_translation.isIdentity();
    }

    /// @brief Obtain translation
    const Translation& getTranslation() const { return m_translation; }

    /// @brief Position/velocity/acceleration
    const Vector3& getPosition() const { return m_translation.m_position; }
    Vector3& position() { return m_translation.m_position; }

    void setPosition(TransformTemplate<WorldMatrixType>* transform, const Vector3& p, bool updateTransform = true) {
        m_translation.m_position = p;
        computeTransformMatrix(transform, updateTransform);
    }

    /// @brief Sets the translation and updates the translation matrix
    void setTranslation(TransformTemplate<WorldMatrixType>* transform, const Translation& position, bool updateTransform = true) {
        m_translation = position;
        computeTransformMatrix(transform, updateTransform);
    }
    
    void setTranslation(TransformTemplate<WorldMatrixType>* transform, const Vector3& position, bool updateTransform = true) {
        m_translation.m_position = position;
        computeTransformMatrix(transform, updateTransform);
    }

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const TranslationComponent& korObject) {
        orJson = { { "translation", korObject.m_translation } };
    }


    /// @}

protected:
    friend class TransformTemplate<WorldMatrixType>;

    /// @brief Compute scale matrix
    void computeTransformMatrix(TransformTemplate<WorldMatrixType>* transform, bool updateTransform = true) {
        if (transform && updateTransform) {
            transform->computeWorldMatrix();
        }
    }

    Translation m_translation; ///< The translation of the object
};



} // end namespacing

