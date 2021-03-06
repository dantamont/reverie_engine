///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GTransform.h"

// Standard Includes

// External

// Project
#include "../scene/GSceneObject.h"
//#include "../GCoreEngine.h"
//#include "../events/GMessenger.h"
#include "../components/GCameraComponent.h"
#include "../utils/GInterpolation.h"
#include "../physics/GPhysicsManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace physx;

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Implementations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Transform
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
Transform Transform::s_identity = Transform();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform Transform::interpolate(const Transform & t1, const Transform & t2, float percentFactor, bool updateWorld)
{
    Quaternion quat = Quaternion::Slerp(t1.m_rotation.getQuaternion(), 
        t2.m_rotation.getQuaternion(), percentFactor);
    Vector3 translation = Interpolation::lerp(t1.m_translation.getPosition(),
        t2.m_translation.getPosition(), percentFactor);
    Vector3 scale = Interpolation::lerp(t1.m_scale.getScale(),
        t2.m_scale.getScale(), percentFactor);

    Transform result;
    result.setPosition(translation, false);
    result.setRotation(quat, false);
    result.setScale(scale, false);
    if (updateWorld) {
        result.updateWorldMatrix();
    }
    else {
        result.updateLocalMatrix();
    }

    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform Transform::interpolate(const std::vector<Transform>& transforms, const std::vector<float>& weights,
    bool updateWorld)
{
    // Compile all translations, rotations, scales
    size_t size = transforms.size();
    std::vector<Vector3> translations;
    translations.reserve(size);
    std::vector<Quaternion> rotations;
    rotations.reserve(size);
    std::vector<Vector3> scales;
    scales.reserve(size);
    for (size_t i = 0; i < size; i++) {
        const Transform& t = transforms[i];
        Vec::EmplaceBack(translations, t.getPosition());
        Vec::EmplaceBack(rotations, t.getRotationQuaternion());
        Vec::EmplaceBack(scales, t.getScaleVec());
    }

    // Interpolate translation, rotation, scale
    //Vector3 translation = Interpolation::lerp(translations, weights);
    //Quaternion rotation = Quaternion::slerp(rotations, weights);
    //Vector3 scale = Interpolation::lerp(scales, weights);

    // Get interpolated transform
    Transform result(Interpolation::lerp(translations, weights), 
        Quaternion::EigenAverage(rotations.data(), weights), 
        Interpolation::lerp(scales, weights));
    //result.translation().setPosition(translation, false);
    //result.rotation().setRotation(rotation, false);
    //result.scale().setScale(scale, false);
    if (updateWorld) {
        result.updateWorldMatrix();
    }
    else {
        result.updateLocalMatrix();
    }

    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform::Transform():
    m_inheritanceType(kAll)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform::Transform(const Vector3 & position, const Quaternion & rotation, const Vector3 & scale, bool updateLocal) :
    m_inheritanceType(kAll),
    m_translation(position),
    m_rotation(this, rotation),
    m_scale(this, scale)
{
    if (updateLocal) {
        computeLocalMatrix();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform::Transform(const QJsonValue & json)
{
    loadFromJson(json);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform& Transform::operator=(const Transform& other)
{
    Identifiable::operator=(other);
    Serializable::operator=(other);

    m_inheritanceType = other.m_inheritanceType;
    m_translation = other.m_translation;
    m_rotation = other.m_rotation;
    m_scale = other.m_scale;
    m_matrices = other.m_matrices;

    if (other.m_parent) {
        m_parent = other.m_parent;
    }

    // Don't support copying with children, since transform no longer owns children
    if (m_children.size()) {
        throw("Error, copy not supported with transform that has children");
    }

    // Duplicate all children to avoid double deletion
    //for (Transform* child: other.m_children) {
    //    Transform* newChild = new Transform(*child);
    //    addChild(newChild);
    //}

    return *this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform::Transform(const physx::PxTransform & pxTransform) :
    m_inheritanceType(InheritanceType::kAll)
{
    const PxVec3& pos = pxTransform.p;
    const PxQuat& quat = pxTransform.q;
    m_translation.setPosition(this, PhysicsManager::toVector3(pos), false);
    m_rotation.setRotation(this, PhysicsManager::toQuaternion(quat), false);

    computeWorldMatrix();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Transform::Transform(const PxTransform & pxTransform, const physx::PxVec3& velocity):
//    m_inheritanceType(InheritanceType::kAll)
//{
//    initialize();
//
//    const PxVec3& pos = pxTransform.p;
//    const PxQuat& quat = pxTransform.q;
//    m_translation.setPosition(PhysicsManager::toVec3(pos), false);
//    m_translation.setVelocity(PhysicsManager::toVec3(velocity), false);
//    m_rotation.setRotation(PhysicsManager::toQuaternion(quat), false);
//
//    computeWorldMatrix();
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform::Transform(const Matrix4x4g & localMatrix) :
    m_inheritanceType(kAll)
{
    decompose(localMatrix);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform::Transform(const Transform & other):
    m_inheritanceType(other.m_inheritanceType),
    m_translation(other.m_translation),
    m_rotation(other.m_rotation),
    m_scale(other.m_scale),
    m_matrices(other.m_matrices)
{
    if (other.m_parent) {
        m_parent = other.m_parent;
    }

    // Don't support copying with children, since transform no longer owns children
    if (m_children.size()) {
        throw("Error, copy not supported with transform that has children");
    }

    // Need to duplicate all children to avoid double deletion
    //for (Transform* child : other.m_children) {
    //    Transform* newChild = new Transform(*child);
    //    addChild(newChild);
    //}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform::~Transform()
{
    // Remove from parent
    if (m_parent) {
        m_parent->removeChild(this);
    }

    // No longer owns children! Since transforms may not live as raw pointers
    // Delete children 
    //for (Transform* child : m_children) {
    //    delete child;
    //}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4g Transform::rotationTranslationMatrix() const
{
    // Remove scaling from rotational component of matrix
    const Vector3& scale = m_scale.getScale();
    Matrix3x3g rotation(m_matrices.m_worldMatrix);
    rotation.normalizeColumns();

    // Construct matrix without scaling, but with translation
    Matrix4x4g rotTrans(rotation);
    rotTrans.setTranslation(m_matrices.m_worldMatrix.getTranslationVector());
    return rotTrans;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Transform::setScale(const Vector3 & scaling, bool updateTransform)
{
    m_scale.setScale(this, scaling, updateTransform);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Transform::setScale(double x, double y, double z, bool updateTransform)
{
    m_scale.setScale(this, x, y, z, updateTransform);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Transform::setScale(double scaling, bool updateTransform)
{
    m_scale.setScale(this, scaling, updateTransform);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Transform::addRotation(const EulerAngles & eulerAngles, bool updateTransform)
{
    m_rotation.addRotation(this, eulerAngles, updateTransform);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Transform::setRotation(const Quaternion & quaternion, bool updateTransform)
{
    m_rotation.setRotation(this, quaternion, updateTransform);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Transform::setRotation(const EulerAngles & eulerAngles, bool updateTransform)
{
    m_rotation.setRotation(this, eulerAngles, updateTransform);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Transform::setTranslation(const Translation & position, bool updateTransform)
{
    m_translation.setTranslation(this, position, updateTransform);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Transform::setTranslation(const Vector3 & position, bool updateTransform)
{
    m_translation.setTranslation(this, position, updateTransform);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Vector3 Transform::worldPosition() const
{
    return Vector3(m_matrices.m_worldMatrix.getColumn(3));
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Transform::setWorldPosition(const Vector3 & worldPos)
{
    if (!m_parent) {
        // No parent, so world position is local position
        m_translation.setPosition(this, worldPos, true);
    }
    else {
        // Get world matrix that would give specified world position, and multiply by inverse of parent world matrix to get correct local matrix
        Matrix4x4 desiredWorld = m_matrices.m_worldMatrix;
        desiredWorld.column(3) = Vector4(worldPos, 1.0);
        Matrix4x4 desiredLocal = m_parent->worldMatrix().inversed() * desiredWorld;
        m_translation.setPosition(this, desiredLocal.getColumn(3));
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Transform::setPosition(const Vector3 & p, bool updateTransform)
{
    m_translation.setPosition(this, p, updateTransform);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Transform::hasChild(const Transform & child, size_t * idx)
{
    const Uuid& uuid = child.getUuid();
    auto iter = std::find_if(m_children.begin(), m_children.end(),
        [&](Transform* t) {
        return t->getUuid() == uuid;
    });

    if (iter != m_children.end()) {
        if (idx) {
            *idx = iter - m_children.begin();
        }
        return true;
    }
    else {
        return false;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::rotateAboutAxis(const Vector3 & axis, float angle)
{
    Quaternion rotation = Quaternion::fromAxisAngle(axis, angle);
    m_rotation.setRotation(this, rotation * m_rotation.getQuaternion());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
physx::PxTransform Transform::asPhysX() const
{
    const Vector3& position = m_translation.getPosition();
    physx::PxQuat quat = m_rotation.getQuaternion().asPhysX();
    return physx::PxTransform(position.x(), position.y(), position.z(), quat);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::updateLocalMatrix()
{
    m_rotation.computeTransformMatrix(false);
    m_scale.computeTransformMatrix(false);
    //m_translation.computeTransformMatrix(false);
    computeLocalMatrix();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::updateWorldMatrix()
{
    m_rotation.computeTransformMatrix(false);
    m_scale.computeTransformMatrix(false);
    //m_translation.computeTransformMatrix(false);
    computeWorldMatrix();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::updateWorldMatrix(const Matrix4x4g & matrix)
{
    decompose(matrix);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform* Transform::parentTransform() const
{
    //auto so = sceneObject();
    //if (so->numParents() > 1) {
    //    throw("Error, transform may only have one parent");
    //}
    //
    //if (so->numParents() == 1) {
    //    std::weak_ptr<DagNode> weakParent = so->m_parents.begin()->second;
    //    std::shared_ptr<DagNode> parent = weakParent.lock();
    //    if (parent) {
    //        auto parentObject = std::static_pointer_cast<SceneObject>(parent);
    //        return parentObject->transform();
    //    }
    //}

    //return nullptr;

    return m_parent;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::setParent(Transform* p)
{
    // Clear old parent
    if (m_parent) {
        m_parent->removeChild(this);
    }

    // Set parent and refresh world matrix
    m_parent = p;
    p->addChild(this);
    computeWorldMatrix();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::clearParent(bool recomputeWorld)
{
    if (m_parent) {
        m_parent->removeChild(this);
        m_parent = nullptr;
        if (recomputeWorld) {
            computeWorldMatrix();
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::addChild(Transform * c)
{
#ifdef DEBUG_MODE
    if (hasChild(*c)) {
        throw("Error, transform already has the given child");
    }
#endif
    m_children.push_back(c);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::removeChild(Transform * c)
{
    size_t idx;
    if (!hasChild(*c, &idx)) {
        throw("Error, transform does not have the specified child");
    }
    m_children.erase(m_children.begin() + idx);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TranslationComponent Transform::worldTranslation() const
{
    return TranslationComponent(m_matrices.m_worldMatrix.getTranslationVector().asReal());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Transform::asJson(const SerializationContext& context) const
{
    QJsonObject object;
    object.insert("translation", m_translation.asJson());
    object.insert("rotation", m_rotation.asJson());
    object.insert("scale", m_scale.asJson());
    object.insert("inheritanceType", int(m_inheritanceType));

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    const QJsonObject& object = json.toObject();
    m_translation.loadFromJson(object.value("translation"));
    m_rotation.loadFromJson(object.value("rotation"));
    m_scale.loadFromJson(object.value("scale"));
    m_inheritanceType = Transform::InheritanceType(object.value("inheritanceType").toInt());

    computeWorldMatrix();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::computeLocalMatrix()
{
    // Multiply by transformations in reverse of their application order
    // e.g.  Want to apply scaling, then rotation, then translation
    m_matrices.m_localMatrix.setToIdentity();

    // Add rotations to the model matrix
    if (!m_rotation.getQuaternion().isIdentity()) {
        m_matrices.m_localMatrix *= m_rotation.getMatrix();
    }

    // Add scaling to model matrix
    if (!m_scale.isIdentity()) {
        m_matrices.m_localMatrix *= m_scale.getMatrix();
    }

    // Add translation without full matrix multiplication
    if (!m_translation.isIdentity()) {
        m_matrices.m_localMatrix.setTranslation(m_translation.getPosition());
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::computeThisWorldMatrix()
{
    // Compute local matrix
    computeLocalMatrix();

    // Initialize model matrix using model matrices of parent as a base
    if (!m_parent) {
        m_matrices.m_worldMatrix = m_matrices.m_localMatrix;
    }
    else {
        m_matrices.m_worldMatrix.setToIdentity();
        switch (m_inheritanceType) {
        case Transform::kTranslation:
            m_matrices.m_worldMatrix.setTranslation(parentTransform()->worldTranslation().getPosition());
            //m_worldMatrix = parentTransform()->worldTranslation().getMatrix();
            break;
        case Transform::kAll:
        case Transform::kPreserveOrientation:
        default:
            m_matrices.m_worldMatrix = parentTransform()->worldMatrix();
            break;
        }

        // Construct model matrix for each inheritance type
        switch (m_inheritanceType) {
        case Transform::kPreserveOrientation:
        {
            // Obtain world translation by removing the rotation component of the world matrix
            Matrix4x4 localTranslation;
            localTranslation.setTranslation(m_translation.getPosition());
            Matrix4x4 translation = m_matrices.m_worldMatrix * localTranslation;
            translation = translation.getTranslationMatrix();

            // Compute world matrix that preserves original orientation of state w.r.t. inertial frame
            m_matrices.m_worldMatrix = translation * m_rotation.getMatrix() * m_scale.getMatrix();
        }
        case Transform::kTranslation:
        case Transform::kAll:
        default:
            // Compute world matrix
            m_matrices.m_worldMatrix *= m_matrices.m_localMatrix;
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::decompose(const Matrix4x4g & mat)
{
    Vector3 translation = mat.getTranslationVector();
    Vector3 scale = Vector3(mat.getColumn(0).length(), mat.getColumn(1).length(), mat.getColumn(2).length());
    Vector3 inverseScale = Vector3(1.0 / scale.x(), 1.0 / scale.y(), 1.0 / scale.z());
    Matrix4x4g matCopy(mat.getData());
    matCopy.addScale(inverseScale);
    Quaternion rotation = Quaternion::fromRotationMatrix(matCopy);
    
    m_translation.setPosition(this, translation, false);
    m_scale.setScale(this, scale, false);
    m_rotation.setRotation(this, rotation, false);
    m_matrices.m_localMatrix = Matrix4x4g(mat.getData());
    if (!m_parent) {
        m_matrices.m_worldMatrix = Matrix4x4g(mat.getData());
    }
    else {
        computeWorldMatrix();
    }
    //computeLocalMatrix();

    //QString str = QString(mat - m_localMatrix);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::computeWorldMatrix()
{
    computeThisWorldMatrix();

    // Update all child states
    for (const auto& child: m_children) {
        //auto childObject = std::static_pointer_cast<SceneObject>(childPair.second);
        //auto childTransform = childObject->transform();
        child->computeWorldMatrix();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing