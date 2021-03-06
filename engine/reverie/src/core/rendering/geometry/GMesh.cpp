#include "GMesh.h"

#include "GSkeleton.h"
#include "../../GCoreEngine.h"
#include "../../resource/GResourceCache.h"
#include "../../processes/GProcess.h"
#include "../models/GModel.h"
#include "../materials/GMaterial.h"
#include "../shaders/GShaderProgram.h"
#include "../../loop/GSimLoop.h"

#include "../../rendering/renderer/GRenderSettings.h"

namespace rev {


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// VertexArrayData
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VertexArrayData::VertexArrayData():
    m_vao(false, false)
{
    // Make sure internal buffer vector can accomadate all types
    m_attributeBuffers.resize((size_t)BufferAttributeType::kMAX_ATTRIBUTE_TYPE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VertexArrayData::~VertexArrayData()
{
    // Destroy buffers on deletion
    destroyBuffers();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
quint64 VertexArrayData::sizeInBytes() const
{
    quint64 len = m_attributes.getSizeInBytes();

    len += m_indices.size() * sizeof(GLuint);

    return len;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int VertexArrayData::sizeInMegaBytes() const
{
    return ceil(sizeInBytes() / (1000.0 * 1000.0));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GL::BufferObject & VertexArrayData::getBuffer(BufferAttributeType type)
{
    if (m_attributeBuffers[(int)type].attributeType() == BufferAttributeType::kNone) {
        throw("Error, buffer object of the specified type not found");
    }
    
    return m_attributeBuffers[(int)type];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayData::drawGeometry(PrimitiveMode mode)
{

    if (!m_vao.isCreated()) {
#ifdef DEBUG_MODE

        // Don't render if vao not loaded yet
        Logger::LogWarning("No vertex data found, returning");
#endif
        return;
    }

#ifdef DEBUG_MODE
    bool error = GL::OpenGLFunctions::printGLError("Shape::draw:: Error before binding VAO");
    if (error) {
        Logger::LogInfo("Error in VAO::drawGeometry");
    }
#endif

    bool bound = m_vao.bind();

#ifdef DEBUG_MODE
    if (bound) {
        // Get array size, used element array buffer because a VAO is essentially indices
        GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();
        int size;
        functions.glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
        if (size > 1e8) {
            // Throw error if size is greater than 10 MB
            throw("Error, VAO is too large");
        }
    }
    else {
        throw("Failed to bind VAO");
    }
#endif

#ifdef DEBUG_MODE
    error = GL::OpenGLFunctions::printGLError("Shape::draw:: Error after binding VAO");
    if (error) {
        throw("Error, failed to vind VAO");
    }
#endif

    glDrawElements((int)mode,
        m_indices.size(),
        GL_UNSIGNED_INT,
        (GLvoid*)(sizeof(GLuint) * 0));

    m_vao.release();

#ifdef DEBUG_MODE
    GL::OpenGLFunctions::printGLError("Shape::draw:: Error drawing geometry");
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayData::loadAttributes()
{
    // Load buffer attributes
    for (GL::BufferObject& buffer : m_attributeBuffers) {
        if (buffer.isNull()) continue;
        m_vao.loadAttributeBuffer(buffer);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayData::loadIntoVAO()
{
    // First ensure that the vertex data is valid
    checkValidity();

    // If the mesh isn't missing data, initialize VAO and load buffer data
    if (!isMissingData()) {
        // Create Vertex Array Object (VAO)
        if (!m_vao.isCreated()) { 
            m_vao.initialize(true); 
        }
        else {
            m_vao.bind();
        }

        // Generate buffers and load data
        loadBufferData();

        // Load attribute data into VAO
        loadAttributes();

        // Release all 
        release();

        //// Delete buffer objects
        //for (const auto& bufferPair : m_attributeBuffers) {
        //    bufferPair.second->destroy();
        //}

    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool VertexArrayData::hasBuffers() const
{
    return m_attributeBuffers.size() != 0 && m_indexBuffer.isCreated();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayData::destroyBuffers()
{
    for (GL::BufferObject& buffer : m_attributeBuffers) {
        buffer.destroy();
    }
    m_attributeBuffers.clear();
    if (m_indexBuffer.isCreated()) {
        m_indexBuffer.destroy();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayData::checkValidity()
{
#ifdef DEBUG_MODE
    if (int(m_usagePattern) < 35040) {
        throw("Error, invalid usage pattern");
    }

#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayData::release() {
    m_vao.release();

    for (GL::BufferObject& buffer : m_attributeBuffers) {
        if (buffer.isCreated()) {
            // Label::populateVertexData was calling VertexArrayData::loadIntoVAO, raising warning
            buffer.release();
        }
        //else {
        //    throw("ERROR ERROR ERROR");
        //}
    }

    m_indexBuffer.release();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayData::loadBufferData() {
    // Generate vertex attrib buffers
    rev::VertexAttributes& attributes = m_attributes;
    m_attributeBuffers[(size_t)BufferAttributeType::kPosition] = GL::BufferObject(QOpenGLBuffer::VertexBuffer,
        BufferAttributeType::kPosition, 
        m_usagePattern);
    GL::BufferObject& vertexBuffer = m_attributeBuffers[(size_t)BufferAttributeType::kPosition];
    vertexBuffer.allocate(&attributes.m_vertices[0], attributes.m_vertices.size() * sizeof(Vector3));

    //std::vector<float> readData = positionBuffer()->getContents(0, attributes.m_vertices.size());

    if (attributes.m_normals.size()) {
        m_attributeBuffers[(size_t)BufferAttributeType::kNormal] = GL::BufferObject(QOpenGLBuffer::VertexBuffer, BufferAttributeType::kNormal, m_usagePattern);
        m_attributeBuffers[(size_t)BufferAttributeType::kNormal].allocate(&attributes.m_normals[0], attributes.m_normals.size() * sizeof(Vector3));
    }
    if (attributes.m_texCoords.size()) {
        m_attributeBuffers[(size_t)BufferAttributeType::kTextureCoordinates] = GL::BufferObject(QOpenGLBuffer::VertexBuffer, BufferAttributeType::kTextureCoordinates, m_usagePattern);
        m_attributeBuffers[(size_t)BufferAttributeType::kTextureCoordinates].allocate(&attributes.m_texCoords[0], attributes.m_texCoords.size() * sizeof(Vector2));
#ifdef DEBUG_MODE
        int size = m_attributeBuffers[(size_t)BufferAttributeType::kTextureCoordinates].size();
        if (size < 0) throw("Error, texture buffer is empty");
        //logInfo("Texture buffer loaded with size: " + QString::number(size));
#endif
    }
    if (attributes.m_colors.size()) {
        m_attributeBuffers[(size_t)BufferAttributeType::kColor] = GL::BufferObject(QOpenGLBuffer::VertexBuffer, BufferAttributeType::kColor, m_usagePattern);
        m_attributeBuffers[(size_t)BufferAttributeType::kColor].allocate(&attributes.m_colors[0], attributes.m_colors.size() * sizeof(Vector4));
    }
    if (attributes.m_tangents.size()) {
        m_attributeBuffers[(size_t)BufferAttributeType::kTangent] = GL::BufferObject(QOpenGLBuffer::VertexBuffer, BufferAttributeType::kTangent, m_usagePattern);
        m_attributeBuffers[(size_t)BufferAttributeType::kTangent].allocate(&attributes.m_tangents[0], attributes.m_tangents.size() * sizeof(Vector3));
    }
    if (attributes.m_miscInt.size()) {
        m_attributeBuffers[(size_t)BufferAttributeType::kMiscInt] = GL::BufferObject(QOpenGLBuffer::VertexBuffer, BufferAttributeType::kMiscInt, m_usagePattern);
        m_attributeBuffers[(size_t)BufferAttributeType::kMiscInt].allocate(&attributes.m_miscInt[0], attributes.m_miscInt.size() * sizeof(Vector<int, 4>));
    }
    if (attributes.m_miscReal.size()) {
        m_attributeBuffers[(size_t)BufferAttributeType::kMiscReal] = GL::BufferObject(QOpenGLBuffer::VertexBuffer, BufferAttributeType::kMiscReal, m_usagePattern);
        m_attributeBuffers[(size_t)BufferAttributeType::kMiscReal].allocate(&attributes.m_miscReal[0], attributes.m_miscReal.size() * sizeof(Vector4));
    }

    // Generate index buffer
    std::vector<GLuint>& indices = m_indices;
    m_indexBuffer = GL::BufferObject(QOpenGLBuffer::IndexBuffer, BufferAttributeType::kNone, m_usagePattern);
    m_indexBuffer.allocate(&indices[0], indices.size() * sizeof(GLuint));

}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mesh
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Mesh::Mesh() : Resource()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Mesh::~Mesh()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Mesh::onRemoval(ResourceCache* cache)
{
    Q_UNUSED(cache);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Mesh::postConstruction()
{
    if (!Object::IsMainThread()) {
        throw("Error, must be called on main thread");
    }

#ifdef DEBUG_MODE
    GL::OpenGLFunctions functions;
    functions.printGLError("Error prior to mesh post-construction");
#endif

    // Initialize GL buffers for the mesh
    m_vertexData.loadIntoVAO();

    // Set cost to be the size of the newly initialized mesh data
    m_cost = m_vertexData.sizeInMegaBytes();

    // Call parent class construction routine
    Resource::postConstruction();

#ifdef DEBUG_MODE
    functions.printGLError("Error after mesh post-construction");
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Mesh::generateBounds()
{
    // Initialize limits for bounding box of the mesh
    float minX = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    // Update min/max vertex data
    for (const auto& vertex : m_vertexData.m_attributes.m_vertices) {
        minX = std::min(vertex.x(), minX);
        maxX = std::max(vertex.x(), maxX);
        minY = std::min(vertex.y(), minY);
        maxY = std::max(vertex.y(), maxY);
        minZ = std::min(vertex.z(), minZ);
        maxZ = std::max(vertex.z(), maxZ);
    }

    m_objectBounds.boxData().setMinX(minX);
    m_objectBounds.boxData().setMaxX(maxX);
    m_objectBounds.boxData().setMinY(minY);
    m_objectBounds.boxData().setMaxY(maxY);
    m_objectBounds.boxData().setMinZ(minZ);
    m_objectBounds.boxData().setMaxZ(maxZ);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespacing