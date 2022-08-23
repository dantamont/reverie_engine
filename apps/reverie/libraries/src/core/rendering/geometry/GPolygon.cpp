#include "core/rendering/geometry/GPolygon.h"

#include <QRegularExpression>
#include "core/GCoreEngine.h"
#include "core/rendering/geometry/GMesh.h"
#include "core/resource/GResourceCache.h"
#include "core/resource/GResource.h"
#include "fortress/constants/GConstants.h"
#include "fortress/containers/GColor.h"
#include "core/rendering/geometry/GCylinder.h"
#include "core/rendering/geometry/GCapsule.h"

namespace rev {

std::vector<Vector3f> PolygonCache::s_cubeVertexPositions = {
    // Front face
     0.5*Vector3f(-1.0, -1.0,  1.0),
     0.5*Vector3f(1.0, -1.0,  1.0),
     0.5*Vector3f(1.0,  1.0,  1.0),
     0.5*Vector3f(-1.0,  1.0,  1.0),

     // Back face
     0.5*Vector3f(-1.0, -1.0, -1.0),
     0.5*Vector3f(-1.0,  1.0, -1.0),
     0.5*Vector3f(1.0,  1.0, -1.0),
     0.5*Vector3f(1.0, -1.0, -1.0),

     // Top face
     0.5*Vector3f(-1.0,  1.0, -1.0),
     0.5*Vector3f(-1.0,  1.0,  1.0),
     0.5*Vector3f(1.0,  1.0,  1.0),
     0.5*Vector3f(1.0,  1.0, -1.0),

     // Bottom face
     0.5*Vector3f(-1.0, -1.0, -1.0),
     0.5*Vector3f(1.0, -1.0, -1.0),
     0.5*Vector3f(1.0, -1.0,  1.0),
     0.5*Vector3f(-1.0, -1.0,  1.0),

     // Right face
     0.5*Vector3f(1.0, -1.0, -1.0),
     0.5*Vector3f(1.0,  1.0, -1.0),
     0.5*Vector3f(1.0,  1.0,  1.0),
     0.5*Vector3f(1.0, -1.0,  1.0),

     // Left face
     0.5*Vector3f(-1.0, -1.0, -1.0),
     0.5*Vector3f(-1.0, -1.0,  1.0),
     0.5*Vector3f(-1.0,  1.0,  1.0),
     0.5*Vector3f(-1.0,  1.0, -1.0),
};


std::vector<Vector3f> PolygonCache::s_cubeNormals = {
    // Front face
    Vector3f(0.0f, 0.0f, 1.0f),
    Vector3f(0.0f, 0.0f, 1.0f),
    Vector3f(0.0f, 0.0f, 1.0f),
    Vector3f(0.0f, 0.0f, 1.0f),

    // Back face
    Vector3f(0.0f, 0.0f, -1.0f),
    Vector3f(0.0f, 0.0f, -1.0f),
    Vector3f(0.0f, 0.0f, -1.0f),
    Vector3f(0.0f, 0.0f, -1.0f),

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
};


// Define UV coordinates
std::vector<Vector2f> PolygonCache::s_cubeVertexUvsIdentical = {
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


// Define indices
std::vector<GLuint> PolygonCache::s_cubeIndices = {
    0,  1,  2,      0,  2,  3,    // front
    4,  5,  6,      4,  6,  7,    // back
    8,  9,  10,     8,  10, 11,   // top
    12, 13, 14,     12, 14, 15,   // bottom
    16, 17, 18,     16, 18, 19,   // right
    20, 21, 22,     20, 22, 23,   // left
};

bool PolygonCache::IsPolygonName(const GString & name)
{
    // Search for an exact match
    /// @todo Move this into TypeFromName
    bool isPolygon = GBasicPolygonType::IsValidName(name.capitalized());

    if (!isPolygon) {
        // Search for a partial match
        isPolygon = TypeFromName(name) != EBasicPolygonType::eINVALID;
    }

    return isPolygon;
}

GBasicPolygonType PolygonCache::TypeFromName(const GString & name)
{
    static GString matchNumber("([\\d]+\\.?[\\d]*)"); // matches a digit, then optionally a period and optional digits
    GBasicPolygonType type(EBasicPolygonType::eINVALID);
    GString capitalizedName = name.capitalized();
    for(size_t i = 0; i < (size_t)EBasicPolygonType::eCOUNT; i++){
        // Check match
        if (i == (size_t)EBasicPolygonType::eLatLonSphere) {
            // Use regex, e.g. sphere_100_20, sphere_20_14
            QRegularExpression re((const char*)("(latlonsphere)_" + matchNumber + "_" + matchNumber));
            QRegularExpressionMatch match = re.match((const char*)name);
            bool matches = match.hasMatch();
            if (matches) {
                type = (GBasicPolygonType)i;
                break;
            }
        }
        else if (i == (size_t)EBasicPolygonType::eGridPlane) {
            // Use regex
            QRegularExpression re((const char*)("(gridplane)_" + matchNumber + "_" + matchNumber ));
            QRegularExpressionMatch match = re.match((const char*)name);
            bool matches = match.hasMatch();
            if (matches) {
                type = (GBasicPolygonType)i;
                break;
            }
        }
        else if (i == (size_t)(size_t)EBasicPolygonType::eGridCube) {
            // Use regex
            QRegularExpression re((const char*)("(gridcube)_" + matchNumber + "_" + matchNumber));
            QRegularExpressionMatch match = re.match((const char*)name);
            bool matches = match.hasMatch();
            if (matches) {
                type = (GBasicPolygonType)i;
                break;
            }
        }
        else if (i == (size_t)EBasicPolygonType::eCapsule) {
            // Use regex
            QRegularExpression re((const char*)("(capsule)_" + matchNumber + "_" + matchNumber ));
            QRegularExpressionMatch match = re.match((const char*)name);
            bool matches = match.hasMatch();
            if (matches) {
                type = (GBasicPolygonType)i;
                break;
            }
        }
        else if (i == (size_t)EBasicPolygonType::eCylinder) {
            // Use regex
            QRegularExpression re((const char*)("(cylinder)_" + matchNumber + "_" + matchNumber));
            QRegularExpressionMatch match = re.match((const char*)name);
            bool matches = match.hasMatch();
            if (matches) {
                type = (GBasicPolygonType)i;
                break;
            }
        }
        else {
            // For everything else, check for only exact match
            type = GBasicPolygonType::FromString(capitalizedName);
            if (EBasicPolygonType::eINVALID != type) {
                break;
            }
        }
    }
 
    return type;
}

PolygonCache::PolygonCache(CoreEngine* engine):
    m_engine(engine)
{
}

PolygonCache::~PolygonCache()
{
}

void PolygonCache::initializeCoreResources()
{
    // Create cube resource
    Mesh* cubeMesh = getCube();
    cubeMesh->handle()->setCore(true);
}

Mesh* PolygonCache::getPolygon(const QString& polygonName)
{
    GBasicPolygonType type = TypeFromName(polygonName.toStdString());
    switch ((EBasicPolygonType)type) {
    case EBasicPolygonType::eRectangle: {
        return getSquare();
    }
    case EBasicPolygonType::eCube: {
        return getCube();
    }
    case EBasicPolygonType::eLatLonSphere: {
        return getSphere(polygonName);
    }
    case EBasicPolygonType::eGridPlane : {
        return getGridPlane(polygonName);
    }
    case EBasicPolygonType::eGridCube: {
        return getGridCube(polygonName);
    }
    case EBasicPolygonType::eCylinder: {
        return getCylinder(polygonName);
    }
    case EBasicPolygonType::eCapsule: {
        return getCapsule(polygonName);
    }
    default:
        Logger::Throw("PolygonCache::getPolygon:: Error, polygon type not implemented");
        break;
    }

    return nullptr;
}

std::unique_ptr<Mesh> PolygonCache::createPolygon(const GString& polygonName,
    std::shared_ptr<ResourceHandle> handle)
{
    // If the polygon exists already, throw an error
    // Commented out, since was causing race condition when loading resources
    //auto exists = getExistingPolygon(polygonName);
    //if (exists) Logger::Throw("Error, polygon already exists");

    GBasicPolygonType type = TypeFromName(polygonName);
    switch ((EBasicPolygonType)type) {
    case EBasicPolygonType::eRectangle: {
        return createSquare();
    }
    case EBasicPolygonType::eCube: {
        return createCube();
    }
    case EBasicPolygonType::eLatLonSphere: {
        return createUnitSphere(polygonName);
    }
    case EBasicPolygonType::eGridPlane: {
        return createGridPlane(polygonName);
    }
    case EBasicPolygonType::eGridCube: {
        return createGridCube(polygonName);
    }
    case EBasicPolygonType::eCylinder: {
        return createCylinder(polygonName);
    }
    case EBasicPolygonType::eCapsule: {
        return createCapsule(polygonName);
    }
    default:
        Logger::Throw("PolygonCache::createPolygon:: Error, polygon type not implemented");
        break;
    }

    return nullptr;
}

Mesh* PolygonCache::getGridPlane(const QString & gridName)
{
    // Get grid of the correct grid size from a given grid name
    QStringList strings = gridName.split("_");
    float spacing = (float)strings[1].toDouble();
    int halfNumSpaces = strings[2].toInt();
    return getGridPlane(spacing, halfNumSpaces);
}

Mesh* PolygonCache::getGridPlane(float spacing, int halfNumSpaces)
{
    // Check if grid exists
    GString gridStr = getGridPlaneName(spacing, halfNumSpaces).toStdString();
    auto exists = getExistingPolygon(gridStr);
    if (exists) return exists;

    // Create grid and resource handle
    std::unique_ptr<Mesh> grid = createGridPlane(spacing, halfNumSpaces);
    std::shared_ptr<ResourceHandle> handle = addToCache(gridStr, std::move(grid));
    return handle->resourceAs<Mesh>();
}

Mesh*  PolygonCache::getGridCube(const QString & gridName)
{
    // Get grid of the correct grid size from a given grid name
    QStringList strings = gridName.split("_");
    float spacing = (float)strings[1].toDouble();
    int halfNumSpaces = strings[2].toInt();
    return getGridCube(spacing, halfNumSpaces);
}

Mesh* PolygonCache::getGridCube(float spacing, int halfNumSpaces)
{
    // Check if grid exists
    GString gridStr = getGridCubeName(spacing, halfNumSpaces).toStdString();
    auto exists = getExistingPolygon(gridStr);
    if (exists) return exists;

    // Create grid and resource handle
    std::unique_ptr<Mesh> grid = createGridCube(spacing, halfNumSpaces);
    std::shared_ptr<ResourceHandle> handle = addToCache(gridStr, std::move(grid));
    return handle->resourceAs<Mesh>();
}

Mesh* PolygonCache::getCylinder(const QString & cylinderName)
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

Mesh* PolygonCache::getCylinder(float baseRadius, float topRadius, 
    float height, int sectorCount, int stackCount)
{
    // Check if cylinder exists
    GString cylinderStr = getCylinderName(baseRadius, topRadius,
        height, sectorCount, stackCount).toStdString();
    auto exists = getExistingPolygon(cylinderStr);
    if (exists) return exists;

    // Create cylinder and resource handle
    std::unique_ptr<Mesh> cylinder = createCylinder(baseRadius, topRadius,
        height, sectorCount, stackCount);
    std::shared_ptr<ResourceHandle> handle = addToCache(cylinderStr, std::move(cylinder));
    return handle->resourceAs<Mesh>();
}

Mesh* PolygonCache::getCapsule(const QString & capsuleName)
{
    // Get capsule with correct specifications from name
    QStringList strings = capsuleName.split("_");
    float radius = (float)strings[1].toDouble();
    float halfHeight = (float)strings[2].toDouble();
    return getCapsule(radius, halfHeight);
}

Mesh* PolygonCache::getCapsule(float radius, float halfHeight)
{
    // Check if capsule exists
    GString capsuleStr = getCapsuleName(radius, halfHeight).toStdString();
    auto exists = getExistingPolygon(capsuleStr);
    if (exists) return exists;

    // Create capsule and resource handle
    std::unique_ptr<Mesh> capsule = createCapsule(radius, halfHeight);
    std::shared_ptr<ResourceHandle> handle = addToCache(capsuleStr, std::move(capsule));
    return handle->resourceAs<Mesh>();
}

Mesh* PolygonCache::getSquare()
{
    // Check if rectangle exists
    const GString& rectStr = GBasicPolygonType::ToString(EBasicPolygonType::eRectangle);
    auto exists = getExistingPolygon(rectStr);
    if (exists) return exists;

    // Create square and resource handle
    std::unique_ptr<Mesh> square = createSquare();
    std::shared_ptr<ResourceHandle> handle = addToCache(rectStr, std::move(square));
    return handle->resourceAs<Mesh>();
}

Mesh* PolygonCache::getCube()
{
    // Check if cube found
    const GString& cubeStr = GBasicPolygonType::ToString(EBasicPolygonType::eCube);
    auto exists = getExistingPolygon(cubeStr);
    if (exists) return exists;

    // Create cube if not found
    std::unique_ptr<Mesh> cube = createCube();
    std::shared_ptr<ResourceHandle> handle = addToCache(cubeStr, std::move(cube));
    return handle->resourceAs<Mesh>();
}

Mesh* PolygonCache::getSphere(const QString & sphereName)
{
    // Get sphere of the correct grid size from a given sphere name
    QStringList strings = sphereName.split("_");
    int latSize = strings[1].toInt();
    int lonSize = strings[2].toInt();
    return getSphere(latSize, lonSize);
}

Mesh* PolygonCache::getSphere(int latSize, int lonSize)
{
    // Check if sphere found
    GString sphereStr = getSphereName(latSize, lonSize).toStdString();
    auto exists = getExistingPolygon(sphereStr);
    if (exists) return exists;

    // Create sphere if not found
    std::unique_ptr<Mesh> sphere = createUnitSphere(latSize, lonSize);
    std::shared_ptr<ResourceHandle> handle = addToCache(sphereStr, std::move(sphere));
    return handle->resourceAs<Mesh>();
}

Mesh* PolygonCache::getExistingPolygon(const GString & name) const
{
    auto handle = ResourceCache::Instance().getHandleWithName(name, EResourceType::eMesh);
    if (handle) {
        return handle->resourceAs<Mesh>();
    }

    return nullptr;
}

std::shared_ptr<ResourceHandle> PolygonCache::addToCache(const GString& name, std::unique_ptr<Mesh> mesh) const
{
    auto handle = ResourceHandle::create(m_engine, (GResourceType)EResourceType::eMesh);
    handle->setResourceType(EResourceType::eMesh);
    //handle->setName(mesh->getName());
    handle->setName(name);
    handle->setRuntimeGenerated(true);
    handle->setResource(std::move(mesh), false);
    handle->setIsLoading(true);
    ResourceCache::Instance().incrementLoadCount();
    emit ResourceCache::Instance().doneLoadingResource(handle->getUuid()); // Need to make sure that post-construction is called

    return handle;
}

QString PolygonCache::getGridPlaneName(float spacing, int halfNumSpaces)
{
    return QStringLiteral("gridplane") + "_" + QString::number(spacing) + "_" + QString::number(halfNumSpaces);
}

QString PolygonCache::getGridCubeName(float spacing, int halfNumSpaces)
{
    return QStringLiteral("gridcube") + "_" + QString::number(spacing) + "_" + QString::number(halfNumSpaces);
}

QString PolygonCache::getCylinderName(float baseRadius, float topRadius, 
    float height, int sectorCount, int stackCount)
{
    return QStringLiteral("cylinder") + "_" + QString::number(baseRadius) + "_" + 
        QString::number(topRadius) + "_" + QString::number(height) + 
        "_" + QString::number(sectorCount) + "_" + QString::number(stackCount);
}

QString PolygonCache::getCapsuleName(float radius, float halfHeight)
{
    return QStringLiteral("capsule") + "_" + QString::number(radius) + "_" + QString::number(halfHeight);
}

QString PolygonCache::getSphereName(int numLatLines, int numLonLines)
{
    return QStringLiteral("latlonsphere") + "_" + QString::number(numLatLines) + "_" + QString::number(numLonLines);
}

std::unique_ptr<Mesh> PolygonCache::createGridPlane(const GString & name)
{
    // Get grid of the correct grid size from a given grid name
    std::vector<GString> strings;
    name.split("_", strings);
    float spacing = (float)strings[1].toDouble();
    int halfNumSpaces = strings[2].toInt();
    std::unique_ptr<Mesh> grid = createGridPlane(spacing, halfNumSpaces);
    return grid;
}

std::unique_ptr<Mesh> PolygonCache::createGridPlane(float spacing, int halfNumSpaces)
{
    // Construct mesh
    auto mesh = std::make_unique<Mesh>();
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

std::unique_ptr<Mesh> PolygonCache::createGridCube(const GString & name)
{
    // Get grid of the correct grid size from a given grid name
    std::vector<GString> strings;
    name.split("_", strings);
    float spacing = (float)strings[1].toDouble();
    int halfNumSpaces = strings[2].toInt();
    std::unique_ptr<Mesh> grid = createGridCube(spacing, halfNumSpaces);
    return grid;
}

std::unique_ptr<Mesh> PolygonCache::createGridCube(float spacing,
    int halfNumSpaces)
{
    // Construct mesh
    auto mesh = std::make_unique<Mesh>();
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

std::unique_ptr<Mesh> PolygonCache::createSquare()
{
    std::unique_ptr<Mesh> square = createRectangle(2, 2, 0);
    return square;
}

std::unique_ptr<Mesh> PolygonCache::createRectangle(Real_t height,
    Real_t width, Real_t z)
{
    auto mesh = std::make_unique<Mesh>();
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

    Color white = Color(255, 255, 255);
    Vector4 whiteVec = white.toVector<Real_t, 4>();

    Vector2f uv1(0.0f, 1.0f);
    Vector2f uv2(0.0f, 0.0f);
    Vector2f uv3(1.0f, 0.0f);
    Vector2f uv4(1.0f, 1.0f);

    meshData.m_attributes.m_vertices = { p21, p22, p23, p24 };
    meshData.m_attributes.m_texCoords = { uv1, uv2, uv3, uv4 };
    meshData.m_attributes.m_colors = {whiteVec, whiteVec, whiteVec, whiteVec };

    // Create bounding box for mesh
    mesh->generateBounds();

    return mesh;
}

std::unique_ptr<Mesh> PolygonCache::createCube()
{
    auto mesh = std::make_unique<Mesh>();
    rev::VertexArrayData& meshData = mesh->vertexData();

    // Color
    Color white = Color(255, 255, 255);
    Vector4 whiteVec = white.toVector<Real_t, 4>();

    // Define vertices
    for (size_t i = 0; i < PolygonCache::s_cubeVertexPositions.size(); i++) {
        Vec::EmplaceBack(meshData.m_attributes.m_vertices, PolygonCache::s_cubeVertexPositions[i]);
        Vec::EmplaceBack(meshData.m_attributes.m_normals, PolygonCache::s_cubeNormals[i]);
        Vec::EmplaceBack(meshData.m_attributes.m_colors, whiteVec);
        Vec::EmplaceBack(meshData.m_attributes.m_texCoords, PolygonCache::s_cubeVertexUvsIdentical[i]);
    }
    meshData.m_indices = PolygonCache::s_cubeIndices;

    // Create bounding box for mesh
    mesh->generateBounds();

    return mesh;
}

std::unique_ptr<Mesh> PolygonCache::createUnitSphere(const GString & name)
{
    // Get sphere of the correct grid size from a given sphere name
    std::vector<GString> strings;
    name.split("_", strings);
    int latSize = strings[1].toInt();
    int lonSize = strings[2].toInt();
    return createUnitSphere(latSize, lonSize);
}

std::unique_ptr<Mesh> PolygonCache::createUnitSphere(int numLatLines, int numLonLines)
{
    // Construct mesh
    auto mesh = std::make_unique<Mesh>();
    rev::VertexArrayData& meshData = mesh->vertexData();

    float radius = 1.0;
    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    // Get number of stacks and sectors
    int sectorCount = std::max(1, numLonLines - 1);
    int stackCount = std::max(1, numLatLines - 1);

    float sectorStep = Constants::TwoPi / sectorCount;
    float stackStep = Constants::Pi / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = Constants::HalfPi - i * stackStep;        // starting from pi/2 to -pi/2
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

    // Create bounding box for mesh
    mesh->generateBounds();
    return mesh;
}

std::unique_ptr<Mesh> PolygonCache::createCylinder(const GString & name)
{
    // Get cylinder with correct specifications from name
    std::vector<GString> strings;
    name.split("_", strings);
    float baseRadius = (float)strings[1].toDouble();
    float topRadius = (float)strings[2].toDouble();
    float height = (float)strings[3].toDouble();
    int sectorCount = 36;
    int stackCount = 1;
    if (strings.size() >= 5) {
        sectorCount = strings[4].toInt();
        stackCount = strings[5].toInt();
    }

    std::unique_ptr<Mesh> cylinder = createCylinder(baseRadius, topRadius,
        height, sectorCount, stackCount);
    return cylinder;
}

std::unique_ptr<Mesh> PolygonCache::createCylinder(float baseRadius, float topRadius, float height,
    int sectorCount, int stackCount)
{
    auto mesh = std::make_unique<Mesh>();
    auto cylinder = Cylinder(mesh->vertexData(), baseRadius, topRadius, height, sectorCount, stackCount);
    
    // Create bounding box for mesh
    mesh->generateBounds();

    return mesh;
}

std::unique_ptr<Mesh> PolygonCache::createCapsule(const GString & name)
{
    // Get capsule with correct specifications from name
    std::vector<GString> strings;
    name.split("_", strings);
    float radius = (float)strings[1].toDouble();
    float halfHeight = (float)strings[2].toDouble();
    std::unique_ptr<Mesh> capsule = createCapsule(radius, halfHeight);
    return capsule;
}

std::unique_ptr<Mesh> PolygonCache::createCapsule(float radius, float halfHeight)
{
    auto mesh = std::make_unique<Mesh>();
    auto capsule = Capsule(mesh->vertexData(), radius, halfHeight);

    // Create bounding box for mesh
    mesh->generateBounds();
    return mesh;
}


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





} // End namespaces