#include "core/rendering/geometry/GVertexArrayData.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/renderer/GRenderContext.h"

namespace rev {


VertexArrayData::VertexArrayData():
    m_vao(false, false)
{
}


VertexArrayData::~VertexArrayData()
{
    // Destroy buffers on deletion
    destroyBuffers();
}

quint64 VertexArrayData::sizeInBytes() const
{
    return m_sizeInBytes;
}

unsigned int VertexArrayData::sizeInMegaBytes() const
{
    return ceil(sizeInBytes() / (1000.0 * 1000.0));
}

GlVertexBufferObject& VertexArrayData::getBuffer(MeshVertexAttributeType type)
{
    return m_attributeBuffers[(int)type];
}

void VertexArrayData::drawGeometry(PrimitiveMode mode, int instanceCount) const
{

    if (!m_vao.isCreated()) {
#ifdef DEBUG_MODE

        // Don't render if vao not loaded yet
        Logger::LogWarning("No vertex data found, returning");
#endif
        return;
    }

#ifdef DEBUG_MODE
    bool error = gl::OpenGLFunctions::printGLError("Shape::draw:: Error before binding VAO");
    if (error) {
        Logger::LogInfo("Error in VAO::drawGeometry");
    }
#endif

    bool bound = m_vao.bind();

#ifdef DEBUG_MODE
    if (bound) {
        // Get array size, used element array buffer because a VAO is essentially indices
        gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();
        int size;
        functions.glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
        if (size > 1e8) {
            // Throw error if size is greater than 10 MB
            Logger::Throw("Error, VAO is too large");
        }

#ifdef DEBUG_MODE
        error = gl::OpenGLFunctions::printGLError("Shape::draw:: Error after binding VAO");
        if (error) {
            Logger::Throw("Error, failed to get buffer parameter for VAO");
        }
#endif
    }
    else {
        Logger::Throw("Failed to bind VAO");
    }
#endif

    constexpr uint32_t sizeOfInt = (uint32_t)sizeof(GLuint);
    if (instanceCount == 1) {
        glDrawElements((int)mode,
            m_indexCount,
            GL_UNSIGNED_INT,
            (GLvoid*)(sizeOfInt * 0));
    }
    else {
        gl::OpenGLFunctions::Functions()->glDrawElementsInstanced(
            (int)mode,
            m_indexCount,
            GL_UNSIGNED_INT,
            (GLvoid*)(sizeOfInt * 0),
            instanceCount);
    }

#ifdef DEBUG_MODE
    error = gl::OpenGLFunctions::printGLError("Shape::draw:: Error drawing geometry");
    if (error) {
        Logger::Throw("Error, failed to draw object");
    }
#endif

    m_vao.release();
}

void VertexArrayData::loadIntoVAO(RenderContext& context, const MeshVertexAttributes& data)
{
#ifdef DEBUG_MODE
    // First ensure that the vertex data is valid
    if (int(m_usagePattern) < 35040) {
        Logger::Throw("Error, invalid usage pattern");
    }
#endif

    // Generate buffers and load data
    loadBufferData(context, data);

    // Create and/or bind Vertex Array Object (VAO)
    if (!m_vao.isCreated()) {
        m_vao.initialize(true);
    }
    else {
        m_vao.bind();
    }

    // Load attribute data into VAO
    loadAttributes<MeshVertexAttributes>();

    // Release all 
    release();
}

bool VertexArrayData::hasBuffers() const
{
    constexpr Int32_t indicesBufferIndex = (Int32_t)MeshVertexAttributeType::kIndices;
    return m_attributeBuffers.size() != 0 && m_attributeBuffers[indicesBufferIndex].hasValidId();
}

void VertexArrayData::destroyBuffers()
{
    for (GlVertexBufferObject& buffer : m_attributeBuffers) {
        buffer.destroy();
    }
}

void VertexArrayData::release() 
{
    m_vao.release();
}

void VertexArrayData::loadBufferData(RenderContext& context, const MeshVertexAttributes& attributes) {

    // Populate each enum type
    for_each_enums<
        MeshVertexAttributeType::kPosition,
        MeshVertexAttributeType::kIndices, // Indices are not included in this iteration
        PopulateBufferStruct>(this, context, attributes);

    // Generate index buffer, must be bound after everything else to be recognized by VAO
    createAndPopulateBuffer<MeshVertexAttributeType::kIndices>(context, attributes, gl::BufferType::kIndexBuffer);
}


} // End namespacing