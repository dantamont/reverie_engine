#pragma once
/// @file Capsule.h
// ==========
// Capsule for OpenGL with (base radius, top radius, height, sectors, stacks)
// The min number of sectors (slices) is 3 and the min number of stacks are 1.
// - base radius: the radius of the Capsule at z = -height/2
// - top radius : the radiusof the Capsule at z = height/2
// - height     : the height of the Capsule along z-axis
// - sectors    : the number of slices of the base and top caps
// - stacks     : the number of subdivisions along z-axis
//
/// @author Song Ho Ahn (song.ahn@gmail.com)
/// @created 2018-03-27
/// @updated 2019-12-02
/// @see http://www.songho.ca/opengl/gl_Capsule.html

#include <vector>
#include <memory>
#include "fortress/containers/math/GVector.h"

namespace rev {

template<typename EnumType, template<typename> typename ContainerType, typename ...Types>
struct VertexAttributes;
enum class MeshVertexAttributeType;
typedef VertexAttributes<MeshVertexAttributeType, std::vector, Vector3, Vector4, Vector2, Vector3, Vector3, Vector4i, Vector4> MeshVertexAttributes;


class Capsule
{
public:
    Capsule(MeshVertexAttributes& outData, float radius = 1.0f, float halfHeight = 1.0f, int sectorCount = 36, int stackCount = 1);
    ~Capsule() {}

    float getRadius() const { return m_radius; }
    float getHalfHeight() const { return m_halfHeight; }
    int getSectorCount() const { return m_sectorCount; }
    int getStackCount() const { return m_stackCount; }

    void set(MeshVertexAttributes& outData, float radius, float halfHeight, int sectorCount, int stackCount);
    void setRadius(MeshVertexAttributes& outData, float radius);
    void setHalfHeight(MeshVertexAttributes& outData, float height);
    void setSectorCount(MeshVertexAttributes& outData, int sectorCount);
    void setStackCount(MeshVertexAttributes& outData, int stackCount);

private:

    /// @brief build vertices of Capsule with smooth shading
    /// @details where v: sector angle (0 <= v <= 360)
    void buildVerticesSmooth(MeshVertexAttributes& outData);

    /// @brief Generate 3D vertices of a unit circle on XY plance
    void buildUnitCircleVertices();

    /// @brief Build out the rest of the capsule from the unit circle
    void buildCaps(MeshVertexAttributes& outData);

    /// @brief Add single vertex to array
    void addVertex(MeshVertexAttributes& outData, float x, float y, float z);

    /// @brief add single normal to array
    void addNormal(MeshVertexAttributes& outData, float x, float y, float z);

    /// @brief Add single texture coord to array
    void addTexCoord(MeshVertexAttributes& outData, float s, float t);

    /// @brief Add 3 indices to array
    void addIndices(MeshVertexAttributes& outData, unsigned int i1, unsigned int i2, unsigned int i3);

    /// @brief Generate shared normal vectors of the side of Capsule
    std::vector<Vector3> getSideNormals(const MeshVertexAttributes& data);

    /// @brief return face normal of a triangle v1-v2-v3
    /// @note if a triangle has no surface (normal length = 0), then return a zero vector
    static Vector3 ComputeFaceNormal(
        float x1, float y1, float z1,
        float x2, float y2, float z2,
        float x3, float y3, float z3);

    float m_radius;
    float m_halfHeight;
    int m_sectorCount;                        // # of slices
    int m_stackCount;                         // # of stacks
    unsigned int m_baseIndex;                 // starting index of base
    unsigned int m_topIndex;                  // starting index of top
    std::vector<Vector3> m_unitCircleVertices;

    static constexpr Int32_t s_minSectorCount = 3;
    static constexpr Int32_t s_minStackCount = 1;
};

// End namespaces 
}
