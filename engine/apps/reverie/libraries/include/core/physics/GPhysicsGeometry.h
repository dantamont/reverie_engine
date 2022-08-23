#pragma once

// Internal
#include "GPhysics.h"
#include "fortress/containers/math/GVector.h"
#include "enums/GPhysicsGeometryTypeEnum.h"

namespace rev {

class Scene;

/// @class PhysicsGeometry
/// @brief Class representing physics geometry
/// @see https://documentation.help/NVIDIA-PhysX-SDK-Guide/Shapes.html
class PhysicsGeometry {
public:
    /// @name Statics
    /// @{

    /// @brief Create geometry from JSON
    static std::unique_ptr<PhysicsGeometry> CreateGeometry(const json& json);

    /// @}

    /// @name Constructors/Destructor
    /// @{
    PhysicsGeometry(GPhysicsGeometryType type);
    virtual ~PhysicsGeometry();
    /// @}

    /// @name Properties
    /// @{

    GPhysicsGeometryType getType() const { return m_type; }

    virtual const physx::PxGeometry& getGeometry() const = 0;

    /// @}

    /// @name Public methods
    /// @{

    /// @brief Return the geometry as JSON, properly casting to child classes before serialization
    json asJson() const;

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const PhysicsGeometry& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, PhysicsGeometry& orObject);


    /// @}

private:
    /// @name Private Members
    /// @{
    
    GPhysicsGeometryType m_type;

    /// @}
};


/// @class BoxGeometry
/// @brief Class representing box geometry
class BoxGeometry : public PhysicsGeometry {
public:
    /// @name Constructors/Destructor
    /// @{
    BoxGeometry(const Vector3f& extents);
    BoxGeometry(float hx = 1.0, float hy = 1.0, float hz = 1.0);
    ~BoxGeometry();
    /// @}

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

    Vector3d halfExtents() const;

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Whether the geometry is valid or not
    inline bool isValid() const { return m_box.isValid(); }

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const BoxGeometry& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, BoxGeometry& orObject);


    /// @}

protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    physx::PxBoxGeometry m_box;

    /// @}

};


/// @class SphereGeometry
/// @brief Class representing box geometry
class SphereGeometry : public PhysicsGeometry {
public:
    /// @name Constructors/Destructor
    /// @{
    SphereGeometry();
    SphereGeometry(float radius);
    ~SphereGeometry();
    /// @}

    /// @name Properties
    /// @{

    virtual const physx::PxGeometry& getGeometry() const override {
        return m_sphere;
    }

    float radius() const { return m_sphere.radius; }
    void setRadius(float r) { m_sphere.radius = r; };

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Whether the geometry is valid or not
    inline bool isValid() const { return m_sphere.isValid(); }

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const SphereGeometry& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, SphereGeometry& orObject);


    /// @}

protected:
    /// @name Protected Members
    /// @{

    physx::PxSphereGeometry m_sphere;

    /// @}

};

/// @class PlaneGeometry
/// @brief Class representing plane geometry
class PlaneGeometry : public PhysicsGeometry {
public:
    /// @name Constructors/Destructor
    /// @{
    PlaneGeometry();
    ~PlaneGeometry();
    /// @}

    /// @name Properties
    /// @{

    virtual const physx::PxGeometry& getGeometry() const override {
        return m_plane;
    }

    /// @brief Normal and origin distance are determined entirely by shape's pose
    //const Vector3f& getNormal() const { return m_normal; }
    //float getDistanceFromOrigin() const { return m_distanceFromOrigin; }

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Whether the geometry is valid or not
    inline bool isValid() const { return m_plane.isValid(); }

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const PlaneGeometry& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, PlaneGeometry& orObject);


    /// @}

protected:
    /// @name Protected Members
    /// @{

    physx::PxPlaneGeometry m_plane;

    /// @}

};



} // End namespaces
