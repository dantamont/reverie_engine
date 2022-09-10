#pragma once 

// QT
#include <QColor>
#include <QOpenGLExtraFunctions>
#include <QOpenGLBuffer>

// Internal
#include "GVertexData.h"
#include "core/rendering/GGLFunctions.h"
#include "enums/GBasicPolygonTypeEnum.h"
#include "fortress/containers/extern/tsl/robin_map.h"

namespace rev {

class Uuid;
class CoreEngine;
class Mesh;
class ResourceHandle;

/// @class PolygonCache
/// @brief Class for handling the rendering of basic geometry
/// @todo Use templates for everything, don't hard-code funciton names
class PolygonCache {
public:
    /// @name Static members
    /// @{

    /// @brief Return true if the given string matches the name of a polygon type
    static bool IsPolygonName(const GString& name);

    /// @brief Return the type of polygon corresponding to the given name
    static GBasicPolygonType TypeFromName(const GString& name);

    /// @brief Vector of cube vertex positions
    static std::vector<Vector3f> s_cubeVertexPositions;

    /// @brief Vector of cube normalss
    static std::vector<Vector3f> s_cubeNormals;

    /// @brief Vector of cube vertex UV Coordinates with identical UVs on each face
    static std::vector<Vector2f> s_cubeVertexUvsIdentical;

    /// @brief Vector of cube vertex indices
    static std::vector<GLuint> s_cubeIndices;

    /// @}

    /// @name Constructors/Destructors
    /// @{

    PolygonCache(CoreEngine* engine);
    ~PolygonCache();

    /// @}

    /// @name Public methods
    /// @{

    void initializeCoreResources();

    /// @brief Obtain vertex data for the specified polygon name
    MeshVertexAttributes& getVertexData(const GString& polygonName);

    /// @brief Retrieve or generate a polygon:
    Mesh* getPolygon(const QString& polygonName);

    /// @brief Retrieve a polygon and add to the given handle
    std::unique_ptr<Mesh> createPolygon(const GString& polygonName, std::shared_ptr<ResourceHandle> handle);

    /// @brief Retrieve grid with specified spacing
    Mesh* getGridPlane(const QString& gridName);
    Mesh* getGridPlane(float spacing = 1.0, int halfNumSpaces = 5);
    
    /// @brief Retrieve grid cube with specified spacing
    Mesh* getGridCube(const QString& gridName);
    Mesh* getGridCube(float spacing = 1.0, int halfNumSpaces = 5);

    /// @brief Retrieve or generate a cylinder
    Mesh* getCylinder(const QString& cylinderName);
    Mesh* getCylinder(float baseRadius = 1.0, 
        float topRadius = 1.0,
        float height = 1.0, 
        int sectorCount = 36,
        int stackCount = 1);

    /// @brief Retrieve or generate a capsule
    Mesh* getCapsule(const QString& capsuleName);
    Mesh* getCapsule(float radius = 1.0f, float halfHeight = 1.0f);

    /// @brief Retrieve or generate a rectangle
    Mesh* getSquare();

    /// @brief Retrieve or generate a cube
    /// @details This cube has the same UV coordinates on every side
    Mesh* getCube();

    /// @brief Retrieve or generate a sphere
    Mesh* getSphere(const QString& sphereName);
    Mesh* getSphere(int latSize, int lonSize);

    /// @}

protected:
    friend class DebugManager;

    /// @name Protected Methods
    /// @{

    /// @brief Return a polygon if it already exists in resource cache
    Mesh* getExistingPolygon(const GString& name) const;
    
    /// @brief Add mesh to the resource cache
    std::shared_ptr<ResourceHandle> addToCache(const GString& name, std::unique_ptr<Mesh> mesh) const;

    /// @brief Add post construction data to the resource cache
    void addResourcePostConstructionData(const GString& resourceName, const Uuid& handleId) const;

    /// @}

    /// @name Static Protected Methods
    /// @{

    /// @brief Generate a rectangle name
    /// @note Only used for cube, so there aren't unique names for parameter
    static GString getRectangleName();

    /// @brief Generate a grid name
    static QString getGridPlaneName(float spacing, int halfNumSpaces);
    static QString getGridCubeName(float spacing, int halfNumSpaces);

    /// @brief Generate a unique cylinder name
    static QString getCylinderName(float baseRadius, float topRadius, 
        float height, int sectorCount, int stackCount);

    /// @brief Generate a unique cylinder name
    static QString getCapsuleName(float radius, float halfHeight);

    /// @brief Generate a sphere name
    static QString getSphereName(int numLatLines=20, int numLonLines=30);

    /// @brief Generate a rectangle
    std::unique_ptr<Mesh> createSquare();
    std::unique_ptr<Mesh> createRectangle(Real_t height = 1.0f,
        Real_t width = 1.0f,
        Real_t z = -0.5f);

    /// @brief Generate a cube
    /// @details This cube has the same UV coordinates on every side
    std::unique_ptr<Mesh> createCube();

    /// @brief Generate a verticle grid with the given spacing, at z=0
    std::unique_ptr<Mesh> createGridPlane(const GString& name);
    std::unique_ptr<Mesh> createGridPlane(float spacing, int numSpaces);

    /// @brief Generate a verticle grid cube with the given spacing, at z=0
    std::unique_ptr<Mesh> createGridCube(const GString& name);
    std::unique_ptr<Mesh> createGridCube(float spacing, int numSpaces);


    /// @brief Generate a sphere (of radius 1.0)
    /// @details This cube has the same UV coordinates on every side
    std::unique_ptr<Mesh> createUnitSphere(const GString& name);
    std::unique_ptr<Mesh> createUnitSphere(int numLatLines = 30,
        int numLonLines = 30);

    /// @brief Generate the vertices for a cylinder
    std::unique_ptr<Mesh> createCylinder(const GString& name);
    std::unique_ptr<Mesh> createCylinder(float baseRadius, float topRadius, float height,
        int sectorCount, int stackCount);

    /// @brief Generate the vertices for a capsule
    std::unique_ptr<Mesh> createCapsule(const GString& name);
    std::unique_ptr<Mesh> createCapsule(float radius, float halfHeight);

    /// @brief Add a triangle using the given vertex data
    static void AddTriangle(std::vector<Vector3>& vertices, std::vector<GLuint>& indices,
        const Vector3& v0, const Vector3& v1, const Vector3& v2, bool clockWise = false);

    /// @brief Add a quad using the given vertex data
    static void AddQuad(std::vector<Vector3>& vertices, std::vector<GLuint>& indices,
        const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& v3);

    /// @}

    /// @name Protected Members
    /// @{

    CoreEngine* m_engine; ///< Pointer to the core engine
    mutable tsl::robin_map<GString, std::unique_ptr<MeshVertexAttributes>> m_vertexData; ///< Data on CPU side for reference while creating meshes

    /// @}

};


} // End namespaces
