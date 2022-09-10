#pragma once

// Project
#include "GTransformComponents.h"
#include "fortress/containers/GContainerExtensions.h"

namespace physx {
class PxTransform;
class PxVec3;
}

namespace rev {

class SceneObject;

template <typename WorldMatrixType>
class TransformTemplate;

template <typename WorldMatrixType>
void to_json(json& orJson, const TransformTemplate<WorldMatrixType>& korObject);

template <typename WorldMatrixType>
void from_json(const nlohmann::json& korJson, TransformTemplate<WorldMatrixType>& orObject);


/// @brief For managing transforms without needing to worry about vector pointer moving upon resize
struct WorldMatrixVector {

    WorldMatrixVector() = default;

    WorldMatrixVector(std::vector<Matrix4x4>& matrices, Uint32_t index) :
        m_worldMatrices(&matrices),
        m_index(index)
    {
    }

    WorldMatrixVector& operator=(const WorldMatrixVector& other) {
        m_worldMatrices = other.m_worldMatrices;
        m_index = other.m_index;
        return *this;
    }

    std::vector<Matrix4x4>* m_worldMatrices{nullptr}; ///< The vector of world matrices referenced by this transform
    Uint32_t m_index; ///< The index of the world matrix being governed by a transform
};

/// @brief Templated to allow for manipulation of an independently managed world matrix
template<typename WorldMatrixType = Matrix4x4>
struct TransformMatrices {
    static_assert(std::is_same_v<std::remove_pointer_t<WorldMatrixType>, Matrix4x4>
        || std::is_same_v<WorldMatrixType, WorldMatrixVector>, "Type must be a 4x4 matrix or a pointer to one");
    static constexpr bool s_hasPointerWorldMatrixType = std::is_pointer_v<WorldMatrixType>;
    static constexpr bool s_hasIndexOfWorldMatrixType = std::is_same_v<WorldMatrixType, WorldMatrixVector>;

    TransformMatrices() = default;

    TransformMatrices(const WorldMatrixType& worldMatrix) :
        m_worldMatrixContainer(worldMatrix)
    {
        static_assert(!s_hasIndexOfWorldMatrixType, "Type must be a 4x4 matrix or a pointer to one");
    }

    TransformMatrices(WorldMatrixType& worldMatrix) :
        m_worldMatrixContainer(worldMatrix)
    {
    }

    /// @brief Initialize with a vector of world matrices to modify during any operations
    /// @param[in] std::vector<Matrix4x4> A vector containing the matrix that will be modified. The vector must outlive the TransformMatrices object
    /// @param[in] index The index of the matrix to modify
    TransformMatrices(std::vector<Matrix4x4>& worldMatrixVec, Uint32_t index) :
        m_worldMatrixContainer(worldMatrixVec, index)
    {
        static_assert(s_hasIndexOfWorldMatrixType, "Invalid template specialization for this constructor");
    }

    ~TransformMatrices() = default;

    Matrix4x4& worldMatrix() {
        if constexpr (s_hasPointerWorldMatrixType) {
            return *m_worldMatrixContainer;
        }
        else if constexpr (s_hasIndexOfWorldMatrixType) {
            return (*m_worldMatrixContainer.m_worldMatrices)[m_worldMatrixContainer.m_index];
        }
        else {
            static_assert(false, "Invalid template type");
        }
    }

    const Matrix4x4& worldMatrix() const {
        if constexpr (s_hasPointerWorldMatrixType) {
            return *m_worldMatrixContainer;
        }
        else if constexpr (s_hasIndexOfWorldMatrixType) {
            return (*m_worldMatrixContainer.m_worldMatrices)[m_worldMatrixContainer.m_index];
        }
        else{
            static_assert(false, "Invalid template type");
        }
    }

    TransformMatrices& operator=(const TransformMatrices& other) {
        m_localMatrix = other.m_localMatrix;
        m_worldMatrixContainer = other.m_worldMatrixContainer;
        return *this;
    }

    Matrix4x4 m_localMatrix; ///< Local model matrix
    WorldMatrixType m_worldMatrixContainer; ///< Global model matrix
};

template<>
struct TransformMatrices<Matrix4x4> {
    inline Matrix4x4& worldMatrix() {
        return m_worldMatrix;
    }

    inline const Matrix4x4& worldMatrix() const {
        return m_worldMatrix;
    }

    Matrix4x4 m_localMatrix; ///< Local model matrix
    Matrix4x4 m_worldMatrix; ///< Global model matrix
};

typedef TransformMatrices<WorldMatrixVector> IndexedTransformMatrices;

/// @class TransformInterface
/// @brief Base class for a transform
class TransformInterface {
public:

    /// @brief Enum for which transforms to inherit from a parent state
    enum InheritanceType {
        kAll, // inherit entire parent world matrix
        kTranslation, // inherit only the translation of the world state (e.g. won't orbit parent)
        kPreserveOrientation // preserves the local orientation of this transform 
    };

    TransformInterface();
    TransformInterface(const TransformInterface& other);
    virtual ~TransformInterface();

    TransformInterface& operator=(const TransformInterface& other);

    virtual const Matrix4x4& worldMatrix() const = 0;

    InheritanceType inheritanceType() const { return m_inheritanceType; }
    void setInheritanceType(InheritanceType type) { m_inheritanceType = type; }

    /// @brief Update the world matrix for the transform
    virtual void computeWorldMatrix() = 0;

    /// @brief Update the world matrix of this transform, given a local matrix
    virtual void updateWorldMatrixWithLocal(const Matrix4x4& localMatrix) = 0;

    virtual const Vector3& getPosition() const = 0;
    virtual const Vector3& getScaleVec() const = 0;
    virtual const Quaternion& getRotationQuaternion() const = 0;

    /// @brief Obtain the parent to this transform
    TransformInterface* parentTransform() const {
        return m_parent;
    }

    void setParent(TransformInterface* p);

    void clearParent(bool recomputeWorld);

    /// @brief Whether the specified child is a child of this transform
    /// \param[in] child    The candidate child transform
    /// \param[out] idx     The index of the child transform, if it is a child
    /// \return Whether or not the specified transform is a child
    bool hasChild(const TransformInterface& child, size_t* idx = nullptr);

    /// @brief Add and remove children from this transform
    void addChild(TransformInterface* c);
    void removeChild(TransformInterface* c);

protected:

    /// @brief Set the unique ID of the transform
    void setUniqueId() {
        static std::mutex s_mutex;
        std::unique_lock lock(s_mutex);
        m_id = s_count;
        s_count++;
    }

    Uint64_t m_id{ 0 }; ///< The unique ID for the transform
    InheritanceType m_inheritanceType{ InheritanceType::kAll }; ///< The type of geometry inheritance for this transform
    TransformInterface* m_parent{ nullptr }; ///< The parent of the transform

    std::vector<TransformInterface*> m_children; ///< The children of the transform
    static Uint64_t s_count; ///< The number of transforms created
};

/// @class TransformTemplate
/// @brief A transform consisting of a translation, a rotation, and a scaling
template<typename WorldMatrixType = Matrix4x4>
class TransformTemplate: public TransformInterface {
private:
    static constexpr bool s_hasIndexOfWorldMatrixType = std::is_same_v<WorldMatrixType, WorldMatrixVector>;
    static_assert(std::is_same_v<std::remove_pointer_t<WorldMatrixType>, Matrix4x4>
        || s_hasIndexOfWorldMatrixType, "Type must be a 4x4 matrix or a pointer to one");
    static constexpr bool s_hasPointerWorldMatrixType = std::is_pointer_v<WorldMatrixType>;

    using ThisType = TransformTemplate<WorldMatrixType>;

public:

    /// @name Static
    /// @{

    static const TransformTemplate& Identity() { return s_identity; }

    /// @}

    /// @name Constructors/Destructor
    /// @{

    TransformTemplate() {
    }
    
    TransformTemplate(const Vector3& position, const Quaternion& rotation, const Vector3&scale, bool updateLocal = false) :
        m_translation(position),
        m_rotation(this, rotation),
        m_scale(this, scale)
    {
        if (updateLocal) {
            computeLocalMatrix();
        }
    }
    
    TransformTemplate(const nlohmann::json& json)
    {
        from_json(json, *this);
    }

    TransformTemplate(const physx::PxTransform& pxTransform)
    {

    }

    TransformTemplate(const Matrix4x4& localMatrix)
    {
        if constexpr (s_hasPointerWorldMatrixType || s_hasIndexOfWorldMatrixType) {
            static_assert(false, "Invalid constructor for Transform<Matrix4x4*>");
        }
        else {
            decompose(localMatrix);
        }
    }

    /// @brief Initialize a PointerTransform with a world matrix to modify during any operations
    /// @param[in] worldMatrix A pointer to the matrix that this transform will modify. The matrix must outlive the transform
    TransformTemplate(Matrix4x4* worldMatrix) :
        m_matrices(worldMatrix)
    {
        static_assert(s_hasPointerWorldMatrixType, "Constructor only valid for Transform<Matrix4x4*>");
    }

    /// @brief Initialize an IndexedTransform with a vector of world matrices to modify during any operations
    /// @param[in] std::vector<Matrix4x4> A vector containing the matrix that this transform will modify. The vector must outlive the transform
    /// @param[in] index The index of the matrix to modify
    TransformTemplate(std::vector<Matrix4x4>& worldMatrixVec, Uint32_t index) :
        m_matrices(worldMatrixVec, index)
    {
        static_assert(s_hasIndexOfWorldMatrixType, "Constructor only valid for Transform<WorldMatrixVector>");
    }

    TransformTemplate(const TransformTemplate& other) :
        TransformInterface(other),
        m_translation(other.m_translation),
        m_rotation(other.m_rotation),
        m_scale(other.m_scale),
        m_matrices(other.m_matrices)
    {
    }

    virtual ~TransformTemplate() {
    }

    /// @}

    /// @name Operators
    /// @{

    TransformTemplate& operator=(const TransformTemplate& other)
    {
        TransformInterface::operator=(other);

        m_translation = other.m_translation;
        m_rotation = other.m_rotation;
        m_scale = other.m_scale;
        m_matrices = other.m_matrices;

        return *this;
    }


    /// @}

    /// @name Properties
    /// @{    

    /// @brief Translation component
    TranslationComponent<WorldMatrixType>& translation() { return m_translation; }
    const Vector3& getPosition() const override { return m_translation.getPosition(); }
    //const Vector3& getVelocity() const { return m_translation.getVelocity(); }

    /// @brief Rotation component
    RotationComponent<WorldMatrixType>& rotation() { return m_rotation; }
    const Quaternion& getRotationQuaternion() const override { return m_rotation.getQuaternion(); }

    /// @brief Scale component
    ScaleComponent<WorldMatrixType>& scale() { return m_scale; }
    const Vector3& getScaleVec() const override { return m_scale.getScale(); }

    /// @brief Local model matrix
    const Matrix4x4& localMatrix() const { return m_matrices.m_localMatrix; }

    /// @brief Global model matrix
    const Matrix4x4& worldMatrix() const override { return m_matrices.worldMatrix(); }

    /// @}

    /// @name Public methods
    /// @{    

    /// @brief Set scaling vector and update transform matrix
    void setScale(const Vector3& scaling, bool updateTransform = true) {
        m_scale.setScale(this, scaling, updateTransform);
    }

    void setScale(double x, double y, double z, bool updateTransform = true) {
        m_scale.setScale(this, x, y, z, updateTransform);
    }

    void setScale(double scaling, bool updateTransform = true) {
        m_scale.setScale(this, scaling, updateTransform);
    }

    /// @brief Add a rotation corresponding to the given euler angles to this component
    void addRotation(const EulerAngles& eulerAngles, bool updateTransform = true) {
        m_rotation.addRotation(this, eulerAngles, updateTransform);
    }

    /// @brief Set rotation using quaternion and update rotation matrix
    void setRotation(const Quaternion& quaternion, bool updateTransform = true) {
        m_rotation.setRotation(this, quaternion, updateTransform);
    }

    /// @brief Set rotation using euler angles and update rotation matrix
    void setRotation(const EulerAngles& eulerAngles, bool updateTransform = true) {
        m_rotation.setRotation(this, eulerAngles, updateTransform);
    }

    /// @brief Sets the translation and updates the translation matrix
    void setTranslation(const Translation& position, bool updateTransform = true) {
        m_translation.setTranslation(this, position, updateTransform);
    }

    void setTranslation(const Vector3& position, bool updateTransform = true) {
        m_translation.setTranslation(this, position, updateTransform);
    }

    /// @brief Return the world position of the transform
    Vector3 worldPosition() const {
        return Vector3(m_matrices.worldMatrix().getColumn(3));
    }

    /// @brief Set the local transform such that it has the given world position
    void setWorldPosition(const Vector3& worldPos) {
        if (!m_parent) {
            // No parent, so world position is local position
            m_translation.setPosition(this, worldPos, true);
        }
        else {
            // Get world matrix that would give specified world position, and multiply by inverse of parent world matrix to get correct local matrix
            Matrix4x4 desiredWorld = m_matrices.worldMatrix();
            desiredWorld.column(3) = Vector4(worldPos, 1.0);
            Matrix4x4 desiredLocal = m_parent->worldMatrix().inversed() * desiredWorld;
            m_translation.setPosition(this, desiredLocal.getColumn(3));
        }
    }
    void setPosition(const Vector3& p, bool updateTransform = true) {
        m_translation.setPosition(this, p, updateTransform);
    }


    /// @brief Rotate about the given axis, in radians
    void rotateAboutAxis(const Vector3& axis, float angle) {
        Quaternion rotation = Quaternion::fromAxisAngle(axis, angle);
        m_rotation.setRotation(this, rotation * m_rotation.getQuaternion());
    }
    
    /// @brief Update the transform manually 
    /// @details Called to avoid redundant matrix multiplications on component modification
    void updateLocalMatrix() {
        m_rotation.computeTransformMatrix(false);
        m_scale.computeTransformMatrix(false);
        computeLocalMatrix();
    }

    void updateWorldMatrix() {
        m_rotation.computeTransformMatrix(false);
        m_scale.computeTransformMatrix(false);
        computeWorldMatrix();
    }


    /// @brief Update the world matrix with a local matrix
    void updateWorldMatrixWithLocal(const Matrix4x4& localMatrix) override {
        decompose(localMatrix);
    }

    /// @brief Compute world matrix
    void computeWorldMatrix() override {
        computeThisWorldMatrix();

        // Update all child states
        for (const auto& child : m_children) {
            child->computeWorldMatrix();
        }
    }

    /// @brief Obtain the translation in world space
    TranslationComponent<WorldMatrixType> worldTranslation() const {
        return TranslationComponent<WorldMatrixType>(m_matrices.worldMatrix().getTranslationVector().asReal());
    }

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json<WorldMatrixType>(nlohmann::json& orJson, const TransformTemplate<WorldMatrixType>& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json<WorldMatrixType>(const nlohmann::json& korJson, TransformTemplate<WorldMatrixType>& orObject);

    /// @}

protected:
    /// @name Friends 
    /// @{

    friend class AbstractTransformComponent;
    friend class AffineComponent;
    friend class TranslationComponent<WorldMatrixType>;
    friend class SceneObject;
    friend class BlendQueue;

    template<typename TransformType> 
    friend class TransformTemplate;
    friend class TransformComponent;

    /// @}

    /// @name Protected methods
    /// @{  

    /// @brief Compute local matrix
    void computeLocalMatrix() {
        // Multiply by transformations in reverse of their application order
        // e.g.  Want to apply scaling, then rotation, then translation
        m_matrices.m_localMatrix.setToIdentity();

        // Add rotations to the model matrix
        if (!m_rotation.getQuaternion().isIdentity()) {
            m_matrices.m_localMatrix *= m_rotation.getMatrix();
        }

        // Add scaling to model matrix
        if (m_scale.m_scale.x() != 1.0) {
            m_matrices.m_localMatrix.column(0) *= m_scale.m_scale.x();
        }
        if (m_scale.m_scale.y() != 1.0) {
            m_matrices.m_localMatrix.column(1) *= m_scale.m_scale.y();
        }
        if (m_scale.m_scale.z() != 1.0) {
            m_matrices.m_localMatrix.column(2) *= m_scale.m_scale.z();
        }

        // Add translation without full matrix multiplication
        if (!m_translation.isIdentity()) {
            m_matrices.m_localMatrix.setTranslation(m_translation.getPosition());
        }
    }

    /// @brief Compute the world matrix for this transform, without updating children
    void computeThisWorldMatrix() {
        // Compute local matrix
        computeLocalMatrix();

        // Initialize model matrix using model matrices of parent as a base
        if (!m_parent) {
            m_matrices.worldMatrix() = m_matrices.m_localMatrix;
        }
        else {
            switch (m_inheritanceType) {
            case TransformTemplate::kTranslation:
                m_matrices.worldMatrix().setToIdentity();
                m_matrices.worldMatrix().setTranslation(parentTransform()->worldMatrix().getColumn(3));
                m_matrices.worldMatrix() *= m_matrices.m_localMatrix;
                break;
            case TransformTemplate::kPreserveOrientation:
            {
                // Obtain world translation by removing the rotation component of the world matrix
                Matrix4x4 localTranslation;
                localTranslation.setTranslation(m_translation.getPosition());
                Matrix4x4 translation = parentTransform()->worldMatrix() * localTranslation;
                translation.toTranslationMatrix();

                // Compute world matrix that preserves original orientation of state w.r.t. inertial frame
                m_matrices.worldMatrix() = translation * m_rotation.getMatrix() * Matrix4x4::ScaleMatrix(m_scale.m_scale);
                break;
            }
            case TransformTemplate::kAll:
            default:
                // Compute world matrix
                m_matrices.worldMatrix() = parentTransform()->worldMatrix();
                m_matrices.worldMatrix() *= m_matrices.m_localMatrix;
                break;
            }
        }
    }

    /// @brief Decompose matrix into scale, rotation, translation
    void decompose(const Matrix4x4& mat) {
        Vector3 translation = mat.getTranslationVector();
        Vector3 scale = Vector3(mat.getColumn(0).length(), mat.getColumn(1).length(), mat.getColumn(2).length());
        Vector3 inverseScale = Vector3(1.0 / scale.x(), 1.0 / scale.y(), 1.0 / scale.z());
        Matrix4x4 matCopy(mat.getData());
        matCopy.addScale(inverseScale);
        Quaternion rotation = Quaternion::fromRotationMatrix(matCopy);

        m_translation.setPosition(this, translation, false);
        m_scale.setScale(this, scale, false);
        m_rotation.setRotation(this, rotation, false);
        m_matrices.m_localMatrix = mat.getData();
        if (!m_parent) {
            m_matrices.worldMatrix() = m_matrices.m_localMatrix;
        }
        else {
            computeWorldMatrix();
        }
    }

    /// @}

    /// @name Protected members
    /// @{    

    TranslationComponent<WorldMatrixType> m_translation; ///< The translational component of the transform
    RotationComponent<WorldMatrixType> m_rotation; ///< The rotational component of the transform
    ScaleComponent<WorldMatrixType> m_scale; ///< The scaling component of the transform
    TransformMatrices<WorldMatrixType> m_matrices; ///< The local and world matrices of the transform
    static TransformTemplate s_identity; ///< The identity transform

    /// @}

};

template<typename WorldMatrixType>
void to_json<WorldMatrixType>(nlohmann::json& orJson, const TransformTemplate<WorldMatrixType>& korObject) {
    orJson["translation"] = korObject.m_translation;
    orJson["rotation"] = korObject.m_rotation;
    orJson["scale"] = korObject.m_scale;
    orJson["inheritanceType"] = int(korObject.m_inheritanceType);
}

template<typename WorldMatrixType>
void from_json<WorldMatrixType>(const nlohmann::json& korJson, TransformTemplate<WorldMatrixType>& orObject) {
    /// \note Was seeing failures with recognizing to_json/from_json
    /// \see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4381.html
    const json& translationJson = korJson.at("translation");
    orObject.m_translation.setTranslation(&orObject, translationJson.at("translation").get<Translation>(), false);

    const json& rotationJson = korJson.at("rotation");
    orObject.m_rotation.setRotation(&orObject, rotationJson.at("quaternion").get<Quaternion>(), false);

    const json& scaleJson = korJson.at("scale");
    orObject.m_scale.setScale(&orObject, scaleJson.at("scaling").get<Vector3>(), false);

    orObject.m_inheritanceType = TransformInterface::InheritanceType(korJson.at("inheritanceType").get<Int32_t>());

    orObject.computeWorldMatrix();
}

template<typename WorldMatrixType>
TransformTemplate<WorldMatrixType> TransformTemplate<WorldMatrixType>::s_identity = TransformTemplate<WorldMatrixType>();

using Transform = TransformTemplate<Matrix4x4>;
using PointerTransform = TransformTemplate<Matrix4x4*>;
using IndexedTransform = TransformTemplate<WorldMatrixVector>;

} // end namespacing
