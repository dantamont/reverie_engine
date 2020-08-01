#ifndef GB_BUFFERS_H
#define GB_BUFFERS_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
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
#include "../../GbObject.h"
#include "../GbGLFunctions.h"
#include "GbVertexData.h"

namespace Gb {  
namespace GL {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class BufferObject;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Class wrapping a QOpenGLVertexArrayObject 
class VertexArrayObject: 
    public QOpenGLVertexArrayObject,
    public OpenGLFunctions,
    public Gb::Object {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Statics
    /// @{

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    VertexArrayObject(bool create = true, bool bind = true);
    ~VertexArrayObject();
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /** @property className
        @brief The name of this class
    */
    virtual const char* className() const { return "VertexArrayObject"; }

    /** @property namespaceName
        @brief The full namespace for this class
    */
    virtual const char* namespaceName() const { return "Gb::GL::VertexArrayObject"; }
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{    

    /// @brief Bind the VAO, returns false if failed
    bool bind();

    /// @brief Initializes the VAO in GL if it does not exist and and binds it
    void initialize(bool bind_ = true);

    /// @brief Load a buffer into the given array attribute
    /// @note Assumes zero offset and stride equivalent to tupleSize * real_g (float or double)
    void loadAttributeBuffer(BufferObject& buffer, bool bindThis = false);



    /// @}
protected:

    /// @brief Loads a set of attribute data into the currently bound VAO
    /// @detailed QT's abstraction fails, for a raw GL example, see:
    /// https://www.khronos.org/opengl/wiki/Tutorial2:_VAOs,_VBOs,_Vertex_and_Fragment_Shaders_(C_/_SDL)
    virtual void loadIntAttribute(int attributeIndex, int tupleSize, int stride, int offset);
    virtual void loadFloatAttribute(int attributeIndex, int tupleSize, int stride, int offset);

};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Class wrapping a QOpenGLBuffer 
class BufferObject :
    public QOpenGLBuffer, 
    public OpenGLFunctions,
    public Gb::Object {

public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Enum of valid buffer data types
    /// @details Enum value represents position in shader
    enum AttributeType {
        kNone=-1,
        kPosition,
        kColor,
        kTextureCoordinates,
        kNormal,
        kTangent,
        kMiscInt,
        kMiscReal,
        kMAX_ATTRIBUTE_TYPE
    };

    /// @brief Create and bind an attribute VBO
    static std::shared_ptr<BufferObject> createAndBindAttributeVBO(
        AttributeType type,
        QOpenGLBuffer::UsagePattern usagePattern = QOpenGLBuffer::DynamicDraw);

    /// @brief Create and bind an index VBO
    static std::shared_ptr<BufferObject> createAndBindIndexVBO(
        QOpenGLBuffer::UsagePattern usagePattern = QOpenGLBuffer::DynamicDraw);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    BufferObject(const BufferObject &other);
    BufferObject(QOpenGLBuffer::Type bufferType, AttributeType attributeType = kNone);
    BufferObject();
    ~BufferObject();;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{
    
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
    void getContents(int offset, int count, std::vector<T>& outContents) {
        // Must bind buffer or will crash
        Q_UNUSED(offset)
        if (m_type != kNone) {
            bind();

            outContents.resize(count);
            unsigned int size = count * sizeof(T);
            bool readData = read(offset, &outContents[0], size);
#ifdef DEBUG_MODE
            if (!readData) {
                throw("Failed to readData");
            }
#endif
        }
    }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /** @property className
        @brief The name of this class
    */
    virtual const char* className() const { return "BufferObject"; }

    /** @property namespaceName
        @brief The full namespace for this class
    */
    virtual const char* namespaceName() const { return "Gb::GL::BufferObject"; }
    /// @}

protected:
    friend class VertexArrayObject;

    unsigned int getTupleSize();

    /// @brief Type of buffer
    AttributeType m_type;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
}} // End namespaces

#endif