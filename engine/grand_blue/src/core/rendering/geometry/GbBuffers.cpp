#include "GbBuffers.h"

namespace Gb { 
namespace GL {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex Array Object
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VertexArrayObject::VertexArrayObject(bool create, bool bind):
    QOpenGLVertexArrayObject(nullptr) // don't allow parenting for safe memory management
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
    OpenGLFunctions& gl = *OpenGLFunctions::Functions();
    bool error = false;
#ifdef DEBUG_MODE
    error = OpenGLFunctions::printGLError("VAO::bind: Error before binding VAO");
    if (error) {
        if (!isCreated())
            logInfo("Error, VAO not created");
    }
#endif
    //QOpenGLVertexArrayObject::bind();
    gl.glBindVertexArray(objectId());
#ifdef DEBUG_MODE
    error = OpenGLFunctions::printGLError("VAO::bind: Error binding VAO");
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
    OpenGLFunctions::printGLError("Error before VAO initialization");
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
    OpenGLFunctions::printGLError("Error initializing VAO");
#endif

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VertexArrayObject::loadAttributeBuffer(BufferObject& buffer,
    bool bindThis)
{
    // Bind this VAO
    if (bindThis) { bind(); }

    // Bind the buffer
    buffer.bind();

    // Load buffer contents into the specified attribute
    // Array is tightly packed, so don't need stride
    //unsigned int stride = tupleSize * sizeof(real_g);
    int type = int(buffer.m_type);
    uint tupleSize = buffer.getTupleSize();
    switch (buffer.m_type) {
    case BufferAttributeType::kMiscInt:
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
    OpenGLFunctions& gl = *OpenGLFunctions::Functions();
    gl.glEnableVertexAttribArray(attributeIndex);
    gl.glVertexAttribIPointer(attributeIndex, // attribute number in VAO
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
    OpenGLFunctions& gl = *OpenGLFunctions::Functions();
    gl.glEnableVertexAttribArray(attributeIndex);
    gl.glVertexAttribPointer(attributeIndex, // attribute number in VAO
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BufferObject::BufferObject(const BufferObject & other) :
    QOpenGLBuffer(other),
    OpenGLFunctions(),
    m_type(other.m_type)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BufferObject::BufferObject(QOpenGLBuffer::Type bufferType, BufferAttributeType attributeType, GL::UsagePattern pattern) :
    QOpenGLBuffer(bufferType),
    OpenGLFunctions(),
    m_type(attributeType)
{
    if (isCreated()) {
        destroy();
    }
    create();
    bind();
    setUsagePattern((QOpenGLBuffer::UsagePattern)pattern);
#ifdef DEBUG_MODE
    bool error = printGLError("Error creating and binding VBO");
    if (error) {
        qDebug() << QStringLiteral("DO NOT IGNORE ERROR");
    }
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BufferObject::BufferObject() : 
    QOpenGLBuffer(),
    m_type(BufferAttributeType::kNone),
    OpenGLFunctions(false)
{
    if (!isNull()) {
        initializeOpenGLFunctions();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BufferObject::~BufferObject()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int BufferObject::getTupleSize()
{
    switch (m_type) {
    case BufferAttributeType::kPosition:
        return 3;
    case BufferAttributeType::kColor:
        return 4;
    case BufferAttributeType::kTextureCoordinates:
        return 2;
    case BufferAttributeType::kNormal:
        return 3;
    case BufferAttributeType::kTangent:
        return 3;
    case BufferAttributeType::kMiscInt:
        return 4;
    case BufferAttributeType::kMiscReal:
        return 4;
    default:
        throw("Error, type is not handled");
    }
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
} // End namespacing