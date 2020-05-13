#include "GbBuffers.h"

namespace Gb { 
namespace GL {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex Array Object
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VertexArrayObject::VertexArrayObject(bool create, bool bind):
    QOpenGLVertexArrayObject(nullptr), // don't allow parenting for safe memory management
    OpenGLFunctions()
{
    if (create) initialize(bind);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VertexArrayObject::~VertexArrayObject()
{
    // Qt handles deletion of underlying GL VAO
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool VertexArrayObject::bind()
{
    bool error = false;
#ifdef DEBUG_MODE
    error = printGLError("VAO::bind: Error before binding VAO");
#endif
    //QOpenGLVertexArrayObject::bind();
    glBindVertexArray(objectId());
#ifdef DEBUG_MODE
    error = printGLError("VAO::bind: Error binding VAO");
    if (error) {
        if (!isCreated()) {
            logError("VAO was not created");
        }
        QString id = QString::number(objectId());
        logError("Failed to bind VAO with ID " + id);
    }
#endif
    return !error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayObject::initialize(bool bind_)
{
#ifndef QT_NO_DEBUG_OUTPUT
    printGLError("Error before VAO initialization");
#endif

    // Destroy underlying VAO if it exists already
    if (isCreated()) {
        destroy();
    }

    // Create and bind the VAO 
    bool created = create();
#ifdef DEBUG_MODE
    if (!created) {
        logError("Error creating VAO in OpenGL");
    }
#else
    Q_UNUSED(created);
#endif
    if (bind_) bind();
	
#ifndef QT_NO_DEBUG_OUTPUT
    printGLError("Error initializing VAO");
#endif

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayObject::loadAttributeBuffer(std::shared_ptr<BufferObject> buffer,
    bool bindThis)
{
    // Bind this VAO
    if (bindThis) { bind(); }

    // Bind the buffer
    buffer->bind();

    // Load buffer contents into the specified attribute
    // Array is tightly packed, so don't need stride
    //unsigned int stride = tupleSize * sizeof(real_g);
    int type = int(buffer->m_type);
    uint tupleSize = buffer->getTupleSize();
    switch (buffer->m_type) {
    case BufferObject::kMiscInt:
        loadIntAttribute(type, tupleSize, 0, 0);
        break;
    default:
        loadFloatAttribute(type, tupleSize, 0, 0);
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayObject::loadIntAttribute(int attributeIndex, int tupleSize, int stride, int offset)
{
    // Bind position attribute
    glEnableVertexAttribArray(attributeIndex);
    glVertexAttribIPointer(attributeIndex, // attribute number in VAO
        tupleSize, // tuple size
        GL_INT, // data type
        stride, // if attributes are packed tightly into array w/ no bytes between them, then 0
        (void*)offset // offset of data
    );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayObject::loadFloatAttribute(int attributeIndex,
    int tupleSize,
    int stride,
    int offset)
{
    // Bind position attribute
    glEnableVertexAttribArray(attributeIndex);
    glVertexAttribPointer(attributeIndex, // attribute number in VAO
        tupleSize, // tuple size
        GL_FLOAT, // data type
        false, // data not normalized
        stride, // if attributes are packed tightly into array w/ no bytes between them, then 0
        (void*)offset // offset of data
    );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer Object

std::shared_ptr<BufferObject> BufferObject::createAndBindAttributeVBO(
    AttributeType type,
    QOpenGLBuffer::UsagePattern usagePattern)
{
    std::shared_ptr<BufferObject> vbo = std::make_shared<BufferObject>(QOpenGLBuffer::VertexBuffer, type);
    if (vbo->isCreated()) { 
        vbo->destroy();
    }
    vbo->create();
    vbo->bind();
    vbo->setUsagePattern(usagePattern);
#ifdef DEBUG_MODE
    bool error = vbo->printGLError("Error creating and binding VBO");
    if (error) {
        qDebug() << QStringLiteral("DO NOT IGNORE ERROR");
    }
#endif

    return vbo;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<BufferObject> BufferObject::createAndBindIndexVBO(QOpenGLBuffer::UsagePattern usagePattern)
{
    std::shared_ptr<BufferObject> vbo = std::make_shared<BufferObject>(QOpenGLBuffer::IndexBuffer);
    if (vbo->isCreated()) {
        vbo->destroy();
    }
    vbo->create();
    vbo->bind();
    vbo->setUsagePattern(usagePattern);
#ifdef DEBUG_MODE
    vbo->printGLError("Error creating and binding VBO");
#endif

    return vbo;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BufferObject::BufferObject(const BufferObject & other) :
    QOpenGLBuffer(other),
    OpenGLFunctions(),
    m_type(other.m_type)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BufferObject::BufferObject(QOpenGLBuffer::Type bufferType, AttributeType attributeType) :
    QOpenGLBuffer(bufferType),
    OpenGLFunctions(),
    m_type(attributeType)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BufferObject::BufferObject() : QOpenGLBuffer()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BufferObject::~BufferObject()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<float> BufferObject::getContents(int offset, int count)
{
    Q_UNUSED(offset)
    if (m_type != kNone) {
        bind();

        std::vector<float> vertices(count);
        unsigned int size = count * sizeof(float);
        bool readData = read(0, &vertices[0], size);
#ifdef DEBUG_MODE
        if (!readData) {
            logWarning("Failed to readData");
        }
#endif
        return vertices;
    }

    return std::vector<float>();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int BufferObject::getTupleSize()
{
    switch (m_type) {
    case kPosition:
        return 3;
    case kColor:
        return 4;
    case kTextureCoordinates:
        return 2;
    case kNormal:
        return 3;
    case kTangent:
        return 3;
    case kMiscInt:
        return 4;
    case kMiscReal:
        return 4;
    default:
        throw("Error, type is not handled");
    }
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
} // End namespacing