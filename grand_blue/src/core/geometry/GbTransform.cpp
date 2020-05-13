///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GbTransform.h"

// Standard Includes

// External

// Project
#include "../scene/GbSceneObject.h"
//#include "../GbCoreEngine.h"
//#include "../events/GbMessenger.h"
#include "../components/GbCamera.h"
#include "../components/GbLight.h"
#include "../utils/GbInterpolation.h"
#include "../physics/GbPhysicsManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace physx;

namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Implementations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Transform
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform Transform::interpolate(const Transform & t1, const Transform & t2, float percentFactor, bool updateWorld)
{
    Quaternion quat = Quaternion::slerp(t1.m_rotation.getQuaternion(), 
        t2.m_rotation.getQuaternion(), percentFactor);
    Vector3 translation = Interpolation::lerp(t1.m_translation.getPosition(),
        t2.m_translation.getPosition(), percentFactor);
    Vector3 scale = Interpolation::lerp(t1.m_scale.getScale(),
        t2.m_scale.getScale(), percentFactor);

    Transform result;
    result.translation().setPosition(translation, false);
    result.rotation().setRotation(quat, false);
    result.scale().setScale(scale, false);
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
        Quaternion::slerp(rotations, weights), 
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
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform::Transform(const Vector3 & position, const Quaternion & rotation, const Vector3 & scale, bool updateLocal) :
    m_inheritanceType(kAll),
    m_translation(position),
    m_rotation(rotation),
    m_scale(scale)
{
    initialize();
    if (updateLocal)
        computeLocalMatrix();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform::Transform(const QJsonValue & json)
{
    loadFromJson(json);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform& Transform::operator=(const Transform& other)
{
    Object::operator=(other);
    Serializable::operator=(other);

    m_parent = other.m_parent;
    for (const std::pair<Uuid, Transform*>& child: other.m_children) {
        Transform* newChild = new Transform(*child.second);
        addChild(newChild);
    }
    m_translation = other.m_translation;
    m_rotation = other.m_rotation;
    m_scale = other.m_scale;
    m_inheritanceType = other.m_inheritanceType;

    initialize();
    computeWorldMatrix();

    return *this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform::Transform(const physx::PxTransform & pxTransform) :
    m_inheritanceType(InheritanceType::kAll)
{
    initialize();

    const PxVec3& pos = pxTransform.p;
    const PxQuat& quat = pxTransform.q;
    m_translation.setPosition(PhysicsManager::toVec3(pos), false);
    m_rotation.setRotation(PhysicsManager::toQuaternion(quat), false);

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
    initialize();
    decompose(localMatrix);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform::Transform(const Transform & other):
    m_inheritanceType(other.m_inheritanceType),
    m_translation(other.m_translation),
    m_rotation(other.m_rotation),
    m_scale(other.m_scale),
    m_localMatrix(other.m_localMatrix),
    m_worldMatrix(other.m_worldMatrix)
{
    initialize();

    if (other.m_parent) {
        m_parent = other.m_parent;
    }

    // Need to duplicate all children to avoid double deletion
    for (const std::pair<Uuid, Transform*>& child : other.m_children) {
        Transform* newChild = new Transform(*child.second);
        addChild(newChild);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Transform::~Transform()
{
    // Delete children 
    for (const std::pair<Uuid, Transform*>& childPair : m_children) {
        delete childPair.second;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4g Transform::rotationTranslationMatrix() const
{
    // Remove scaling from rotational component of matrix
    Vector3g scale = m_scale.getScale().asReal();
    Matrix3x3g rotation(m_worldMatrix);
    rotation.normalizeColumns();

    // Construct matrix without scaling, but with translation
    Matrix4x4g rotTrans(rotation);
    rotTrans.setTranslation(m_translation.getPosition().asReal());
    return rotTrans;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::rotateAboutAxis(const Vector3g & axis, float angle)
{
    Quaternion rotation = Quaternion::fromAxisAngle(axis, angle);
    m_rotation.setRotation(rotation * m_rotation.getQuaternion());
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
void Transform::clearParent()
{
    if (m_parent) {
        m_parent->removeChild(this);
        m_parent = nullptr;
        computeWorldMatrix();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::addChild(Transform * c)
{
    const Uuid& id = c->getUuid();
#ifdef DEBUG_MODE
    if (Map::HasKey(m_children, id)) {
        throw("Error, transform already has the given child");
    }
#endif
    m_children[id] = c;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::removeChild(Transform * c)
{
    const Uuid& id = c->getUuid();
#ifdef DEBUG_MODE
    if (!Map::HasKey(m_children, id)) {
        throw("Error, transform does not have the specified child");
    }
#endif
    m_children.erase(id);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TranslationComponent Transform::worldTranslation() const
{
    return TranslationComponent(m_worldMatrix.getTranslationVector().asDouble());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Transform::asJson() const
{
    QJsonObject object;
    object.insert("translation", m_translation.asJson());
    object.insert("rotation", m_rotation.asJson());
    object.insert("scale", m_scale.asJson());
    object.insert("inheritanceType", int(m_inheritanceType));

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();
    m_translation.loadFromJson(object.value("translation"));
    m_rotation.loadFromJson(object.value("rotation"));
    m_scale.loadFromJson(object.value("scale"));
    m_inheritanceType = Transform::InheritanceType(object.value("inheritanceType").toInt());

    computeWorldMatrix();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::initialize()
{
    m_translation.setTransform(this);
    m_rotation.setTransform(this);
    m_scale.setTransform(this);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::computeLocalMatrix()
{
    // Multiply by transformations in reverse of their application order
    // e.g.  Want to apply scaling, then rotation, then translation
    m_localMatrix.setToIdentity();

    // Add rotations to the model matrix
    if(!m_rotation.getQuaternion().isIdentity())
        m_localMatrix *= m_rotation.getMatrix();

    // Add scaling to model matrix
    if(!m_scale.isIdentity())
        m_localMatrix *= m_scale.getMatrix();

    // Add translation without full matrix multiplication
    if (!m_translation.isIdentity()) {
        m_localMatrix.setTranslation(m_translation.getPosition().asReal());
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Transform::decompose(const Matrix4x4g & mat)
{
    Vector3g translation = mat.getTranslationVector();
    Vector3g scale = Vector3g(mat.getColumn(0).length(), mat.getColumn(1).length(), mat.getColumn(2).length());
    Vector3g inverseScale = Vector3g(1.0 / scale.x(), 1.0 / scale.y(), 1.0 / scale.z());
    Matrix4x4g matCopy(mat.getData());
    matCopy.addScale(inverseScale);
    Quaternion rotation = Quaternion::fromRotationMatrix(matCopy);
    
    m_translation.setPosition(translation.asDouble(), false);
    m_scale.setScale(scale.asDouble(), false);
    m_rotation.setRotation(rotation, false);
    m_localMatrix = Matrix4x4g(mat.getData());
    if (!m_parent) {
        m_worldMatrix = Matrix4x4g(mat.getData());
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
    // Compute local matrix
    computeLocalMatrix();

    // Initialize model matrix using model matrices of parent as a base
    if (!m_parent) {
        m_worldMatrix = m_localMatrix;
    }
    else {
        m_worldMatrix.setToIdentity();
        switch (m_inheritanceType) {
        case Transform::kTranslation:
            m_worldMatrix.setTranslation(parentTransform()->worldTranslation().getPosition().asReal());
            //m_worldMatrix = parentTransform()->worldTranslation().getMatrix();
            break;
        case Transform::kAll:
        case Transform::kPreserveOrientation:
        default:
            m_worldMatrix = parentTransform()->worldMatrix();
            break;
        }

        // Construct model matrix for each inheritance type
        switch (m_inheritanceType) {
        case Transform::kPreserveOrientation:
        {
            // Obtain world translation by removing the rotation component of the world matrix
            Matrix4x4f localTranslation;
            localTranslation.setTranslation(m_translation.getPosition().asReal());
            Matrix4x4f translation = m_worldMatrix * localTranslation;
            translation = translation.getTranslationMatrix();

            // Compute world matrix that preserves original orientation of state w.r.t. inertial frame
            m_worldMatrix = translation * m_rotation.getMatrix() * m_scale.getMatrix();
        }
        case Transform::kTranslation:
        case Transform::kAll:
        default:
            // Compute world matrix
            m_worldMatrix *= m_localMatrix;
            break;
        }
    }

    // Update all child states
    for (const std::pair<Uuid, Transform*>& childPair: m_children) {
        //auto childObject = std::static_pointer_cast<SceneObject>(childPair.second);
        //auto childTransform = childObject->transform();
        childPair.second->computeWorldMatrix();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing