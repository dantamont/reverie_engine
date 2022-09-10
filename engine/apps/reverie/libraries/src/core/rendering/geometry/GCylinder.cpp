
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

#include "core/rendering/geometry/GCylinder.h"

#include <iostream>
#include <iomanip>
#include <cmath>

#include "core/rendering/geometry/GMesh.h"

namespace rev {

// constants 
const int s_minSectorCount = 3;
const int s_minStackCount = 1;

Cylinder::Cylinder(MeshVertexAttributes& outVertexData, 
    float baseRadius, float topRadius, 
    float height, int sectors, int stacks)
{
    set(outVertexData, baseRadius, topRadius, height, sectors, stacks);
}

void Cylinder::set(MeshVertexAttributes& outData, float baseRadius, float topRadius, float height, int sectors,
    int stacks)
{
    this->m_baseRadius = baseRadius;
    this->m_topRadius = topRadius;
    this->m_height = height;
    this->m_sectorCount = sectors;
    if (sectors < s_minSectorCount)
        this->m_sectorCount = s_minSectorCount;
    this->m_stackCount = stacks;
    if (stacks < s_minStackCount)
        this->m_stackCount = s_minStackCount;

    // generate unit circle vertices first
    buildUnitCircleVertices();

    buildVerticesSmooth(outData);

}

void Cylinder::setBaseRadius(MeshVertexAttributes& outData, float radius)
{
    if (this->m_baseRadius != radius)
        set(outData, radius, m_topRadius, m_height, m_sectorCount, m_stackCount);
}

void Cylinder::setTopRadius(MeshVertexAttributes& outData, float radius)
{
    if (this->m_topRadius != radius)
        set(outData, m_baseRadius, radius, m_height, m_sectorCount, m_stackCount);
}

void Cylinder::setHeight(MeshVertexAttributes& outData, float height)
{
    if (this->m_height != height)
        set(outData, m_baseRadius, m_topRadius, height, m_sectorCount, m_stackCount);
}

void Cylinder::setSectorCount(MeshVertexAttributes& outData, int sectors)
{
    if (this->m_sectorCount != sectors)
        set(outData, m_baseRadius, m_topRadius, m_height, sectors, m_stackCount);
}

void Cylinder::setStackCount(MeshVertexAttributes& outData, int stacks)
{
    if (this->m_stackCount != stacks)
        set(outData, m_baseRadius, m_topRadius, m_height, m_sectorCount, stacks);
}

void Cylinder::buildVerticesSmooth(MeshVertexAttributes& outData)
{
    outData.clear();

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
            addVertex(outData, x * radius, y * radius, z);   // position
            addNormal(outData, sideNormals[k].x(), sideNormals[k].y(), sideNormals[k].z()); // normal
            addTexCoord(outData, (float)j / m_sectorCount, t); // tex coord
        }
    }

    // remember where the base.top vertices start
    unsigned int baseVertexIndex = (unsigned int)outData.get<MeshVertexAttributeType::kPosition>().size();

    // put vertices of base of cylinder
    z = -m_height * 0.5f;
    addVertex(outData, 0, 0, z);
    addNormal(outData, 0, 0, -1);
    addTexCoord(outData, 0.5f, 0.5f);
    for (int i = 0, j = 0; i < m_sectorCount; ++i, j++)
    {
        x = m_unitCircleVertices[j].x();
        y = m_unitCircleVertices[j].y();
        addVertex(outData, x * m_baseRadius, y * m_baseRadius, z);
        addNormal(outData, 0, 0, -1);
        addTexCoord(outData, -x * 0.5f + 0.5f, -y * 0.5f + 0.5f);    // flip horizontal
    }

    // remember where the base vertices start
    unsigned int topVertexIndex = (unsigned int)outData.get<MeshVertexAttributeType::kPosition>().size();

    // put vertices of top of cylinder
    z = m_height * 0.5f;
    addVertex(outData, 0, 0, z);
    addNormal(outData, 0, 0, 1);
    addTexCoord(outData, 0.5f, 0.5f);
    for (int i = 0, j = 0; i < m_sectorCount; ++i, j++)
    {
        x = m_unitCircleVertices[j].x();
        y = m_unitCircleVertices[j].y();
        addVertex(outData, x * m_topRadius, y * m_topRadius, z);
        addNormal(outData, 0, 0, 1);
        addTexCoord(outData, x * 0.5f + 0.5f, -y * 0.5f + 0.5f);
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
            addIndices(outData, k1, k1 + 1, k2);
            addIndices(outData, k2, k1 + 1, k2 + 1);
        }
    }

    // remember where the base indices start
    m_baseIndex = (unsigned int)outData.get<MeshVertexAttributeType::kIndices>().size();

    // put indices for base
    for (int i = 0, k = baseVertexIndex + 1; i < m_sectorCount; ++i, ++k)
    {
        if (i < (m_sectorCount - 1))
            addIndices(outData, baseVertexIndex, k + 1, k);
        else    // last triangle
            addIndices(outData, baseVertexIndex, baseVertexIndex + 1, k);
    }

    // remember where the base indices start
    m_topIndex = (unsigned int)outData.get<MeshVertexAttributeType::kIndices>().size();

    for (int i = 0, k = topVertexIndex + 1; i < m_sectorCount; ++i, ++k)
    {
        if (i < (m_sectorCount - 1))
            addIndices(outData, topVertexIndex, k, k + 1);
        else
            addIndices(outData, topVertexIndex, k, topVertexIndex + 1);
    }

}

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

void Cylinder::addVertex(MeshVertexAttributes& outData, float x, float y, float z)
{
    Vec::EmplaceBack(outData.get<MeshVertexAttributeType::kPosition>(), x, y, z);
}

void Cylinder::addNormal(MeshVertexAttributes& outData, float nx, float ny, float nz)
{
    Vec::EmplaceBack(outData.get<MeshVertexAttributeType::kNormal>(), nx, ny, nz );
}

void Cylinder::addTexCoord(MeshVertexAttributes& outData, float s, float t)
{
    Vec::EmplaceBack(outData.get<MeshVertexAttributeType::kTextureCoordinates>(), s, t);
}

void Cylinder::addIndices(MeshVertexAttributes& outData, unsigned int i1, unsigned int i2, unsigned int i3)
{
    Vec::EmplaceBack(outData.get<MeshVertexAttributeType::kIndices>(), i1);
    Vec::EmplaceBack(outData.get<MeshVertexAttributeType::kIndices>(), i2);
    Vec::EmplaceBack(outData.get<MeshVertexAttributeType::kIndices>(), i3);
}

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


}