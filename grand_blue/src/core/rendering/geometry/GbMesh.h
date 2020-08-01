#ifndef GB_MESH_H
#define GB_MESH_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard

// QT

// Internal
#include "GbBuffers.h"
#include "../../resource/GbResource.h"
#include "../../geometry/GbMatrix.h"
#include "../../animation/GbAnimation.h"
#include "../../containers/GbContainerExtensions.h"

namespace Gb {
class ObjReader;

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Model;
class Skeleton;
class CoreEngine;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class VertexArrayData
/// @brief Class representing a set of vertex data
class VertexArrayData : public Object {
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    VertexArrayData(const QString& filepath = QString());
    ~VertexArrayData();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties 
    /// @{

    /// @brief Return vertex array object for this VAO group
    std::shared_ptr<GL::VertexArrayObject> vao() { return m_vao; }

    /// @}


    //---------------------------------------------------------------------------------------
    /// @name Methods 
    /// @{

    /// @brief Perform draw of geometry
    void drawGeometry(int glMode) const;

    /// @brief Initialize the data for this mesh in the GL VAO
    void loadIntoVAO();

    /// @brief Whether or not the VAO has buffers
    bool hasBuffers() const;

    /// @brief Destroy all buffers in the VAO
    void destroyBuffers();

    /// @brief Check that the mesh is valid
    void checkValidity();

    /// @brief Release vertex and index buffers
    void release();

    /// @brief Returns the size of the data stored in the mesh vertices
    quint64 sizeInBytes() const;
    unsigned int sizeInMegaBytes() const;

    // Moved to model
    /// @brief Whether or not the mesh data has an associated material
    //bool hasMaterial() const { return !m_materialName.isEmpty(); }

    /// @brief Return whether or not this is empty
    bool isMissingData() const { return m_attributes.empty() || m_indices.empty(); }

    GL::BufferObject& getBuffer(GL::BufferObject::AttributeType type);

    /// @}


    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "VertexArrayObject"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::VertexArrayObject"; }
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Members 
    /// @{

    /// @brief OpenGL buffers
    std::shared_ptr<GL::VertexArrayObject> m_vao = nullptr;
    std::unordered_map<int, std::shared_ptr<GL::BufferObject>> m_attributeBuffers; // index is of type GL::BufferObject::AttributeType
    std::shared_ptr<GL::BufferObject> m_indexBuffer = nullptr;


    // See: https://www.reddit.com/r/opengl/comments/57i9cl/examples_of_when_to_use_gl_dynamic_draw/
    // GL_STATIC_DRAW basically means "I will load this vertex data once and then never change it." This would include any static props or level geometry, but also animated models / particles if you are doing all the animation with vertex shaders on the GPU(modern engines with skeletal animation do this, for example).
    // GL_STREAM_DRAW basically means "I am planning to change this vertex data basically every frame." If you are manipulating the vertices a lot on the CPU, and it's not feasible to use shaders instead, you probably want to use this one. Sprites or particles with complex behavior are often best served as STREAM vertices. While STATIC+shaders is preferable for animated geometry, modern hardware can spew incredible amounts of vertex data from the CPU to the GPU every frame without breaking a sweat, so you will generally not notice the performance impact.
    // GL_DYNAMIC_DRAW basically means "I may need to occasionally update this vertex data, but not every frame." This is the least common one.It's not really suited for most forms of animation since those usually require very frequent updates. Animations where the vertex shader interpolates between occasional keyframe updates are one possible case. A game with Minecraft-style dynamic terrain might try using DYNAMIC, since the terrain changes occur less frequently than every frame. DYNAMIC also tends to be useful in more obscure scenarios, such as if you're batching different chunks of model data in the same vertex buffer, and you occasionally need to move them around.
    // Keep in mind these 3 flags don't imply any hard and fast rules within OpenGL or the hardware. They are just hints so the driver can set things up in a way that it thinks will be the most efficient.
    QOpenGLBuffer::UsagePattern m_usagePattern = QOpenGLBuffer::StaticDraw;

    /// @brief Vertex attribute data
    VertexAttributes m_attributes;

    /// @brief Vertex indices
    std::vector<GLuint> m_indices;

    // Has been moved to model
    ///// @brief Lowercase material name corresponding to this mesh
    //QString m_materialName;

    /// @brief Filepath, if loaded from a file, otherwise is an identifier name
    QString m_source;

    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Friends

    friend class CubeMap;
    friend class ObjReader;
    friend class ModelReader;
    friend class Animation;
    friend class NodeAnimation;
    friend class Mesh;
    friend class SkeletonJoint;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private methods 
    /// @{

    /// @brief Generate VAO and its buffers
    /// @detailed Loads attribute data into the currently bound VAO
    /// @note QT's abstraction for VAO fails, for a raw GL example, see:
    /// https://www.khronos.org/opengl/wiki/Tutorial2:_VAOs,_VBOs,_Vertex_and_Fragment_Shaders_(C_/_SDL)
    virtual void loadAttributes();

    /// @brief Generate VBOS (buffers), and load data
    void loadBufferData();


    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class Mesh
/// @brief Encapsulates a VAO as a resource
class Mesh: public Resource {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    Mesh(const QString& uniqueName);
    ~Mesh();
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Methods
    /// @{

    /// @brief What to do on removal from resource cache
    void onRemoval(ResourceCache* cache = nullptr) override;

    /// @brief What action to perform post-construction of the resource
    /// @details For performing any actions that need to be done on the main thread
    virtual void postConstruction() override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties 
    /// @{

    /// @brief Return map of mesh data
    Gb::VertexArrayData& vertexData() { return m_vertexData; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "Mesh"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::Mesh"; }
    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Friends

    friend class CubeMap;
    friend class ObjReader;
    friend class ModelReader;
    friend class Animation;
    friend class NodeAnimation;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private methods 
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private members 
    /// @{

    /// @brief Map of mesh vertex data
    Gb::VertexArrayData m_vertexData;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif