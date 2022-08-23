#pragma once

// Standard
#include <cstdlib>
#include <memory>
#include <vector>

// QT
#include <QObject>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMetaType>

// Internal
#include "core/rendering/GGLFunctions.h"
#include "GVertexData.h"

namespace rev {  
namespace gl {

class BufferObject;

/// @brief Class wrapping a QOpenGLVertexArrayObject 
class VertexArrayObject: 
    public QOpenGLVertexArrayObject
{
    Q_OBJECT
public:

    /// @name Constructors and Destructors
    /// @{
    VertexArrayObject(bool create, bool bind);
    ~VertexArrayObject();
    /// @}

    /// @name Public methods
    /// @{    

    /// @brief Bind the VAO, returns false if failed
    bool bind();

    /// @brief Initializes the VAO in GL if it does not exist and and binds it
    void initialize(bool bind_ = true);

    /// @brief Load a buffer into the given array attribute
    /// @note Assumes zero offset and stride equivalent to tupleSize * Real_t (float or double)
    void loadAttributeBuffer(BufferObject& buffer, bool bindThis = false);



    /// @}
protected:

    /// @brief Loads a set of attribute data into the currently bound VAO
    /// @detailed QT's abstraction fails, for a raw GL example, see:
    /// https://www.khronos.org/opengl/wiki/Tutorial2:_VAOs,_VBOs,_Vertex_and_Fragment_Shaders_(C_/_SDL)
    virtual void loadIntAttribute(int attributeIndex, int tupleSize, int stride, int offset);
    virtual void loadFloatAttribute(int attributeIndex, int tupleSize, int stride, int offset);

};


/// @brief Class wrapping a QOpenGLBuffer 
// TODO: Replace with my own GlBuffer abstraction that doesn't rely on QOpenGlBuffer
class BufferObject :
    public QOpenGLBuffer, 
    private OpenGLFunctions
{

public:
    /// @name Constructors and Destructors
    /// @{
    BufferObject(const BufferObject &other);
    BufferObject(QOpenGLBuffer::Type bufferType, 
        BufferAttributeType attributeType,
        gl::UsagePattern pattern = gl::UsagePattern::kDynamicDraw);
    BufferObject();
    ~BufferObject();;

    /// @}

    /// @name Properties
    /// @{

    /// @brief Attribute type of buffer
    const BufferAttributeType& attributeType() const {
        return m_type;
    }

    /// @}

    /// @name Public methods
    /// @{

    /// @brief If this is a null buffer
    bool isNull() const {
        return m_type == BufferAttributeType::kNone;
    }
    
    /// @brief Get contents
    /// @details Count is the number of elements to return
    template<typename T>
    void getContents(std::vector<T>& outContents) {
        // Note: Must bind buffer before calling this or will crash
        int s = size();
        if (s < 0) return;
        size_t count = s / sizeof(T);
        getContents(0, count, outContents);
    }

    template<typename T>
    void getContents(int /*offset*/, int count, std::vector<T>& outContents) {
        // Must bind buffer or will crash
        if (m_type != kNone) {
            bind();

            outContents.resize(count);
            unsigned int size = count * sizeof(T);
            bool readData = read(offset, &outContents[0], size);
#ifdef DEBUG_MODE
            if (!readData) {
                Logger::Throw("Failed to readData");
            }
#endif
        }
    }

    /// @}

protected:
    friend class VertexArrayObject;

    unsigned int getTupleSize();

    /// @brief Type of buffer
    BufferAttributeType m_type = BufferAttributeType::kNone;
};



}
} // End namespaces
