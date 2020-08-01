#include "GbMesh.h"

#include "GbSkeleton.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../../processes/GbProcess.h"
#include "../models/GbModel.h"
#include "../materials/GbMaterial.h"
#include "../shaders/GbShaders.h"
#include "../../loop/GbSimLoop.h"

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// VertexArrayData
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VertexArrayData::VertexArrayData(const QString& filepath):
    Object(filepath),
    m_source(filepath)
{
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
GL::BufferObject & VertexArrayData::getBuffer(GL::BufferObject::AttributeType type)
{
    if (!Map::HasKey(m_attributeBuffers, (int)type))
        throw("Error, buffer object of the specified type not found");
    
    return *m_attributeBuffers[(int)type];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayData::drawGeometry(int mode) const
{
    if (!m_vao) {
        // Don't render if vao not loaded yet
        logWarning("No vertex data found, returning");
        return;
    }

#ifdef DEBUG_MODE
    bool error = m_vao->printGLError("Shape::draw:: Error before binding VAO");
    if (error) {
        logInfo("Error in VAO::drawGeometry: " + m_name);
    }
#endif

    bool bound = m_vao->bind();

#ifdef DEBUG_MODE
    if (bound) {
        // Get array size, used element array buffer because a VAO is essentially indices
        GL::OpenGLFunctions functions;
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
    error = m_vao->printGLError("Shape::draw:: Error after binding VAO");
    if (error) {
        throw("Error, failed to vind VAO");
    }
#endif

    glDrawElements(mode,
        m_indices.size(),
        GL_UNSIGNED_INT,
        (GLvoid*)(sizeof(GLuint) * 0));

    m_vao->release();

#ifdef DEBUG_MODE
    m_vao->printGLError("Shape::draw:: Error drawing geometry");
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayData::loadAttributes()
{
    // Load buffer attributes
    for (const std::pair<int, std::shared_ptr<GL::BufferObject>>& bufferPair : m_attributeBuffers) {
        m_vao->loadAttributeBuffer(*bufferPair.second);
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
        if (!m_vao) { 
            m_vao = std::make_shared<GL::VertexArrayObject>(true, true); 
        }
        else {
            m_vao->bind();
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
    return m_attributeBuffers.size() != 0 && m_indexBuffer;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayData::destroyBuffers()
{
    for (const auto& bufferPair : m_attributeBuffers) {
        bufferPair.second->destroy();
    }
    m_attributeBuffers.clear();
    if (m_indexBuffer) {
        m_indexBuffer->destroy();
        m_indexBuffer = nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayData::checkValidity()
{
#ifdef DEBUG_MODE
    if (int(m_usagePattern) < 35040) {
        throw("Error, invalid usage pattern");
    }

    CoreEngine* engine = CoreEngine::engines().begin()->second;
    if (QOpenGLContext::currentContext() != engine->getGLWidgetContext()) {
        throw("Error, invalid context for loading VAO");
    }

#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayData::release() {
    m_vao->release();

    for (const auto& bufferPair : m_attributeBuffers) {
        bufferPair.second->release();
    }

    m_indexBuffer->release();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayData::loadBufferData() {
    // Generate vertex attrib buffers
    Gb::VertexAttributes& attributes = m_attributes;
    std::shared_ptr<GL::BufferObject> vertexBuffer = GL::BufferObject::createAndBindAttributeVBO(GL::BufferObject::kPosition, m_usagePattern);
    vertexBuffer->allocate(&attributes.m_vertices[0], attributes.m_vertices.size() * sizeof(Vector3g));
    m_attributeBuffers[GL::BufferObject::kPosition] = vertexBuffer;

    //std::vector<float> readData = positionBuffer()->getContents(0, attributes.m_vertices.size());

    if (attributes.m_normals.size()) {
        auto normalBuffer = GL::BufferObject::createAndBindAttributeVBO(GL::BufferObject::kNormal, m_usagePattern);
        normalBuffer->allocate(&attributes.m_normals[0], attributes.m_normals.size() * sizeof(Vector3g));
        m_attributeBuffers[GL::BufferObject::kNormal] = normalBuffer;
    }
    if (attributes.m_texCoords.size()) {
        auto textureBuffer = GL::BufferObject::createAndBindAttributeVBO(GL::BufferObject::kTextureCoordinates, m_usagePattern);
        textureBuffer->allocate(&attributes.m_texCoords[0], attributes.m_texCoords.size() * sizeof(Vector2g));
        m_attributeBuffers[GL::BufferObject::kTextureCoordinates] = textureBuffer;
#ifdef DEBUG_MODE
        int size = textureBuffer->size();
        if (size < 0) throw("Error, texture buffer is empty");
        //logInfo("Texture buffer loaded with size: " + QString::number(size));
#endif
    }
    if (attributes.m_colors.size()) {
        auto colorBuffer = GL::BufferObject::createAndBindAttributeVBO(GL::BufferObject::kColor, m_usagePattern);
        colorBuffer->allocate(&attributes.m_colors[0], attributes.m_colors.size() * sizeof(Vector4g));
        m_attributeBuffers[GL::BufferObject::kColor] = colorBuffer;
    }
    if (attributes.m_tangents.size()) {
        auto tangentBuffers = GL::BufferObject::createAndBindAttributeVBO(GL::BufferObject::kTangent, m_usagePattern);
        tangentBuffers->allocate(&attributes.m_tangents[0], attributes.m_tangents.size() * sizeof(Vector3g));
        m_attributeBuffers[GL::BufferObject::kTangent] = tangentBuffers;
    }
    if (attributes.m_miscInt.size()) {
        auto miscIntBuffer = GL::BufferObject::createAndBindAttributeVBO(GL::BufferObject::kMiscInt, m_usagePattern);
        miscIntBuffer->allocate(&attributes.m_miscInt[0], attributes.m_miscInt.size() * sizeof(Vector<int, 4>));
        m_attributeBuffers[GL::BufferObject::kMiscInt] = miscIntBuffer;
    }
    if (attributes.m_miscReal.size()) {
        auto miscRealBuffer = GL::BufferObject::createAndBindAttributeVBO(GL::BufferObject::kMiscReal, m_usagePattern);
        miscRealBuffer->allocate(&attributes.m_miscReal[0], attributes.m_miscReal.size() * sizeof(Vector4g));
        m_attributeBuffers[GL::BufferObject::kMiscReal] = miscRealBuffer;
    }

    // Generate index buffer
    std::vector<GLuint>& indices = m_indices;
    m_indexBuffer = GL::BufferObject::createAndBindIndexVBO(m_usagePattern);
    m_indexBuffer->allocate(&indices[0], indices.size() * sizeof(GLuint));

}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mesh
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Mesh::Mesh(const QString& uniqueName) :
    Resource(uniqueName, kMesh)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Mesh::~Mesh()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Mesh::onRemoval(ResourceCache* cache)
{
    Q_UNUSED(cache)
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Mesh::postConstruction()
{
    if (!Process::isMainThread()) 
        throw("Error, must be called on main thread");

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
} // End namespacing