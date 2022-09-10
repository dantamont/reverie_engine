#pragma once

// Internal
#include "core/rendering/buffers/GVertexArrayObject.h"
#include "core/rendering/buffers/GGlBuffer.h"
#include "core/resource/GResourceHandle.h"
#include "fortress/containers/math/GMatrix.h"
#include "fortress/containers/GContainerExtensions.h"
#include "core/animation/GAnimation.h"
#include "heave/collisions/GCollisions.h"
#include "core/rendering/geometry/GVertexArrayData.h"

namespace rev {

class Model;
class Skeleton;
class CoreEngine;
class RenderContext;
enum class PrimitiveMode;

/// @class Mesh
/// @brief Encapsulates a VAO as a resource
class Mesh: public Resource {
public:
    /// @name Constructors and Destructors
    /// @{
    //Mesh(const GString& uniqueName);
    Mesh();
    ~Mesh();
    /// @}

    /// @name Properties 
    /// @{

    /// @brief Return map of mesh data
    const rev::VertexArrayData& vertexData() const { return m_vertexData; }

    /// @brief Return bounding box for the mesh
    const AABB& objectBounds() const { return m_objectBounds; }
    AABB& objectBounds() { return m_objectBounds; }

    /// @}

    /// @name Methods
    /// @{

    /// @brief Get the type of resource stored by this handle
    virtual GResourceType getResourceType() const override {
        return EResourceType::eMesh;
    }

    /// @brief What to do on removal from resource cache
    void onRemoval(ResourceCache* cache = nullptr) override;

    /// @brief What action to perform post-construction of the resource
    /// @details For performing any actions that need to be done on the main thread
    virtual void postConstruction(const ResourcePostConstructionData& postConstructData) override;

    /// @brief Generate object bounds for the mesh
    void generateBounds(const MeshVertexAttributes& data);

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Mesh& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Mesh& orObject);


    /// @}

private:
    /// @name Friends

    friend class CubeMap;
    friend class ObjReader;
    friend class ModelReader;
    friend class Animation;
    friend class NodeAnimation;

    /// @}

    /// @name Private members 
    /// @{

    rev::VertexArrayData m_vertexData; ///< Vertex data for the mesh
    AABB m_objectBounds; ///< The bounds of the mesh

    /// @}
};


} // End namespaces
