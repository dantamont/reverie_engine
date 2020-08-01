#ifndef GB_PHYSICS_GEOMETRY_H
#define GB_PHYSICS_GEOMETRY_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// External

// QT
#include <QJsonObject>

// Internal
#include "GbPhysics.h"
#include "../geometry/GbVector.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Scene;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class PhysicsGeometry
/// @brief Class representing physics geometry
/// See: https://documentation.help/NVIDIA-PhysX-SDK-Guide/Shapes.html
class PhysicsGeometry : public Object, public Serializable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Statics
    /// @{

    /// @brief Create geometry from JSON
    static std::shared_ptr<PhysicsGeometry> createGeometry(const QJsonObject& json);

    enum GeometryType {
        kBase = -1,
        kBox,
        kSphere,
        kPlane
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    PhysicsGeometry(GeometryType type);
    virtual ~PhysicsGeometry();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    GeometryType getType() const { return m_type; }

    virtual const physx::PxGeometry& getGeometry() const = 0;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PhysicsGeometry"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PhysicsGeometry"; }

    /// @}

private:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Private Members
    /// @{
    
    GeometryType m_type;

    /// @}
};


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class BoxGeometry
/// @brief Class representing box geometry
class BoxGeometry : public PhysicsGeometry {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    BoxGeometry(const Vector3f& extents);
    BoxGeometry(float hx = 1.0, float hy = 1.0, float hz = 1.0);
    ~BoxGeometry();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    virtual const physx::PxGeometry& getGeometry() const override{
        return m_box;
    }

    float hx() const { return m_box.halfExtents.x; };
    float hy() const { return m_box.halfExtents.y; };
    float hz() const { return m_box.halfExtents.z; };

    void setHx(float x) { m_box.halfExtents.x = x; };
    void setHy(float y) { m_box.halfExtents.y = y; };
    void setHz(float z) { m_box.halfExtents.z = z; };

    Vector3 halfExtents() const;

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Whether the geometry is valid or not
    inline bool isValid() const { return m_box.isValid(); }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "BoxGeometry"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::BoxGeometry"; }

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    physx::PxBoxGeometry m_box;

    /// @}

};


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class SphereGeometry
/// @brief Class representing box geometry
class SphereGeometry : public PhysicsGeometry {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    SphereGeometry();
    SphereGeometry(float radius);
    ~SphereGeometry();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    virtual const physx::PxGeometry& getGeometry() const override {
        return m_sphere;
    }

    float radius() const { return m_sphere.radius; }
    void setRadius(float r) { m_sphere.radius = r; };

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Whether the geometry is valid or not
    inline bool isValid() const { return m_sphere.isValid(); }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "SphereGeometry"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::SphereGeometry"; }

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    physx::PxSphereGeometry m_sphere;

    /// @}

};



//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class PlaneGeometry
/// @brief Class representing plane geometry
class PlaneGeometry : public PhysicsGeometry {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    PlaneGeometry();
    ~PlaneGeometry();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    virtual const physx::PxGeometry& getGeometry() const override {
        return m_plane;
    }

    /// @brief Normal and origin distance are determined entirely by shape's pose
    //const Vector3f& getNormal() const { return m_normal; }
    //float getDistanceFromOrigin() const { return m_distanceFromOrigin; }

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Whether the geometry is valid or not
    inline bool isValid() const { return m_plane.isValid(); }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PlaneGeometry"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PlaneGeometry"; }

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    physx::PxPlaneGeometry m_plane;

    /// @}

};


//////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif