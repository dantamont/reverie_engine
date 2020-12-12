/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_POLYGON_H
#define GB_POLYGON_H
// Internal
#include <map>

// QT
#include <QColor>
#include <QOpenGLExtraFunctions>
#include <QOpenGLBuffer>

// Internal
#include "GbVertexData.h"
#include "../GbGLFunctions.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class Mesh;
class ResourceHandle;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief PolygonCache class
/// @detailed Class for handling the rendering of basic geometry
class PolygonCache {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static members
    /// @{

    /// @brief Enum of polygon types
    enum PolygonType {
        kInvalid = -1,
        kRectangle=0,
        kCube,
        kLatLonSphere, // See: http://www.songho.ca/opengl/gl_sphere.html
        kGridPlane,
        kGridCube,
        kCylinder,
        kCapsule
    };

    /// @brief Return true if the given string matches the name of a polygon type
    static bool isPolygonName(const QString& name);

    /// @brief Return the type of polygon corresponding to the given name
    static PolygonType typeFromName(const QString& name);

    /// @brief QStrings used to name shape meshes
    static tsl::robin_map<PolygonType, QString> POLYGON_NAMES;

    /// @brief Vector of cube vertex positions
    static std::vector<Vector3f> CUBE_VERTEX_POSITIONS;

    /// @brief Vector of cube normalss
    static std::vector<Vector3f> CUBE_NORMALS;

    /// @brief Vector of cube vertex UV Coordinates with identical UVs on each face
    static std::vector<Vector2f> CUBE_VERTEX_UVS_IDENTICAL;

    /// @brief Vector of cube vertex indices
    static std::vector<GLuint> CUBE_VERTEX_INDICES;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructors
    /// @{

    PolygonCache(CoreEngine* engine);
    ~PolygonCache();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    void initializeCoreResources();

    /// @brief Retrieve or generate a polygon:
    std::shared_ptr<Mesh> getPolygon(const QString& polygonName);

    /// @brief Retrieve a polygon and add to the given handle
    std::shared_ptr<Mesh> createPolygon(const QString& polygonName,
        std::shared_ptr<ResourceHandle> handle);

    /// @brief Retrieve grid with specified spacing
    std::shared_ptr<Mesh> getGridPlane(const QString& gridName);
    std::shared_ptr<Mesh> getGridPlane(float spacing = 1.0, int halfNumSpaces = 5);
    
    /// @brief Retrieve grid cube with specified spacing
    std::shared_ptr<Mesh> getGridCube(const QString& gridName);
    std::shared_ptr<Mesh> getGridCube(float spacing = 1.0, int halfNumSpaces = 5);

    /// @brief Retrieve or generate a cylinder
    std::shared_ptr<Mesh> getCylinder(const QString& cylinderName);
    std::shared_ptr<Mesh> getCylinder(float baseRadius = 1.0, 
        float topRadius = 1.0,
        float height = 1.0, 
        int sectorCount = 36,
        int stackCount = 1);

    /// @brief Retrieve or generate a capsule
    std::shared_ptr<Mesh> getCapsule(const QString& capsuleName);
    std::shared_ptr<Mesh> getCapsule(float radius = 1.0f, float halfHeight = 1.0f);

    /// @brief Retrieve or generate a rectangle
    std::shared_ptr<Mesh> getSquare();

    /// @brief Retrieve or generate a cube
    /// @details This cube has the same UV coordinates on every side
    std::shared_ptr<Mesh> getCube();

    /// @brief Retrieve or generate a sphere
    std::shared_ptr<Mesh> getSphere(const QString& sphereName);
    std::shared_ptr<Mesh> getSphere(int latSize, int lonSize);

    /// @}

protected:
    friend class DebugManager;

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Return a polygon if it already exists in resource cache
    std::shared_ptr<Mesh> getExistingPolygon(const QString& name) const;
    
    /// @brief Add mesh to the resource cache
    void addToCache(const std::shared_ptr<Mesh>& mesh) const;

    /// @}


    //--------------------------------------------------------------------------------------------
    /// @name Static Protected Methods
    /// @{

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
    static std::shared_ptr<Mesh> createSquare();
    static std::shared_ptr<Mesh> createRectangle(real_g height = 1.0f,
        real_g width = 1.0f,
        real_g z = -0.5f);

    /// @brief Generate a cube
    /// @details This cube has the same UV coordinates on every side
    static std::shared_ptr<Mesh> createCube();

    /// @brief Generate a verticle grid with the given spacing, at z=0
    static std::shared_ptr<Mesh> createGridPlane(const QString& name);
    static std::shared_ptr<Mesh> createGridPlane(float spacing, int numSpaces);

    /// @brief Generate a verticle grid cube with the given spacing, at z=0
    static std::shared_ptr<Mesh> createGridCube(const QString& name);
    static std::shared_ptr<Mesh> createGridCube(float spacing, int numSpaces);


    /// @brief Generate a sphere (of radius 1.0)
    /// @details This cube has the same UV coordinates on every side
    static std::shared_ptr<Mesh> createUnitSphere(const QString& name);
    static std::shared_ptr<Mesh> createUnitSphere(int numLatLines = 30,
        int numLonLines = 30);

    /// @brief Generate the vertices for a cylinder
    static std::shared_ptr<Mesh> createCylinder(const QString& name);
    static std::shared_ptr<Mesh> createCylinder(float baseRadius, float topRadius, float height,
        int sectorCount, int stackCount);

    /// @brief Generate the vertices for a capsule
    static std::shared_ptr<Mesh> createCapsule(const QString& name);
    static std::shared_ptr<Mesh> createCapsule(float radius, float halfHeight);

    /// @brief Add a triangle using the given vertex data
    static void addTriangle(std::vector<Vector3>& vertices, std::vector<GLuint>& indices,
        const Vector3& v0, const Vector3& v1, const Vector3& v2, bool clockWise = false);

    /// @brief Add a quad using the given vertex data
    static void addQuad(std::vector<Vector3>& vertices, std::vector<GLuint>& indices,
        const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& v3);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Pointer to the core engine
    CoreEngine* m_engine;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Static Protected Members
    /// @{

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif