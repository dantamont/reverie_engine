// See:
// https://www.trentreed.net/blog/qt5-opengl-part-1-basic-rendering/

#ifndef GB_VERTEX_DATA_H
#define GB_VERTEX_DATA_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// QT
#include "../GGLFunctions.h"

// Internal
#include "../../GObject.h"
#include "../../geometry/GVector.h"
/////////////////////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef GB_USE_DOUBLE
typedef double real_g;
#else
typedef float real_g;
#endif

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Enum of valid buffer data types
/// @details Enum value represents position in shader
enum class BufferAttributeType {
    kNone = -1,
    kPosition,
    kColor,
    kTextureCoordinates,
    kNormal,
    kTangent,
    kMiscInt,
    kMiscReal,
    kMAX_ATTRIBUTE_TYPE
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct VertexAttributeInfo
struct VertexAttributeInfo {
    enum NumericalType : uint32_t {
        kInt,
        kFloat,
        kDouble
    };

    bool operator==(const VertexAttributeInfo& other) const {
        return (other.m_attributeLength == m_attributeLength) &&
            (other.m_numericalType == m_numericalType) &&
            (other.m_count == m_count);
    }

    bool operator!=(const VertexAttributeInfo& other) const {
        return !(*this == other);
    }

    uint32_t m_attributeLength; // e.g., vec3 vs vec4
    NumericalType m_numericalType; // e.g., vec3i vs vec3
    uint32_t m_count; // Number of attributes used, e.g., 3 for vec3 if vertices, normals, tangents used
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct VertexAttributeLayout
/// @brief Layout for describing vertex attributesfor serialization, in case format changes
struct VertexAttributesLayout {
    std::vector<VertexAttributeInfo> m_layout = { 
        {2, VertexAttributeInfo::kFloat, 1}, 
        {3, VertexAttributeInfo::kFloat, 3}, 
        {4, VertexAttributeInfo::kFloat, 2},
        {4, VertexAttributeInfo::kInt, 1}
    };
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct VertexAttributes
/// @brief Struct holding vertex attributes
struct VertexAttributes {
    std::vector<Vector2> m_texCoords;   // 'vt'(uv)

    /// @brief Vertex attribute data
    std::vector<Vector3> m_vertices;    // 'v'(xyz)
    std::vector<Vector3> m_normals;     // 'vn'
    std::vector<Vector3> m_tangents;    // tangents for normal mapping

    std::vector<Vector4> m_colors;      // extension: vertex colors
    std::vector<Vector4> m_miscReal; // (Skeletal animation) used for bone weights for skeletal animation
    
    std::vector<Vector4i> m_miscInt; // (Skeletal animation) used for IDs of the bones corresponding to the bone weights

    quint64 getSizeInBytes() const;

    bool empty() const;

    void clear();
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}

#endif
