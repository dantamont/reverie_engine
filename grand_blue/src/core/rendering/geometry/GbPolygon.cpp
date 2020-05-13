#include "GbPolygon.h"

#include <QRegularExpression>
#include "../../GbCoreEngine.h"
#include "../../rendering/geometry/GbMesh.h"
#include "../../resource/GbResourceCache.h"
#include "../../resource/GbResource.h"
#include "../../containers/GbColor.h"
#include "GbCylinder.h"
#include "GbCapsule.h"

namespace Gb {

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
// Using statements

////////////////////////////////////////////////////////////////////////////////////////////////////////
std::unordered_map<PolygonCache::PolygonType, QString> PolygonCache::POLYGON_NAMES{
    {kRectangle, "rectangle"},
    {kCube, "cube"},
    {kLatLonSphere, "latlonsphere"},
    {kGridPlane, "gridplane"},
    {kGridCube, "gridcube"},
    {kCylinder, "cylinder"},
    {kCapsule, "capsule"}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<Vector3f> PolygonCache::CUBE_VERTEX_POSITIONS = {
    // Back face
    Vector3f(-0.5f, 0.5f, -0.5f),
    Vector3f(-0.5f, -0.5f, -0.5f),
    Vector3f(0.5f, -0.5f, -0.5f),
    Vector3f(0.5f, 0.5f, -0.5f),

    // Front face
    Vector3f(-0.5f, 0.5f, 0.5f),
    Vector3f(-0.5f, -0.5f, 0.5f),
    Vector3f(0.5f, -0.5f, 0.5f),
    Vector3f(0.5f, 0.5f, 0.5f),

    // Right face
    Vector3f(0.5f, 0.5f, -0.5f),
    Vector3f(0.5f, -0.5f, -0.5f),
    Vector3f(0.5f, -0.5f, 0.5f),
    Vector3f(0.5f, 0.5f,0.5f),

    // Left face
    Vector3f(-0.5f, 0.5f,-0.5f),
    Vector3f(-0.5f, -0.5f,-0.5f),
    Vector3f(-0.5f, -0.5f,0.5f),
    Vector3f(-0.5f, 0.5f,0.5f),

    // Top face
    Vector3f(-0.5f, 0.5f,0.5f),
    Vector3f(-0.5f, 0.5f,-0.5f),
    Vector3f(0.5f, 0.5f,-0.5f),
    Vector3f(0.5f, 0.5f,0.5f),

    // Bottom face
    Vector3f(-0.5f, -0.5f,0.5f),
    Vector3f(-0.5f, -0.5f,-0.5f),
    Vector3f(0.5f, -0.5f,-0.5f),
    Vector3f(0.5f, -0.5f,0.5f)
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<Vector3f> PolygonCache::CUBE_NORMALS = {
    // Back face
    Vector3f(0.0f, 0.0f, -1.0f),
    Vector3f(0.0f, 0.0f, -1.0f),
    Vector3f(0.0f, 0.0f, -1.0f),
    Vector3f(0.0f, 0.0f, -1.0f),

    // Front face
    Vector3f(0.0f, 0.0f, 1.0f),
    Vector3f(0.0f, 0.0f, 1.0f),
    Vector3f(0.0f, 0.0f, 1.0f),
    Vector3f(0.0f, 0.0f, 1.0f),

    // Right face
    Vector3f(1.0f, 0.0f, 0.0f),
    Vector3f(1.0f, 0.0f, 0.0f),
    Vector3f(1.0f, 0.0f, 0.0f),
    Vector3f(1.0f, 0.0f, 0.0f),

    // Left face
    Vector3f(-1.0f, 0.0f, 0.0f),
    Vector3f(-1.0f, 0.0f, 0.0f),
    Vector3f(-1.0f, 0.0f, 0.0f),
    Vector3f(-1.0f, 0.0f, 0.0f),

    // Top face
    Vector3f(0.0f, 1.0f, 0.0f),
    Vector3f(0.0f, 1.0f, 0.0f),
    Vector3f(0.0f, 1.0f, 0.0f),
    Vector3f(0.0f, 1.0f, 0.0f),

    // Bottom face
    Vector3f(0.0f, -1.0f, 0.0f),
    Vector3f(0.0f, -1.0f, 0.0f),
    Vector3f(0.0f, -1.0f, 0.0f),
    Vector3f(0.0f, -1.0f, 0.0f),
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define UV coordinates
std::vector<Vector2f> PolygonCache::CUBE_VERTEX_UVS_IDENTICAL = {
            Vector2f(0.0f, 1.0f), // These are flipped vertically from reference
            Vector2f(0.0f, 0.0f),
            Vector2f(1.0f, 0.0f),
            Vector2f(1.0f, 1.0f),
            Vector2f(0.0f, 1.0f),
            Vector2f(0.0f, 0.0f),
            Vector2f(1.0f, 0.0f),
            Vector2f(1.0f, 1.0f),
            Vector2f(0.0f, 1.0f),
            Vector2f(0.0f, 0.0f),
            Vector2f(1.0f, 0.0f),
            Vector2f(1.0f, 1.0f),
            Vector2f(0.0f, 1.0f),
            Vector2f(0.0f, 0.0f),
            Vector2f(1.0f, 0.0f),
            Vector2f(1.0f, 1.0f),
            Vector2f(0.0f, 1.0f),
            Vector2f(0.0f, 0.0f),
            Vector2f(1.0f, 0.0f),
            Vector2f(1.0f, 1.0f),
            Vector2f(0.0f, 1.0f),
            Vector2f(0.0f, 0.0f),
            Vector2f(1.0f, 0.0f),
            Vector2f(1.0f, 1.0f)
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define indices
std::vector<GLuint> PolygonCache::CUBE_VERTEX_INDICES = {
    0,1,3,
    3,1,2,
    4,5,7,
    7,5,6,
    8,9,11,
    11,9,10,
    12,13,15,
    15,13,14,
    16,17,19,
    19,17,18,
    20,21,23,
    23,21,22
};
////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PolygonCache::isPolygonName(const QString & name)
{
    // Search for an exact match
    bool isPolygon = std::find_if(POLYGON_NAMES.begin(), POLYGON_NAMES.end(),
        [&](const std::pair<PolygonType, QString>& namePair) {
        return namePair.second == name;
    }) != POLYGON_NAMES.end();

    if (isPolygon) return isPolygon;

    // Search for a partial match
    isPolygon = typeFromName(name) != kInvalid;

    return isPolygon;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
PolygonCache::PolygonType PolygonCache::typeFromName(const QString & name)
{
    QString matchNumber = QStringLiteral("([\\d]+\\.?[\\d]*)"); // matches a digit, then optionally a period and optional digits
    auto iter = std::find_if(POLYGON_NAMES.begin(), POLYGON_NAMES.end(),
        [&](const std::pair<PolygonType, QString>& namePair) {
        // Check match
        if (namePair.second == POLYGON_NAMES[kLatLonSphere]) {
            // Use regex, e.g. sphere_100_20, sphere_20_14
            QRegularExpression re("(latlonsphere)_" + matchNumber + "_" + matchNumber);
            QRegularExpressionMatch match = re.match(name);
            return match.hasMatch();
        }
        else if (namePair.second == POLYGON_NAMES[kGridPlane]) {
            // Use regex
            QRegularExpression re("(gridplane)_" + matchNumber + "_" + matchNumber );
            QRegularExpressionMatch match = re.match(name);
            bool matches = match.hasMatch();
            return matches;
        }
        else if (namePair.second == POLYGON_NAMES[kGridCube]) {
            // Use regex
            QRegularExpression re("(gridcube)_" + matchNumber + "_" + matchNumber);
            QRegularExpressionMatch match = re.match(name);
            bool matches = match.hasMatch();
            return matches;
        }
        else if (namePair.second == POLYGON_NAMES[kCapsule]) {
            // Use regex
            QRegularExpression re("(capsule)_" + matchNumber + "_" + matchNumber );
            QRegularExpressionMatch match = re.match(name);
            bool matches = match.hasMatch();
            return matches;
        }
        else if (namePair.second == POLYGON_NAMES[kCylinder]) {
            // Use regex
            QRegularExpression re("(cylinder)_" + matchNumber + "_" + matchNumber);
            QRegularExpressionMatch match = re.match(name);
            bool matches = match.hasMatch();
            return matches;
        }
        else {
            // For everything else, check for only exact match
            return namePair.second == name;
        }
    });
 
    if (iter == POLYGON_NAMES.end()) {
        // Not a valid polygon name
        return kInvalid;
    }
    else {
        return iter->first;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
PolygonCache::PolygonCache(CoreEngine* engine):
    m_engine(engine)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
PolygonCache::~PolygonCache()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> PolygonCache::getPolygon(const QString& polygonName)
{
    PolygonType type = typeFromName(polygonName);
    switch (type) {
    case kRectangle: {
        return getSquare();
    }
    case kCube: {
        return getCube();
    }
    case kLatLonSphere: {
        return getSphere(polygonName);
    }
    case kGridPlane : {
        return getGridPlane(polygonName);
    }
    case kGridCube: {
        return getGridCube(polygonName);
    }
    case kCylinder: {
        return getCylinder(polygonName);
    }
    case kCapsule: {
        return getCapsule(polygonName);
    }
    default:
        throw("Error, polygon type not implemented");
        break;
    }

    return nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> PolygonCache::getGridPlane(const QString & gridName)
{
    // Get grid of the correct grid size from a given grid name
    QStringList strings = gridName.split("_");
    float spacing = (float)strings[1].toDouble();
    int halfNumSpaces = strings[2].toInt();
    return getGridPlane(spacing, halfNumSpaces);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> PolygonCache::getGridPlane(float spacing, int halfNumSpaces)
{
    // Check if grid exists
    const QString& gridStr = getGridPlaneName(spacing, halfNumSpaces);
    if (m_engine->resourceCache()->handleWithName(gridStr, Resource::kMesh)) {
        return m_engine->resourceCache()->getMesh(gridStr);
    }

    // Create grid and resource handle
    std::shared_ptr<Mesh> grid = createGridPlane(spacing, halfNumSpaces);
    auto handle = std::make_shared<ResourceHandle>(m_engine, grid, ResourceHandle::kPermanent);
    m_engine->resourceCache()->insert(handle);

    return handle;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> PolygonCache::getGridCube(const QString & gridName)
{
    // Get grid of the correct grid size from a given grid name
    QStringList strings = gridName.split("_");
    float spacing = (float)strings[1].toDouble();
    int halfNumSpaces = strings[2].toInt();
    return getGridCube(spacing, halfNumSpaces);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> PolygonCache::getGridCube(float spacing, int halfNumSpaces)
{
    // Check if grid exists
    const QString& gridStr = getGridCubeName(spacing, halfNumSpaces);
    if (m_engine->resourceCache()->handleWithName(gridStr, Resource::kMesh)) {
        return m_engine->resourceCache()->getMesh(gridStr);
    }

    // Create grid and resource handle
    std::shared_ptr<Mesh> grid = createGridCube(spacing, halfNumSpaces);
    auto handle = std::make_shared<ResourceHandle>(m_engine, grid, ResourceHandle::kPermanent);
    m_engine->resourceCache()->insert(handle);

    return handle;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> PolygonCache::getCylinder(const QString & cylinderName)
{
    // Get cylinder with correct specifications from name
    QStringList strings = cylinderName.split("_");
    float baseRadius = (float)strings[1].toDouble();
    float topRadius = (float)strings[2].toDouble();
    float height = (float)strings[3].toDouble();
    int sectorCount = 36;
    int stackCount = 1;
    if (strings.size() >= 5) {
        sectorCount = strings[4].toInt();
        stackCount = strings[5].toInt();
    }
    return getCylinder(baseRadius, topRadius, height, sectorCount, stackCount);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> PolygonCache::getCylinder(float baseRadius, float topRadius, 
    float height, int sectorCount, int stackCount)
{
    // Check if cylinder exists
    const QString& cylinderStr = getCylinderName(baseRadius, topRadius,
        height, sectorCount, stackCount);
    if (m_engine->resourceCache()->handleWithName(cylinderStr, Resource::kMesh)) {
        return m_engine->resourceCache()->getMesh(cylinderStr);
    }

    // Create cylinder and resource handle
    std::shared_ptr<Mesh> grid = createCylinder(baseRadius, topRadius,
        height, sectorCount, stackCount);
    auto handle = std::make_shared<ResourceHandle>(m_engine, grid, ResourceHandle::kPermanent);
    m_engine->resourceCache()->insert(handle);

    return handle;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> PolygonCache::getCapsule(const QString & capsuleName)
{
    // Get capsule with correct specifications from name
    QStringList strings = capsuleName.split("_");
    float radius = (float)strings[1].toDouble();
    float halfHeight = (float)strings[2].toDouble();
    return getCapsule(radius, halfHeight);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> PolygonCache::getCapsule(float radius, float halfHeight)
{
    // Check if capsule exists
    const QString& capsuleStr = getCapsuleName(radius, halfHeight);
    if (m_engine->resourceCache()->handleWithName(capsuleStr, Resource::kMesh)) {
        return m_engine->resourceCache()->getMesh(capsuleStr);
    }

    // Create capsule and resource handle
    std::shared_ptr<Mesh> grid = createCapsule(radius, halfHeight);
    auto handle = std::make_shared<ResourceHandle>(m_engine, grid, ResourceHandle::kPermanent);
    m_engine->resourceCache()->insert(handle);

    return handle;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> PolygonCache::getSquare()
{
    // Check if rectangle exists
    const QString& rectStr = POLYGON_NAMES.at(kRectangle);
    if (m_engine->resourceCache()->handleWithName(rectStr, Resource::kMesh)) {
        return m_engine->resourceCache()->getMesh(rectStr);
    }

    // Create square and resource handle
    std::shared_ptr<Mesh> square = createRectangle(1, 1, 0, QOpenGLBuffer::StaticDraw);
    auto handle = std::make_shared<ResourceHandle>(m_engine, square, ResourceHandle::kPermanent);
    m_engine->resourceCache()->insert(handle);

    return handle;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> PolygonCache::getCube()
{
    // Check if cube found
    const QString& cubeStr = POLYGON_NAMES.at(kCube);
    if (m_engine->resourceCache()->handleWithName(cubeStr, Resource::kMesh)) {
        return m_engine->resourceCache()->getMesh(cubeStr);
    }

    // Create cube if not found
    std::shared_ptr<Mesh> cube = createCube(QOpenGLBuffer::StaticDraw);
    auto handle = std::make_shared<ResourceHandle>(m_engine, cube, ResourceHandle::kPermanent);
    m_engine->resourceCache()->insert(handle);

    return handle;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> PolygonCache::getSphere(const QString & sphereName)
{
    // Get sphere of the correct grid size from a given sphere name
    QStringList strings = sphereName.split("_");
    int latSize = strings[1].toInt();
    int lonSize = strings[2].toInt();
    return getSphere(latSize, lonSize);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> PolygonCache::getSphere(int latSize, int lonSize)
{
    // Check if sphere found
    const QString& sphereStr = getSphereName(latSize, lonSize);
    if (m_engine->resourceCache()->handleWithName(sphereStr, Resource::kMesh)) {
        return m_engine->resourceCache()->getMesh(sphereStr);
    }

    // Create sphere if not found
    std::shared_ptr<Mesh> sphere = createUnitSphere(latSize, lonSize, QOpenGLBuffer::StaticDraw);
    auto handle = std::make_shared<ResourceHandle>(m_engine, sphere, ResourceHandle::kPermanent);
    m_engine->resourceCache()->insert(handle);

    return handle;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
QString PolygonCache::getGridPlaneName(float spacing, int halfNumSpaces)
{
    return QStringLiteral("gridplane") + "_" + QString::number(spacing) + "_" + QString::number(halfNumSpaces);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
QString PolygonCache::getGridCubeName(float spacing, int halfNumSpaces)
{
    return QStringLiteral("gridcube") + "_" + QString::number(spacing) + "_" + QString::number(halfNumSpaces);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
QString PolygonCache::getCylinderName(float baseRadius, float topRadius, 
    float height, int sectorCount, int stackCount)
{
    return QStringLiteral("cylinder") + "_" + QString::number(baseRadius) + "_" + 
        QString::number(topRadius) + "_" + QString::number(height) + 
        "_" + QString::number(sectorCount) + "_" + QString::number(stackCount);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
QString PolygonCache::getCapsuleName(float radius, float halfHeight)
{
    return QStringLiteral("capsule") + "_" + QString::number(radius) + "_" + QString::number(halfHeight);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
QString PolygonCache::getSphereName(int numLatLines, int numLonLines)
{
    return QStringLiteral("latlonsphere") + "_" + QString::number(numLatLines) + "_" + QString::number(numLonLines);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createGridPlane(float spacing,
    int halfNumSpaces,
    QOpenGLBuffer::UsagePattern usage)
{
    // Define vertices
    VertexArrayData vertexData;

    // Vertical lines
    float x = 0;
    float y = 0;
    float halfTotalSize = spacing * halfNumSpaces;
    int count = 0;
    for (int w = -halfNumSpaces; w <= halfNumSpaces; w++) {
        Vector3f bottom(x + w * spacing, y - halfTotalSize, 0);
        Vector3f top(x + w * spacing, y + halfTotalSize, 0);
        Vec::EmplaceBack(vertexData.m_attributes.m_vertices, bottom);
        Vec::EmplaceBack(vertexData.m_attributes.m_vertices, top);
        Vec::EmplaceBack(vertexData.m_indices, count++);
        Vec::EmplaceBack(vertexData.m_indices, count++);
    }

    // Horizontal lines
    for (int h = -halfNumSpaces; h <= halfNumSpaces; h++) {
        Vec::EmplaceBack(vertexData.m_attributes.m_vertices, Vector3f(x - halfTotalSize, y + h * spacing, 0));
        Vec::EmplaceBack(vertexData.m_attributes.m_vertices, Vector3f(x + halfTotalSize, y + h * spacing, 0));
        Vec::EmplaceBack(vertexData.m_indices, count++);
        Vec::EmplaceBack(vertexData.m_indices, count++);
    }

    // Construct final mesh
    QString gridStr = getGridPlaneName(spacing, halfNumSpaces);
    auto mesh = std::make_shared<Mesh>(gridStr, std::move(vertexData), usage);
    return mesh;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createGridCube(float spacing,
    int halfNumSpaces,
    QOpenGLBuffer::UsagePattern usage)
{
    // Define vertices
    VertexArrayData vertexData;

    // Vertical lines
    float x = 0;
    float y = 0;
    float z = 0;
    float halfTotalSize = spacing * halfNumSpaces;
    int count = 0;
    for (int i = -halfNumSpaces; i <= halfNumSpaces; i++) {
        z = i * spacing;
        for (int w = -halfNumSpaces; w <= halfNumSpaces; w++) {
            Vector3f bottom(x + w * spacing, y - halfTotalSize, z);
            Vector3f top(x + w * spacing, y + halfTotalSize, z);
            vertexData.m_attributes.m_vertices.push_back(bottom);
            vertexData.m_attributes.m_vertices.push_back(top);
            vertexData.m_indices.push_back(count++);
            vertexData.m_indices.push_back(count++);
        }

        // Horizontal lines
        for (int h = -halfNumSpaces; h <= halfNumSpaces; h++) {
            vertexData.m_attributes.m_vertices.push_back(Vector3f(x - halfTotalSize, y + h * spacing, z));
            vertexData.m_attributes.m_vertices.push_back(Vector3f(x + halfTotalSize, y + h * spacing, z));
            vertexData.m_indices.push_back(count++);
            vertexData.m_indices.push_back(count++);

        }

        // Draw depth lines
        if (i == halfNumSpaces) continue;
        for (int w = -halfNumSpaces; w <= halfNumSpaces; w++) {
            for (int h = -halfNumSpaces; h <= halfNumSpaces; h++) {
                vertexData.m_attributes.m_vertices.push_back(Vector3f(x + w * spacing, y + h * spacing, z));
                vertexData.m_attributes.m_vertices.push_back(Vector3f(x + w * spacing, y + h * spacing, z + spacing));
                vertexData.m_indices.push_back(count++);
                vertexData.m_indices.push_back(count++);
            }
        }
    }




    // Construct final mesh
    QString gridStr = getGridCubeName(spacing, halfNumSpaces);
    auto mesh = std::make_shared<Mesh>(gridStr, std::move(vertexData), usage);
    return mesh;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createRectangle(real_g height, 
    real_g width, real_g z,
    QOpenGLBuffer::UsagePattern pattern)
{
    // Define vertex positions
    Vector3f p21(-0.5f * width, 0.5f * height, z);
    Vector3f p22(-0.5f * width, -0.5f * height, z);
    Vector3f p23(0.5f * width, -0.5f * height, z);
    Vector3f p24(0.5f * width, 0.5f * height, z);

    // Define indices
    std::vector<GLuint> indices = {
        0,1,3,//top left triangle (v0, v1, v3)
        3,1,2//bottom right triangle (v3, v1, v2)
    };

    Gb::VertexArrayData meshData;
    meshData.m_indices.swap(indices);

    Color white = Color(QColor(255, 255, 255));
    Vector4f whiteVec = white.toVector4g();

    Vector2f uv1(0.0f, 1.0f);
    Vector2f uv2(0.0f, 0.0f);
    Vector2f uv3(1.0f, 0.0f);
    Vector2f uv4(1.0f, 1.0f);

    meshData.m_attributes.m_vertices = { p21, p22, p23, p24 };
    meshData.m_attributes.m_texCoords = { uv1, uv2, uv3, uv4 };
    meshData.m_attributes.m_colors = {whiteVec, whiteVec, whiteVec, whiteVec };

    const QString& rectStr = POLYGON_NAMES.at(kRectangle);
    auto mesh = std::make_shared<Mesh>(rectStr, std::move(meshData), pattern);

    return mesh;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createCube(QOpenGLBuffer::UsagePattern pattern)
{
    // Color
    Color white = Color(QColor(255, 255, 255));
    Vector4f whiteVec = white.toVector4g();

    // Define vertices
    Gb::VertexArrayData meshData;
    for (size_t i = 0; i < PolygonCache::CUBE_VERTEX_POSITIONS.size(); i++) {
        Vec::EmplaceBack(meshData.m_attributes.m_vertices, PolygonCache::CUBE_VERTEX_POSITIONS[i]);
        Vec::EmplaceBack(meshData.m_attributes.m_normals, PolygonCache::CUBE_NORMALS[i]);
        Vec::EmplaceBack(meshData.m_attributes.m_colors, whiteVec);
        Vec::EmplaceBack(meshData.m_attributes.m_texCoords, PolygonCache::CUBE_VERTEX_UVS_IDENTICAL[i]);
    }
    meshData.m_indices = PolygonCache::CUBE_VERTEX_INDICES;

    const QString& cubeStr = POLYGON_NAMES.at(kCube);
    auto mesh = std::make_shared<Mesh>(cubeStr, std::move(meshData), pattern);

    return mesh;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createUnitSphere(int numLatLines,
    int numLonLines, 
    QOpenGLBuffer::UsagePattern pattern)
{
    // Define vertices
    Gb::VertexArrayData meshData;

    float radius = 1.0;
    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    // Get number of stacks and sectors
    int sectorCount = std::max(1, numLonLines - 1);
    int stackCount = std::max(1, numLatLines - 1);

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            meshData.m_attributes.m_vertices.push_back(Vector3f(x, y, z));

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            meshData.m_attributes.m_normals.push_back(Vector3f(nx, ny, nz));

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            meshData.m_attributes.m_texCoords.push_back(Vector2f(s, t));
        }
    }

    // Generate indices
    // generate CCW index list of sphere triangles
    int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0)
            {
                meshData.m_indices.emplace_back(k1);
                meshData.m_indices.emplace_back(k2);
                meshData.m_indices.emplace_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1))
            {
                meshData.m_indices.emplace_back(k1 + 1);
                meshData.m_indices.emplace_back(k2);
                meshData.m_indices.emplace_back(k2 + 1);
            }
        }
    }

    // Construct final mesh
    const QString& sphereStr = getSphereName(numLatLines, numLonLines);
    auto mesh = std::make_shared<Mesh>(sphereStr, std::move(meshData), pattern);
    return mesh;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createCylinder(float baseRadius, float topRadius, float height, 
    int sectorCount, int stackCount)
{
    QString cylinderStr = getCylinderName(baseRadius, topRadius, height, sectorCount, stackCount);
    auto cylinder = Cylinder(baseRadius, topRadius, height, sectorCount, stackCount);
    auto mesh = std::make_shared<Mesh>(cylinderStr, std::move(*cylinder.vertexData()), QOpenGLBuffer::StaticDraw);
    return mesh;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createCapsule(float radius, float halfHeight)
{
    QString capsuleStr = getCapsuleName(radius, halfHeight);
    auto capsule = Capsule(radius, halfHeight);
    auto mesh = std::make_shared<Mesh>(capsuleStr, std::move(*capsule.vertexData()), QOpenGLBuffer::StaticDraw);
    return mesh;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void PolygonCache::addTriangle(std::vector<Vector3g>& vertices,
    std::vector<GLuint>& indices,
    const Vector3g & v0, const Vector3g & v1, const Vector3g & v2, bool clockWise)
{
    int base = vertices.size();
    Vec::EmplaceBack(vertices, v0);
    Vec::EmplaceBack(vertices, v1);
    Vec::EmplaceBack(vertices, v2);

    if (clockWise) {
        indices.emplace_back(base);
        indices.emplace_back(base + 1);
        indices.emplace_back(base + 2);
    }
    else {
        indices.emplace_back(base);
        indices.emplace_back(base + 2);
        indices.emplace_back(base + 1);
    }
}

void PolygonCache::addQuad(std::vector<Vector3g>& vertices, 
    std::vector<GLuint>& indices, 
    const Vector3g & v0, const Vector3g & v1, const Vector3g & v2, const Vector3g & v3)
{
    int base = vertices.size();
    Vec::EmplaceBack(vertices, v0);
    Vec::EmplaceBack(vertices, v1);
    Vec::EmplaceBack(vertices, v2);
    Vec::EmplaceBack(vertices, v3);

    indices.emplace_back(base);
    indices.emplace_back(base + 3);
    indices.emplace_back(base + 1);
    indices.emplace_back(base + 2);
    indices.emplace_back(base + 1);
    indices.emplace_back(base + 3);
}




////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces