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
void VertexArrayData::drawGeometry(int mode) const
{
    if (!m_vao) {
        // Don't render if vao not loaded yet
        logWarning("No mesh vertex data found, returning");
        return;
    }

//#ifdef DEBUG_MODE
//    m_vao->printGLError("Shape::draw:: Error before binding VAO");
//#endif

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

//#ifdef DEBUG_MODE
//    bool error = m_vao->printGLError("Shape::draw:: Error after binding VAO");
//    if (error) {
//        throw("Error, failed to vind VAO");
//    }
//#endif

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
    for (const auto& bufferPair : m_attributeBuffers) {
        m_vao->loadAttributeBuffer(bufferPair.second);
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

    //if (m_name.isEmpty()) {
    //    throw("Error, mesh must have a unique name");
    //}

    //if (m_meshData.isMissingData()) {
    //    throw std::out_of_range("No vertex or index m_data given");
    //}
#endif

    //// Check validity of all child meshes
    //for (Mesh* child : m_children) {
    //    child->checkValidity();
    //}
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
    Resource(uniqueName, kMesh),
    m_skeleton(std::make_shared<Skeleton>(uniqueName)),
    m_usagePattern(QOpenGLBuffer::StaticDraw)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Mesh::Mesh(const QString& uniqueName, QOpenGLBuffer::UsagePattern usagePattern):
    Resource(uniqueName, kMesh),
    m_skeleton(std::make_shared<Skeleton>(uniqueName)),
    m_usagePattern(usagePattern)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Mesh::Mesh(const QString& uniqueName, const Gb::VertexArrayData& data,
    QOpenGLBuffer::UsagePattern usagePattern) :
    Resource(uniqueName, kMesh),
    m_skeleton(std::make_shared<Skeleton>(uniqueName)),
    m_usagePattern(usagePattern)
{
    // Load mesh data and check validity
    m_meshData.emplace(uniqueName, new VertexArrayData(data));
    m_meshData[uniqueName]->setName(uniqueName);

    // Run post-construction if on the main thread, otherwise return
    // Warning, be sure to run this after switching to the correct openGL context
    if (Process::isMainThread()) {
        postConstruction();
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Mesh::~Mesh()
{
    // Delete mesh data associated with this mesh
    for (const std::pair<QString, VertexArrayData*>& meshPair : m_meshData) {
        delete meshPair.second;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Mesh::draw(CoreEngine* core, 
    const std::shared_ptr<Gb::ShaderProgram>& shaderProgram, 
    int mode)
{
    if (m_skeleton->root()) {
        // If there is an actual skeleton, walk node hierarchy and draw
        m_skeleton->root()->draw(core, shaderProgram, mode);
    }
    else {
        // Perform flat draw routine over meshes
        for (const std::pair<QString, VertexArrayData*>& meshPair : m_meshData) {
            const VertexArrayData& mesh = *meshPair.second;

            if (mesh.hasMaterial()) {
                std::shared_ptr<Material> mtl =
                    core->resourceCache()->getMaterial(mesh.m_materialName, false);

                // Draw geometry if there is a material
                if (mtl) {
                    // Bind material
                    if (!mtl->isBound()) mtl->bind(shaderProgram);

                    // Draw
                    mesh.drawGeometry(mode);

                    // Unbind material
                    if (mtl->isBound()) mtl->release();
                }
            }
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Mesh::hasMaterial() const
{
    bool hasMat = true;
    for (const std::pair<QString, VertexArrayData*>& meshPair : m_meshData) {
        const VertexArrayData& mesh = *meshPair.second;
        hasMat |= mesh.hasMaterial();
    }
    return hasMat;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Mesh::getMaterialNames(std::unordered_map<QString, QString>& outMap) const
{
    for (const std::pair<QString, VertexArrayData*>& meshPair : m_meshData) {
        const VertexArrayData& mesh = *meshPair.second;
        Map::Emplace(outMap, m_name, mesh.m_materialName);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Mesh::setMaterialNames(const std::unordered_map<QString, QString>& map)
{
    for (std::pair<const QString, VertexArrayData*>& meshPair : m_meshData) {
        VertexArrayData& mesh = *meshPair.second;

        if (map.find(mesh.m_name) != map.end()) {
            mesh.m_materialName = map.at(mesh.m_name);
        }
        else {
#ifdef DEBUG_MODE
            throw("Error, could not find mesh corresponding to designated material");
#endif
        }

    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Mesh::onRemoval(ResourceCache* cache)
{
    Q_UNUSED(cache)
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Mesh::postConstruction()
{
#ifdef DEBUG_MODE
    logConstructedWarning();
    if (m_isConstructed) {
        return;
    }
    GL::OpenGLFunctions functions;
    functions.printGLError("Error prior to mesh post-construction");
#else
    if (m_isConstructed) {
        return;
    }
#endif


    // Iterate through mesh data
    for (std::pair<const QString, VertexArrayData*>& meshPair : m_meshData) {
        VertexArrayData& mesh = *meshPair.second;

        // Set the usage pattern, ensuring it matches this mesh
        mesh.m_usagePattern = m_usagePattern;

        // Initialize GL buffers for the mesh
        mesh.loadIntoVAO();

        // Set cost to be the size of the newly initialized mesh data
        m_cost = mesh.sizeInMegaBytes();
    }

    // Call parent class construction routine
    Resource::postConstruction();

    //// Perform post-construction for all child meshes
    //for (Mesh* child : m_children) {
    //    child->postConstruction();
    //}
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespacing