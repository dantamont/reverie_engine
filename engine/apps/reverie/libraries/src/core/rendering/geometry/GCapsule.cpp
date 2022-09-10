
// Capsule.cpp
// ============
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
// UPDATED: 2020-03-14

#include "core/rendering/geometry/GCapsule.h"

#include <iostream>
#include <iomanip>
#include <cmath>

#include <fortress/constants/GConstants.h>
#include "core/rendering/geometry/GMesh.h"

namespace rev {

Capsule::Capsule(MeshVertexAttributes& outData, float radius, float halfHeight, int sectors, int stacks)
{
    set(outData, radius, halfHeight, sectors, stacks);
}

void Capsule::set(MeshVertexAttributes& outData, float radius, float halfHeight, int sectors, int stacks)
{
    this->m_radius = radius;
    this->m_halfHeight = halfHeight;
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

void Capsule::setRadius(MeshVertexAttributes& outData, float radius)
{
    if (this->m_radius != radius) {
        set(outData, radius, m_halfHeight, m_sectorCount, m_stackCount);
    }
}


void Capsule::setHalfHeight(MeshVertexAttributes& outData, float halfHeight)
{
    if (this->m_halfHeight != halfHeight) {
        set(outData, m_radius, halfHeight, m_sectorCount, m_stackCount);
    }
}

void Capsule::setSectorCount(MeshVertexAttributes& outData, int sectors)
{
    if (this->m_sectorCount != sectors) {
        set(outData, m_radius, m_halfHeight, sectors, m_stackCount);
    }
}

void Capsule::setStackCount(MeshVertexAttributes& outData, int stacks)
{
    if (this->m_stackCount != stacks) {
        set(outData, m_radius, m_halfHeight, m_sectorCount, stacks);
    }
}

void Capsule::buildVerticesSmooth(MeshVertexAttributes& outData)
{
    outData.clear();
    buildCaps(outData);
}

void Capsule::buildUnitCircleVertices()
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

void Capsule::buildCaps(MeshVertexAttributes& outData)
{
    int startIndex = outData.get<MeshVertexAttributeType::kPosition>().size() - 1;

    float x, y, z, xy;                                // vertex position
    float nx, ny, nz, lengthInv = 1.0f / m_radius;      // vertex normal
    //float s, t;                                     // vertex texCoord

    int stackCount = 20;
    if (stackCount % 2) {
        stackCount += 1;
    }
    int sectorCount = 36;
    float sectorStep = 2.0 * Constants::Pi / sectorCount;
    float stackStep = Constants::Pi / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount / 2; ++i)
    {
        stackAngle = Constants::Pi / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = m_radius * cosf(stackAngle);             // r * cos(u)
        z = m_radius * sinf(stackAngle) + m_halfHeight;              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            addVertex(outData, x, z, y); // flipped y and z

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            addNormal(outData, nx, nz, ny); // flipped y and z

            //// vertex tex coord (s, t) range between [0, 1]
            //s = (float)j / sectorCount;
            //t = (float)i / stackCount;
            //meshData.m_attributes.m_texCoords.push_back(Vector2f(s, t));
        }
    }

    for (int i = stackCount / 2 + 1; i <= stackCount; ++i)
    {
        stackAngle = Constants::Pi / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = m_radius * cosf(stackAngle);             // r * cos(u)
        z = m_radius * sinf(stackAngle) - m_halfHeight;              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            addVertex(outData, x, z, y); // flipped y and z

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            addNormal(outData, nx, nz, ny); // flipped y and z

            //// vertex tex coord (s, t) range between [0, 1]
            //s = (float)j / sectorCount;
            //t = (float)i / stackCount;
            //meshData.m_attributes.m_texCoords.push_back(Vector2f(s, t));
        }
    }

    // Generate indices
    // generate CCW index list of sphere triangles
    int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = startIndex + i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0)
            {
                addIndices(outData, k1, k2, k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1))
            {
                addIndices(outData, k1 + 1, k2, k2 + 1);
            }
        }
    }

}

void Capsule::addVertex(MeshVertexAttributes& outData, float x, float y, float z)
{
    Vec::EmplaceBack(outData.get<MeshVertexAttributeType::kPosition>(), x, y, z);
}

void Capsule::addNormal(MeshVertexAttributes& outData, float nx, float ny, float nz)
{
    Vec::EmplaceBack(outData.get<MeshVertexAttributeType::kNormal>(), nx, ny, nz);
}

void Capsule::addTexCoord(MeshVertexAttributes& outData, float s, float t)
{
    Vec::EmplaceBack(outData.get<MeshVertexAttributeType::kTextureCoordinates>(), s, t);
}

void Capsule::addIndices(MeshVertexAttributes& outData, unsigned int i1, unsigned int i2, unsigned int i3)
{
    Vec::EmplaceBack(outData.get<MeshVertexAttributeType::kIndices>(), i1);
    Vec::EmplaceBack(outData.get<MeshVertexAttributeType::kIndices>(), i2);
    Vec::EmplaceBack(outData.get<MeshVertexAttributeType::kIndices>(), i3);
}

std::vector<Vector3> Capsule::getSideNormals(const MeshVertexAttributes& data)
{
    const float PI = acos(-1);
    float sectorStep = 2 * PI / m_sectorCount;
    float sectorAngle;  // radian

    // compute the normal vector at 0 degree first
    float zAngle = atan2(0.0f, m_halfHeight * 2.0f);
    float x0 = cos(zAngle);     // nx
    float y0 = 0;               // ny
    float z0 = sin(zAngle);     // nz

    // rotate (x0,y0,z0) per sector angle
    std::vector<Vector3> normals;
    normals.reserve(m_sectorCount);
    for (int i = 0; i <= m_sectorCount; ++i)
    {
        sectorAngle = i * sectorStep;
        normals.push_back(
            { cos(sectorAngle)*x0 - sin(sectorAngle)*y0,   // nx
            sin(sectorAngle)*x0 + cos(sectorAngle)*y0,   // ny
            z0 }
        );  // nz
    }

    return normals;
}

Vector3 Capsule::ComputeFaceNormal(float x1, float y1, float z1,  // v1
    float x2, float y2, float z2,  // v2
    float x3, float y3, float z3)  // v3
{
    static constexpr float s_tolerance = 0.000001f;

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
    if (length > s_tolerance)
    {
        // normalize
        float lengthInv = 1.0f / length;
        normal[0] = nx * lengthInv;
        normal[1] = ny * lengthInv;
        normal[2] = nz * lengthInv;
    }

    return normal;
}


}