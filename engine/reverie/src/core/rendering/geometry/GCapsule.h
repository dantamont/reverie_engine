///////////////////////////////////////////////////////////////////////////////
// Capsule.h
// ==========
// Capsule for OpenGL with (base radius, top radius, height, sectors, stacks)
// The min number of sectors (slices) is 3 and the min number of stacks are 1.
// - base radius: the radius of the Capsule at z = -height/2
// - top radius : the radiusof the Capsule at z = height/2
// - height     : the height of the Capsule along z-axis
// - sectors    : the number of slices of the base and top caps
// - stacks     : the number of subdivisions along z-axis
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2018-03-27
// UPDATED: 2019-12-02
// See: http://www.songho.ca/opengl/gl_Capsule.html
///////////////////////////////////////////////////////////////////////////////

#ifndef GB_GEOMETRY_CAPSULE_H
#define GB_GEOMETRY_CAPSULE_H

#include <vector>
#include <memory>
#include "../../geometry/GVector.h"

namespace rev {

///////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////
class VertexArrayData;
///////////////////////////////////////////////////////////////////////////////
class Capsule
{
public:
    // ctor/dtor
    Capsule(VertexArrayData& outVertexData, float radius = 1.0f, float halfHeight = 1.0f, int sectorCount = 36, int stackCount = 1);
    ~Capsule() {}

    // getters/setters
    float getRadius() const { return m_radius; }
    float getHalfHeight() const { return m_halfHeight; }
    int getSectorCount() const { return m_sectorCount; }
    int getStackCount() const { return m_stackCount; }
    void set(float radius, float halfHeight, int sectorCount, int stackCount);
    void setRadius(float radius);
    void setHalfHeight(float height);
    void setSectorCount(int sectorCount);
    void setStackCount(int stackCount);

    VertexArrayData& vertexData() { return m_vertexData; }

protected:

private:
    // member functions
    void clearArrays();
    void buildVerticesSmooth();
    void buildUnitCircleVertices();
    void buildCaps();
    void addVertex(float x, float y, float z);
    void addNormal(float x, float y, float z);
    void addTexCoord(float s, float t);
    void addIndices(unsigned int i1, unsigned int i2, unsigned int i3);
    std::vector<Vector3> getSideNormals();
    Vector3 computeFaceNormal(float x1, float y1, float z1,
        float x2, float y2, float z2,
        float x3, float y3, float z3);

    // member vars
    float m_radius;
    float m_halfHeight;
    int m_sectorCount;                        // # of slices
    int m_stackCount;                         // # of stacks
    unsigned int m_baseIndex;                 // starting index of base
    unsigned int m_topIndex;                  // starting index of top
    std::vector<Vector3> m_unitCircleVertices;
    VertexArrayData& m_vertexData;
    std::vector<unsigned int> m_lineIndices;

};

///////////////////////////////////////////////////////////////////////////////
// End namespaces 
}

#endif
