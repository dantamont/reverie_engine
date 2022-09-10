#pragma once
///////////////////////////////////////////////////////////////////////////////
// Cylinder.h
// ==========
// Cylinder for OpenGL with (base radius, top radius, height, sectors, stacks)
// The min number of sectors (slices) is 3 and the min number of stacks are 1.
// - base radius: the radius of the cylinder at z = -height/2
// - top radius : the radiusof the cylinder at z = height/2
// - height     : the height of the cylinder along z-axis
// - sectors    : the number of slices of the base and top caps
// - stacks     : the number of subdivisions along z-axis
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2018-03-27
// UPDATED: 2019-12-02
// See: http://www.songho.ca/opengl/gl_cylinder.html
///////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <memory>
#include "fortress/containers/math/GVector.h"

namespace rev {

template<typename EnumType, template<typename> typename ContainerType, typename ...Types>
struct VertexAttributes;
enum class MeshVertexAttributeType;
typedef VertexAttributes<MeshVertexAttributeType, std::vector, Vector3, Vector4, Vector2, Vector3, Vector3, Vector4i, Vector4> MeshVertexAttributes;

class Cylinder
{
public:
    Cylinder(MeshVertexAttributes& outVertexData, float baseRadius = 1.0f, float topRadius = 1.0f, float height = 1.0f,
        int sectorCount = 36, int stackCount = 1);
    ~Cylinder() {}

    float getBaseRadius() const { return m_baseRadius; }
    float getTopRadius() const { return m_topRadius; }
    float getHeight() const { return m_height; }
    int getSectorCount() const { return m_sectorCount; }
    int getStackCount() const { return m_stackCount; }

    void set(MeshVertexAttributes& outData, float baseRadius, float topRadius, float height, int sectorCount, int stackCount);
    void setBaseRadius(MeshVertexAttributes& outData, float radius);
    void setTopRadius(MeshVertexAttributes& outData, float radius);
    void setHeight(MeshVertexAttributes& outData, float radius);
    void setSectorCount(MeshVertexAttributes& outData, int sectorCount);
    void setStackCount(MeshVertexAttributes& outData, int stackCount);

private:
    /// @brief build vertices of cylinder with smooth shading
    /// where v: sector angle (0 <= v <= 360)
    void buildVerticesSmooth(MeshVertexAttributes& outData);

    /// @brief generate 3D vertices of a unit circle on XY plance
    void buildUnitCircleVertices();

    /// @brief add single vertex to array
    void addVertex(MeshVertexAttributes& outData, float x, float y, float z);

    /// @brief add single normal to array
    void addNormal(MeshVertexAttributes& outData, float x, float y, float z);

    /// @brief add single texture coord to array
    void addTexCoord(MeshVertexAttributes& outData, float s, float t);

    /// @brief add 3 indices to array
    void addIndices(MeshVertexAttributes& outData, unsigned int i1, unsigned int i2, unsigned int i3);

    /// @brief generate shared normal vectors of the side of cylinder
    std::vector<Vector3> getSideNormals();

    float m_baseRadius;
    float m_topRadius;
    float m_height;
    int m_sectorCount;                        // # of slices
    int m_stackCount;                         // # of stacks
    unsigned int m_baseIndex;                 // starting index of base
    unsigned int m_topIndex;                  // starting index of top
    std::vector<Vector3> m_unitCircleVertices;


    static constexpr Int32_t s_minSectorCount = 3;
    static constexpr Int32_t s_minStackCount = 1;
};

} // End namespaces 
