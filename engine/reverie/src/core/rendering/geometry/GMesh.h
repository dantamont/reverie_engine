#ifndef GB_MESH_H
#define GB_MESH_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard

// QT

// Internal
#include "GBuffers.h"
#include "../../resource/GResource.h"
#include "../../geometry/GMatrix.h"
#include "../../animation/GAnimation.h"
#include "../../containers/GContainerExtensions.h"
#include "../../geometry/GCollisions.h"

namespace rev {
class ObjReader;

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Model;
class Skeleton;
class CoreEngine;
enum class PrimitiveMode;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class VertexArrayData
/// @brief Class representing a set of vertex data
class VertexArrayData {
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    VertexArrayData();
    virtual ~VertexArrayData();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties 
    /// @{

    /// @brief Return vertex array object for this VAO group
    GL::VertexArrayObject& vao() { return m_vao; }

    /// @}


    //---------------------------------------------------------------------------------------
    /// @name Methods 
    /// @{

    /// @brief Perform draw of geometry
    void drawGeometry(PrimitiveMode glMode);

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

    GL::BufferObject& getBuffer(BufferAttributeType type);

    /// @}


    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "VertexArrayObject"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::VertexArrayObject"; }
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Members 
    /// @{

    /// @brief OpenGL VAO, aggregates buffers, but does not store data within itself
    GL::VertexArrayObject m_vao;

    /// @brief The buffers storing data for OpenGL
    std::vector<GL::BufferObject> m_attributeBuffers; // index is of type BufferAttributeType
    
    /// @brief The buffer storing index data
    GL::BufferObject m_indexBuffer;


    // See: https://www.reddit.com/r/opengl/comments/57i9cl/examples_of_when_to_use_gl_dynamic_draw/
    // GL_STATIC_DRAW basically means "I will load this vertex data once and then never change it." This would include any static props or level geometry, but also animated models / particles if you are doing all the animation with vertex shaders on the GPU(modern engines with skeletal animation do this, for example).
    // GL_STREAM_DRAW basically means "I am planning to change this vertex data basically every frame." If you are manipulating the vertices a lot on the CPU, and it's not feasible to use shaders instead, you probably want to use this one. Sprites or particles with complex behavior are often best served as STREAM vertices. While STATIC+shaders is preferable for animated geometry, modern hardware can spew incredible amounts of vertex data from the CPU to the GPU every frame without breaking a sweat, so you will generally not notice the performance impact.
    // GL_DYNAMIC_DRAW basically means "I may need to occasionally update this vertex data, but not every frame." This is the least common one.It's not really suited for most forms of animation since those usually require very frequent updates. Animations where the vertex shader interpolates between occasional keyframe updates are one possible case. A game with Minecraft-style dynamic terrain might try using DYNAMIC, since the terrain changes occur less frequently than every frame. DYNAMIC also tends to be useful in more obscure scenarios, such as if you're batching different chunks of model data in the same vertex buffer, and you occasionally need to move them around.
    // Keep in mind these 3 flags don't imply any hard and fast rules within OpenGL or the hardware. They are just hints so the driver can set things up in a way that it thinks will be the most efficient.
    GL::UsagePattern m_usagePattern = GL::UsagePattern::kStaticDraw;

    /// @brief Vertex attribute data
    // TODO: This is stored in buffers, so maybe don't duplicate here
    VertexAttributes m_attributes;

    /// @brief Vertex indices
    std::vector<GLuint> m_indices;

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
    //Mesh(const GString& uniqueName);
    Mesh();
    ~Mesh();
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties 
    /// @{

    /// @brief Return map of mesh data
    rev::VertexArrayData& vertexData() { return m_vertexData; }
    const rev::VertexArrayData& vertexData() const { return m_vertexData; }

    /// @brief Return bounding box for the mesh
    const AABB& objectBounds() const { return m_objectBounds; }
    AABB& objectBounds() { return m_objectBounds; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Methods
    /// @{

    /// @brief Get the type of resource stored by this handle
    virtual ResourceType getResourceType() const override {
        return ResourceType::kMesh;
    }

    /// @brief What to do on removal from resource cache
    void onRemoval(ResourceCache* cache = nullptr) override;

    /// @brief What action to perform post-construction of the resource
    /// @details For performing any actions that need to be done on the main thread
    virtual void postConstruction() override;

    /// @brief Generate object bounds for the mesh
    void generateBounds();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "Mesh"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::Mesh"; }
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
    rev::VertexArrayData m_vertexData;

    /// @brief The bounds of the mesh
    AABB m_objectBounds;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif