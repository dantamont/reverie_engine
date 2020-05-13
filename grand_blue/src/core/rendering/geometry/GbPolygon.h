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
class ResourceCache;
class ResourceHandle;
class CoreEngine;
class Mesh;

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
    static std::unordered_map<PolygonType, QString> POLYGON_NAMES;

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

    /// @brief Retrieve or generate a polygon:
    std::shared_ptr<ResourceHandle> getPolygon(const QString& polygonName);

    /// @brief Retrieve grid with specified spacing
    std::shared_ptr<ResourceHandle> getGridPlane(const QString& gridName);
    std::shared_ptr<ResourceHandle> getGridPlane(float spacing = 1.0, int halfNumSpaces = 5);
    std::shared_ptr<ResourceHandle> getGridCube(const QString& gridName);
    std::shared_ptr<ResourceHandle> getGridCube(float spacing = 1.0, int halfNumSpaces = 5);

    /// @brief Retrieve or generate a cylinder
    std::shared_ptr<ResourceHandle> getCylinder(const QString& cylinderName);
    std::shared_ptr<ResourceHandle> getCylinder(float baseRadius = 1.0, float topRadius = 1.0,
        float height = 1.0, int sectorCount = 36, int stackCount = 1);

    /// @brief Retrieve or generate a capsule
    std::shared_ptr<ResourceHandle> getCapsule(const QString& capsuleName);
    std::shared_ptr<ResourceHandle> getCapsule(float radius = 1.0f, float halfHeight = 1.0f);

    /// @brief Retrieve or generate a rectangle
    std::shared_ptr<ResourceHandle> getSquare();

    /// @brief Retrieve or generate a cube
    /// @details This cube has the same UV coordinates on every side
    std::shared_ptr<ResourceHandle> getCube();

    /// @brief Retrieve or generate a sphere
    std::shared_ptr<ResourceHandle> getSphere(const QString& sphereName);
    std::shared_ptr<ResourceHandle> getSphere(int latSize, int lonSize);

    /// @}

protected:
    friend class DebugManager;

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

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
    static std::shared_ptr<Mesh> createRectangle(real_g height = 1.0f,
        real_g width = 1.0f,
        real_g z = -0.5f,
        QOpenGLBuffer::UsagePattern pattern = QOpenGLBuffer::StaticDraw);

    /// @brief Generate a cube
    /// @details This cube has the same UV coordinates on every side
    static std::shared_ptr<Mesh> createCube(QOpenGLBuffer::UsagePattern pattern = QOpenGLBuffer::StaticDraw);

    /// @brief Generate a verticle grid with the given spacing, at z=0
    std::shared_ptr<Mesh> createGridPlane(float spacing, int numSpaces, QOpenGLBuffer::UsagePattern pattern = QOpenGLBuffer::StaticDraw);
    std::shared_ptr<Mesh> createGridCube(float spacing, int numSpaces, QOpenGLBuffer::UsagePattern pattern = QOpenGLBuffer::StaticDraw);


    /// @brief Generate a sphere (of radius 1.0)
    /// @details This cube has the same UV coordinates on every side
    static std::shared_ptr<Mesh> createUnitSphere(int numLatLines = 30,
        int numLonLines = 30,
        QOpenGLBuffer::UsagePattern pattern = QOpenGLBuffer::StaticDraw);

    /// @brief Generate the vertices for a cylinder
    static std::shared_ptr<Mesh> createCylinder(float baseRadius, float topRadius, float height,
        int sectorCount, int stackCount);

    /// @brief Generate the vertices for a capsule
    static std::shared_ptr<Mesh> createCapsule(float radius, float halfHeight);

    /// @brief Add a triangle using the given vertex data
    static void addTriangle(std::vector<Vector3g>& vertices, std::vector<GLuint>& indices,
        const Vector3g& v0, const Vector3g& v1, const Vector3g& v2, bool clockWise = false);

    /// @brief Add a quad using the given vertex data
    static void addQuad(std::vector<Vector3g>& vertices, std::vector<GLuint>& indices,
        const Vector3g& v0, const Vector3g& v1, const Vector3g& v2, const Vector3g& v3);

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