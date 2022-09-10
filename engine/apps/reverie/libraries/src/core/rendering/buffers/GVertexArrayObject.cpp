#include "core/rendering/buffers/GVertexArrayObject.h"
#include "core/rendering/buffers/GGlBuffer.h"
#include "logging/GLogger.h"

namespace rev { 
namespace gl {

VertexArrayObject::VertexArrayObject(bool create, bool bind)
{
    if (create) {
        initialize(bind);
    }
}

VertexArrayObject::~VertexArrayObject()
{
    destroy();
}

bool VertexArrayObject::create()
{
#ifdef DEBUG_MODE
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        Logger::Throw("VertexArrayObject::create() requires a valid current OpenGL context");
        return false;
    }
#endif

    OpenGLFunctions& gl = *OpenGLFunctions::Functions();
    bool created = false;
    if (m_vaoId == s_invalidId) {
        gl.glGenVertexArrays(1, &m_vaoId);
        created = true;

#ifdef DEBUG_MODE
        bool error = gl.printGLError("Error after VAO creation");
        if (error) {
            Logger::Throw("Error after VAO creation");
        }
#endif
    }
    return created;
}

void VertexArrayObject::destroy()
{
    OpenGLFunctions& gl = *OpenGLFunctions::Functions();
    if (m_vaoId != s_invalidId) {
        gl.glDeleteVertexArrays(1, &m_vaoId);
        m_vaoId = s_invalidId;
    }

#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error after VAO destruction");
    if (error) {
        Logger::Throw("Error after VAO destruction");
    }
#endif
}

bool VertexArrayObject::bind() const
{
    bool error = false;
#ifdef DEBUG_MODE
    error = OpenGLFunctions::printGLError("VAO::bind: Error before binding VAO");
    if (error) {
        if (!isCreated()) {
            Logger::Throw("Error, VAO not created");
        }
    }
#endif

    OpenGLFunctions& gl = *OpenGLFunctions::Functions();
    gl.glBindVertexArray(m_vaoId);
#ifdef DEBUG_MODE
    error = OpenGLFunctions::printGLError("VAO::bind: Error binding VAO");
    if (error) {
        if (!isCreated()) {
            Logger::Throw("VAO was not created");
        }
        GString id = GString::FromNumber(m_vaoId);
        Logger::Throw("Failed to bind VAO with ID " + id);
    }
#endif
    return !error;
}

void VertexArrayObject::release() const
{
    OpenGLFunctions& gl = *OpenGLFunctions::Functions();

#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error before VAO release");
    if (error) {
        Logger::Throw("Error before VAO release");
    }
#endif

    gl.glBindVertexArray(0);

#ifdef DEBUG_MODE
    error = gl.printGLError("Error after VAO release");
    if (error) {
        Logger::Throw("Error after VAO release");
    }
#endif
}


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
        Logger::Throw("Error creating VAO in OpenGL");
    }
#else
    Q_UNUSED(created);
#endif
    if (bind_) {
        bind();
    }

#ifdef DEBUG_MODE
    OpenGLFunctions& gl = *OpenGLFunctions::Functions();
    bool error = gl.printGLError("Error after VAO initialization bind");
    if (error) {
        Logger::Throw("Error after VAO initialization bind");
    }
#endif
}


} // gl
} // rev