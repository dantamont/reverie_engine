///////////////////////////////////////////////////////////////////////////////
// Cylinder.cpp
// ============
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
// UPDATED: 2020-03-14
///////////////////////////////////////////////////////////////////////////////
#include "GCylinder.h"

#include <iostream>
#include <iomanip>
#include <cmath>

#include "GMesh.h"

namespace rev {

// constants //////////////////////////////////////////////////////////////////
const int MIN_SECTOR_COUNT = 3;
const int MIN_STACK_COUNT = 1;



///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
Cylinder::Cylinder(VertexArrayData& outVertexData, float baseRadius, float topRadius, float height, int sectors,
    int stacks):
    m_vertexData(outVertexData)
{
    set(baseRadius, topRadius, height, sectors, stacks);
}



///////////////////////////////////////////////////////////////////////////////
// setters
///////////////////////////////////////////////////////////////////////////////
void Cylinder::set(float baseRadius, float topRadius, float height, int sectors,
    int stacks)
{
    this->m_baseRadius = baseRadius;
    this->m_topRadius = topRadius;
    this->m_height = height;
    this->m_sectorCount = sectors;
    if (sectors < MIN_SECTOR_COUNT)
        this->m_sectorCount = MIN_SECTOR_COUNT;
    this->m_stackCount = stacks;
    if (stacks < MIN_STACK_COUNT)
        this->m_stackCount = MIN_STACK_COUNT;

    // generate unit circle vertices first
    buildUnitCircleVertices();

    buildVerticesSmooth();

}

void Cylinder::setBaseRadius(float radius)
{
    if (this->m_baseRadius != radius)
        set(radius, m_topRadius, m_height, m_sectorCount, m_stackCount);
}

void Cylinder::setTopRadius(float radius)
{
    if (this->m_topRadius != radius)
        set(m_baseRadius, radius, m_height, m_sectorCount, m_stackCount);
}

void Cylinder::setHeight(float height)
{
    if (this->m_height != height)
        set(m_baseRadius, m_topRadius, height, m_sectorCount, m_stackCount);
}

void Cylinder::setSectorCount(int sectors)
{
    if (this->m_sectorCount != sectors)
        set(m_baseRadius, m_topRadius, m_height, sectors, m_stackCount);
}

void Cylinder::setStackCount(int stacks)
{
    if (this->m_stackCount != stacks)
        set(m_baseRadius, m_topRadius, m_height, m_sectorCount, stacks);
}


///////////////////////////////////////////////////////////////////////////////
// dealloc vectors
///////////////////////////////////////////////////////////////////////////////
void Cylinder::clearArrays()
{
    m_vertexData.m_attributes.clear();
    m_vertexData.m_indices.clear();
    std::vector<unsigned int>().swap(m_lineIndices);
}



///////////////////////////////////////////////////////////////////////////////
// build vertices of cylinder with smooth shading
// where v: sector angle (0 <= v <= 360)
///////////////////////////////////////////////////////////////////////////////
void Cylinder::buildVerticesSmooth()
{
    // clear memory of prev arrays
    clearArrays();

    float x, y, z;                                  // vertex position
    //float s, t;                                     // texCoord
    float radius;                                   // radius for each stack

    // get normals for cylinder sides
    std::vector<Vector3> sideNormals = getSideNormals();

    // put vertices of side cylinder to array by scaling unit circle
    for (int i = 0; i <= m_stackCount; ++i)
    {
        z = -(m_height * 0.5f) + (float)i / m_stackCount * m_height;      // vertex position z
        radius = m_baseRadius + (float)i / m_stackCount * (m_topRadius - m_baseRadius);     // lerp
        float t = 1.0f - (float)i / m_stackCount;   // top-to-bottom

        for (int j = 0, k = 0; j <= m_sectorCount; ++j, k++)
        {
            x = m_unitCircleVertices[k].x();
            y = m_unitCircleVertices[k].y();
            addVertex(x * radius, y * radius, z);   // position
            addNormal(sideNormals[k].x(), sideNormals[k].y(), sideNormals[k].z()); // normal
            addTexCoord((float)j / m_sectorCount, t); // tex coord
        }
    }

    // remember where the base.top vertices start
    unsigned int baseVertexIndex = (unsigned int)m_vertexData.m_attributes.m_vertices.size();

    // put vertices of base of cylinder
    z = -m_height * 0.5f;
    addVertex(0, 0, z);
    addNormal(0, 0, -1);
    addTexCoord(0.5f, 0.5f);
    for (int i = 0, j = 0; i < m_sectorCount; ++i, j++)
    {
        x = m_unitCircleVertices[j].x();
        y = m_unitCircleVertices[j].y();
        addVertex(x * m_baseRadius, y * m_baseRadius, z);
        addNormal(0, 0, -1);
        addTexCoord(-x * 0.5f + 0.5f, -y * 0.5f + 0.5f);    // flip horizontal
    }

    // remember where the base vertices start
    unsigned int topVertexIndex = (unsigned int)m_vertexData.m_attributes.m_vertices.size();

    // put vertices of top of cylinder
    z = m_height * 0.5f;
    addVertex(0, 0, z);
    addNormal(0, 0, 1);
    addTexCoord(0.5f, 0.5f);
    for (int i = 0, j = 0; i < m_sectorCount; ++i, j++)
    {
        x = m_unitCircleVertices[j].x();
        y = m_unitCircleVertices[j].y();
        addVertex(x * m_topRadius, y * m_topRadius, z);
        addNormal(0, 0, 1);
        addTexCoord(x * 0.5f + 0.5f, -y * 0.5f + 0.5f);
    }

    // put indices for sides
    unsigned int k1, k2;
    for (int i = 0; i < m_stackCount; ++i)
    {
        k1 = i * (m_sectorCount + 1);     // bebinning of current stack
        k2 = k1 + m_sectorCount + 1;      // beginning of next stack

        for (int j = 0; j < m_sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector
            addIndices(k1, k1 + 1, k2);
            addIndices(k2, k1 + 1, k2 + 1);

            // vertical lines for all stacks
            m_lineIndices.push_back(k1);
            m_lineIndices.push_back(k2);
            // horizontal lines
            m_lineIndices.push_back(k2);
            m_lineIndices.push_back(k2 + 1);
            if (i == 0)
            {
                m_lineIndices.push_back(k1);
                m_lineIndices.push_back(k1 + 1);
            }
        }
    }

    // remember where the base indices start
    m_baseIndex = (unsigned int)m_vertexData.m_indices.size();

    // put indices for base
    for (int i = 0, k = baseVertexIndex + 1; i < m_sectorCount; ++i, ++k)
    {
        if (i < (m_sectorCount - 1))
            addIndices(baseVertexIndex, k + 1, k);
        else    // last triangle
            addIndices(baseVertexIndex, baseVertexIndex + 1, k);
    }

    // remember where the base indices start
    m_topIndex = (unsigned int)m_vertexData.m_indices.size();

    for (int i = 0, k = topVertexIndex + 1; i < m_sectorCount; ++i, ++k)
    {
        if (i < (m_sectorCount - 1))
            addIndices(topVertexIndex, k, k + 1);
        else
            addIndices(topVertexIndex, k, topVertexIndex + 1);
    }

}

///////////////////////////////////////////////////////////////////////////////
// generate 3D vertices of a unit circle on XY plance
///////////////////////////////////////////////////////////////////////////////
void Cylinder::buildUnitCircleVertices()
{
    const float PI = acos(-1);
    float sectorStep = 2 * PI / m_sectorCount;
    float sectorAngle;  // radian

    std::vector<Vector3>().swap(m_unitCircleVertices);
    for (int i = 0; i <= m_sectorCount; ++i)
    {
        sectorAngle = i * sectorStep;
        m_unitCircleVertices.push_back({cos(sectorAngle), sin(sectorAngle), 0 });
    }
}



///////////////////////////////////////////////////////////////////////////////
// add single vertex to array
///////////////////////////////////////////////////////////////////////////////
void Cylinder::addVertex(float x, float y, float z)
{
    Vec::EmplaceBack(m_vertexData.m_attributes.m_vertices, x, y, z);
}



///////////////////////////////////////////////////////////////////////////////
// add single normal to array
///////////////////////////////////////////////////////////////////////////////
void Cylinder::addNormal(float nx, float ny, float nz)
{
    Vec::EmplaceBack(m_vertexData.m_attributes.m_normals, nx, ny, nz );
}



///////////////////////////////////////////////////////////////////////////////
// add single texture coord to array
///////////////////////////////////////////////////////////////////////////////
void Cylinder::addTexCoord(float s, float t)
{
    Vec::EmplaceBack(m_vertexData.m_attributes.m_texCoords, s, t);
}



///////////////////////////////////////////////////////////////////////////////
// add 3 indices to array
///////////////////////////////////////////////////////////////////////////////
void Cylinder::addIndices(unsigned int i1, unsigned int i2, unsigned int i3)
{
    Vec::EmplaceBack(m_vertexData.m_indices, i1);
    Vec::EmplaceBack(m_vertexData.m_indices, i2);
    Vec::EmplaceBack(m_vertexData.m_indices, i3);
}



///////////////////////////////////////////////////////////////////////////////
// generate shared normal vectors of the side of cylinder
///////////////////////////////////////////////////////////////////////////////
std::vector<Vector3> Cylinder::getSideNormals()
{
    const float PI = acos(-1);
    float sectorStep = 2 * PI / m_sectorCount;
    float sectorAngle;  // radian

    // compute the normal vector at 0 degree first
    // tanA = (baseRadius-topRadius) / height
    float zAngle = atan2(m_baseRadius - m_topRadius, m_height);
    float x0 = cos(zAngle);     // nx
    float y0 = 0;               // ny
    float z0 = sin(zAngle);     // nz

    // rotate (x0,y0,z0) per sector angle
    std::vector<Vector3> normals;
    for (int i = 0; i <= m_sectorCount; ++i)
    {
        sectorAngle = i * sectorStep;
        normals.push_back({ cos(sectorAngle)*x0 - sin(sectorAngle)*y0,   // nx
            sin(sectorAngle)*x0 + cos(sectorAngle)*y0,   // ny
            z0 });  // nz
    }

    return normals;
}



///////////////////////////////////////////////////////////////////////////////
// return face normal of a triangle v1-v2-v3
// if a triangle has no surface (normal length = 0), then return a zero vector
///////////////////////////////////////////////////////////////////////////////
Vector3 Cylinder::computeFaceNormal(float x1, float y1, float z1,  // v1
    float x2, float y2, float z2,  // v2
    float x3, float y3, float z3)  // v3
{
    const float EPSILON = 0.000001f;

    std::vector<float> normal(3, 0.0f);     // default return value (0,0,0)
    float nx, ny, nz;

    // find 2 edge vectors: v1-v2, v1-v3
    float ex1 = x2 - x1;
    float ey1 = y2 - y1;
    float ez1 = z2 - z1;
    float ex2 = x3 - x1;
    float ey2 = y3 - y1;
    float ez2 = z3 - z1;

    // cross product: e1 x e2
    nx = ey1 * ez2 - ez1 * ey2;
    ny = ez1 * ex2 - ex1 * ez2;
    nz = ex1 * ey2 - ey1 * ex2;

    // normalize only if the length is > 0
    float length = sqrtf(nx * nx + ny * ny + nz * nz);
    if (length > EPSILON)
    {
        // normalize
        float lengthInv = 1.0f / length;
        normal[0] = nx * lengthInv;
        normal[1] = ny * lengthInv;
        normal[2] = nz * lengthInv;
    }

    return normal;
}

///////////////////////////////////////////////////////////////////////////////
}