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
tsl::robin_map<PolygonCache::PolygonType, QString> PolygonCache::POLYGON_NAMES{
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
void PolygonCache::initializeCoreResources()
{
    // Create cube resource
    std::shared_ptr<Mesh> cubeMesh = getCube();
    cubeMesh->handle()->setCore(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::getPolygon(const QString& polygonName)
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
std::shared_ptr<Mesh> PolygonCache::createPolygon(const QString& polygonName,
    std::shared_ptr<ResourceHandle> handle)
{
    // If the polygon exists already, throw an error
    // Commented out, since was causing race condition when loading resources
    //auto exists = getExistingPolygon(polygonName);
    //if (exists) throw("Error, polygon already exists");

    PolygonType type = typeFromName(polygonName);
    switch (type) {
    case kRectangle: {
        return createSquare();
    }
    case kCube: {
        return createCube();
    }
    case kLatLonSphere: {
        return createUnitSphere(polygonName);
    }
    case kGridPlane: {
        return createGridPlane(polygonName);
    }
    case kGridCube: {
        return createGridCube(polygonName);
    }
    case kCylinder: {
        return createCylinder(polygonName);
    }
    case kCapsule: {
        return createCapsule(polygonName);
    }
    default:
        throw("Error, polygon type not implemented");
        break;
    }

    return nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::getGridPlane(const QString & gridName)
{
    // Get grid of the correct grid size from a given grid name
    QStringList strings = gridName.split("_");
    float spacing = (float)strings[1].toDouble();
    int halfNumSpaces = strings[2].toInt();
    return getGridPlane(spacing, halfNumSpaces);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::getGridPlane(float spacing, int halfNumSpaces)
{
    // Check if grid exists
    const QString& gridStr = getGridPlaneName(spacing, halfNumSpaces);
    auto exists = getExistingPolygon(gridStr);
    if (exists) return exists;

    // Create grid and resource handle
    std::shared_ptr<Mesh> grid = createGridPlane(spacing, halfNumSpaces);
    addToCache(grid);
    return grid;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::getGridCube(const QString & gridName)
{
    // Get grid of the correct grid size from a given grid name
    QStringList strings = gridName.split("_");
    float spacing = (float)strings[1].toDouble();
    int halfNumSpaces = strings[2].toInt();
    return getGridCube(spacing, halfNumSpaces);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::getGridCube(float spacing, int halfNumSpaces)
{
    // Check if grid exists
    const QString& gridStr = getGridCubeName(spacing, halfNumSpaces);
    auto exists = getExistingPolygon(gridStr);
    if (exists) return exists;

    // Create grid and resource handle
    std::shared_ptr<Mesh> grid = createGridCube(spacing, halfNumSpaces);
    addToCache(grid);
    return grid;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::getCylinder(const QString & cylinderName)
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
std::shared_ptr<Mesh> PolygonCache::getCylinder(float baseRadius, float topRadius, 
    float height, int sectorCount, int stackCount)
{
    // Check if cylinder exists
    const QString& cylinderStr = getCylinderName(baseRadius, topRadius,
        height, sectorCount, stackCount);
    auto exists = getExistingPolygon(cylinderStr);
    if (exists) return exists;

    // Create cylinder and resource handle
    std::shared_ptr<Mesh> cylinder = createCylinder(baseRadius, topRadius,
        height, sectorCount, stackCount);
    addToCache(cylinder);
    return cylinder;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::getCapsule(const QString & capsuleName)
{
    // Get capsule with correct specifications from name
    QStringList strings = capsuleName.split("_");
    float radius = (float)strings[1].toDouble();
    float halfHeight = (float)strings[2].toDouble();
    return getCapsule(radius, halfHeight);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::getCapsule(float radius, float halfHeight)
{
    // Check if capsule exists
    const QString& capsuleStr = getCapsuleName(radius, halfHeight);
    auto exists = getExistingPolygon(capsuleStr);
    if (exists) return exists;

    // Create capsule and resource handle
    std::shared_ptr<Mesh> capsule = createCapsule(radius, halfHeight);
    addToCache(capsule);
    return capsule;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::getSquare()
{
    // Check if rectangle exists
    const QString& rectStr = POLYGON_NAMES.at(kRectangle);
    auto exists = getExistingPolygon(rectStr);
    if (exists) return exists;

    // Create square and resource handle
    std::shared_ptr<Mesh> square = createSquare();
    addToCache(square);
    return square;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::getCube()
{
    // Check if cube found
    const QString& cubeStr = POLYGON_NAMES.at(kCube);
    auto exists = getExistingPolygon(cubeStr);
    if (exists) return exists;

    // Create cube if not found
    std::shared_ptr<Mesh> cube = createCube();
    addToCache(cube);
    return cube;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::getSphere(const QString & sphereName)
{
    // Get sphere of the correct grid size from a given sphere name
    QStringList strings = sphereName.split("_");
    int latSize = strings[1].toInt();
    int lonSize = strings[2].toInt();
    return getSphere(latSize, lonSize);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::getSphere(int latSize, int lonSize)
{
    // Check if sphere found
    const QString& sphereStr = getSphereName(latSize, lonSize);
    auto exists = getExistingPolygon(sphereStr);
    if (exists) return exists;

    // Create sphere if not found
    std::shared_ptr<Mesh> sphere = createUnitSphere(latSize, lonSize);
    addToCache(sphere);
    return sphere;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::getExistingPolygon(const QString & name) const
{
    auto handle = m_engine->resourceCache()->getHandleWithName(name, Resource::kMesh);
    if (handle) {
        return handle->resourceAs<Mesh>();
    }

    return nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void PolygonCache::addToCache(const std::shared_ptr<Mesh>& mesh) const
{
    auto handle = ResourceHandle::create(m_engine, Resource::kMesh);
    handle->setResourceType(Resource::kMesh);
    handle->setName(mesh->getName());
    handle->setUserGenerated(true);
    handle->setResource(mesh, false);
    handle->setIsLoading(true);
    m_engine->resourceCache()->incrementLoadCount();
    emit m_engine->resourceCache()->doneLoadingResource(handle); // Need to make sure that post-construction is called
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
std::shared_ptr<Mesh> PolygonCache::createGridPlane(const QString & name)
{
    // Get grid of the correct grid size from a given grid name
    QStringList strings = name.split("_");
    float spacing = (float)strings[1].toDouble();
    int halfNumSpaces = strings[2].toInt();
    std::shared_ptr<Mesh> grid = createGridPlane(spacing, halfNumSpaces);
    return grid;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createGridPlane(float spacing, int halfNumSpaces)
{
    // Construct mesh
    QString gridStr = getGridPlaneName(spacing, halfNumSpaces);
    auto mesh = std::make_shared<Mesh>(gridStr);
    VertexArrayData& vertexData = mesh->vertexData();

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

    return mesh;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createGridCube(const QString & name)
{
    // Get grid of the correct grid size from a given grid name
    QStringList strings = name.split("_");
    float spacing = (float)strings[1].toDouble();
    int halfNumSpaces = strings[2].toInt();
    std::shared_ptr<Mesh> grid = createGridCube(spacing, halfNumSpaces);
    return grid;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createGridCube(float spacing,
    int halfNumSpaces)
{
    // Construct mesh
    QString gridStr = getGridCubeName(spacing, halfNumSpaces);
    auto mesh = std::make_shared<Mesh>(gridStr);
    VertexArrayData& vertexData = mesh->vertexData();

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

    return mesh;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createSquare()
{
    std::shared_ptr<Mesh> square = createRectangle(2, 2, 0);
    return square;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createRectangle(real_g height, 
    real_g width, real_g z)
{
    const QString& rectStr = POLYGON_NAMES.at(kRectangle);
    auto mesh = std::make_shared<Mesh>(rectStr);
    VertexArrayData& meshData = mesh->vertexData();

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

    return mesh;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createCube()
{
    const QString& cubeStr = POLYGON_NAMES.at(kCube);
    auto mesh = std::make_shared<Mesh>(cubeStr);
    Gb::VertexArrayData& meshData = mesh->vertexData();

    // Color
    Color white = Color(QColor(255, 255, 255));
    Vector4f whiteVec = white.toVector4g();

    // Define vertices
    for (size_t i = 0; i < PolygonCache::CUBE_VERTEX_POSITIONS.size(); i++) {
        Vec::EmplaceBack(meshData.m_attributes.m_vertices, PolygonCache::CUBE_VERTEX_POSITIONS[i]);
        Vec::EmplaceBack(meshData.m_attributes.m_normals, PolygonCache::CUBE_NORMALS[i]);
        Vec::EmplaceBack(meshData.m_attributes.m_colors, whiteVec);
        Vec::EmplaceBack(meshData.m_attributes.m_texCoords, PolygonCache::CUBE_VERTEX_UVS_IDENTICAL[i]);
    }
    meshData.m_indices = PolygonCache::CUBE_VERTEX_INDICES;

    return mesh;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createUnitSphere(const QString & sphereName)
{
    // Get sphere of the correct grid size from a given sphere name
    QStringList strings = sphereName.split("_");
    int latSize = strings[1].toInt();
    int lonSize = strings[2].toInt();
    return createUnitSphere(latSize, lonSize);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createUnitSphere(int numLatLines, int numLonLines)
{
    // Construct mesh
    const QString& sphereStr = getSphereName(numLatLines, numLonLines);
    auto mesh = std::make_shared<Mesh>(sphereStr);
    Gb::VertexArrayData& meshData = mesh->vertexData();

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

    return mesh;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createCylinder(const QString & name)
{
    // Get cylinder with correct specifications from name
    QStringList strings = name.split("_");
    float baseRadius = (float)strings[1].toDouble();
    float topRadius = (float)strings[2].toDouble();
    float height = (float)strings[3].toDouble();
    int sectorCount = 36;
    int stackCount = 1;
    if (strings.size() >= 5) {
        sectorCount = strings[4].toInt();
        stackCount = strings[5].toInt();
    }

    std::shared_ptr<Mesh> cylinder = createCylinder(baseRadius, topRadius,
        height, sectorCount, stackCount);
    return cylinder;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createCylinder(float baseRadius, float topRadius, float height, 
    int sectorCount, int stackCount)
{
    QString cylinderStr = getCylinderName(baseRadius, topRadius, height, sectorCount, stackCount);
    auto mesh = std::make_shared<Mesh>(cylinderStr);
    auto cylinder = Cylinder(mesh->vertexData(), baseRadius, topRadius, height, sectorCount, stackCount);
    return mesh;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createCapsule(const QString & name)
{
    // Get capsule with correct specifications from name
    QStringList strings = name.split("_");
    float radius = (float)strings[1].toDouble();
    float halfHeight = (float)strings[2].toDouble();
    std::shared_ptr<Mesh> capsule = createCapsule(radius, halfHeight);
    return capsule;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> PolygonCache::createCapsule(float radius, float halfHeight)
{
    QString capsuleStr = getCapsuleName(radius, halfHeight);
    auto mesh = std::make_shared<Mesh>(capsuleStr);
    auto capsule = Capsule(mesh->vertexData(), radius, halfHeight);
    return mesh;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void PolygonCache::addTriangle(std::vector<Vector3>& vertices,
    std::vector<GLuint>& indices,
    const Vector3 & v0, const Vector3 & v1, const Vector3 & v2, bool clockWise)
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

void PolygonCache::addQuad(std::vector<Vector3>& vertices, 
    std::vector<GLuint>& indices, 
    const Vector3 & v0, const Vector3 & v1, const Vector3 & v2, const Vector3 & v3)
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