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

#ifndef GB_GEOMETRY_CYLINDER_H
#define GB_GEOMETRY_CYLINDER_H

#include <vector>
#include <memory>
#include "fortress/containers/math/GVector.h"

namespace rev {

///////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////
class VertexArrayData;
///////////////////////////////////////////////////////////////////////////////
class Cylinder
{
public:
    // ctor/dtor
    Cylinder(VertexArrayData& outVertexData, float baseRadius = 1.0f, float topRadius = 1.0f, float height = 1.0f,
        int sectorCount = 36, int stackCount = 1);
    ~Cylinder() {}

    // getters/setters
    float getBaseRadius() const { return m_baseRadius; }
    float getTopRadius() const { return m_topRadius; }
    float getHeight() const { return m_height; }
    int getSectorCount() const { return m_sectorCount; }
    int getStackCount() const { return m_stackCount; }
    void set(float baseRadius, float topRadius, float height,
        int sectorCount, int stackCount);
    void setBaseRadius(float radius);
    void setTopRadius(float radius);
    void setHeight(float radius);
    void setSectorCount(int sectorCount);
    void setStackCount(int stackCount);

    VertexArrayData& vertexData() { return m_vertexData; }

    // for indices of base/top/side parts
    //unsigned int getBaseIndexCount() const { return ((unsigned int)m_vertexData->m_indices.size() - m_baseIndex) / 2; }
    //unsigned int getTopIndexCount() const { return ((unsigned int)m_vertexData->m_indices.size() - m_baseIndex) / 2; }
    //unsigned int getSideIndexCount() const { return m_baseIndex; }
    //unsigned int getBaseStartIndex() const { return m_baseIndex; }
    //unsigned int getTopStartIndex() const { return m_topIndex; }
    //unsigned int getSideStartIndex() const { return 0; }   // side starts from the begining

protected:

private:
    // member functions
    void clearArrays();
    void buildVerticesSmooth();
    void buildUnitCircleVertices();
    void addVertex(float x, float y, float z);
    void addNormal(float x, float y, float z);
    void addTexCoord(float s, float t);
    void addIndices(unsigned int i1, unsigned int i2, unsigned int i3);
    std::vector<Vector3> getSideNormals();
    Vector3 computeFaceNormal(float x1, float y1, float z1,
        float x2, float y2, float z2,
        float x3, float y3, float z3);

    // member vars
    float m_baseRadius;
    float m_topRadius;
    float m_height;
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
