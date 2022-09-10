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
#include "core/rendering/buffers/GGlBuffer.h"
#include "core/rendering/GGLFunctions.h"
#include "core/rendering/geometry/GVertexData.h"

namespace rev {  

namespace gl {
class VertexArrayObject;

/// @brief Supported data types for VAO data
enum class VertexBufferDataType {
    kNone = -1,
    kByte = GL_BYTE,
    kUnsignedByte = GL_UNSIGNED_BYTE,
    kShort = GL_SHORT,
    kUnsignedShort = GL_UNSIGNED_SHORT,
    kInt = GL_INT,
    kUnsignedInt = GL_UNSIGNED_INT,
    kFloat = GL_FLOAT,
    kDouble = GL_DOUBLE
};

}


/// @class VertexBufferObject
/// @brief A buffer to be stored in a VAO
class GlVertexBufferObject : public GlBuffer {
public:
    using GlBuffer::GlBuffer;
private:
    friend class gl::VertexArrayObject;
};


namespace gl {

/// @brief Class wrapping a QOpenGLVertexArrayObject 
class VertexArrayObject
{
private:
    static constexpr Uint32_t s_invalidId = Uint32_t(-1);

public:

    /// @name Constructors and Destructors
    /// @{
    VertexArrayObject(bool create, bool bind);
    ~VertexArrayObject();
    /// @}

    /// @name Public methods
    /// @{    

    /// @brief Whether or not the VAO is created
    bool isCreated() const { return m_vaoId != s_invalidId; }

    /// @brief Create the VAO in OpenGL
    /// @return true if created
    bool create();

    /// @brief Destroy the VAO in OpenGL
    void destroy();

    /// @brief Bind the VAO, returns false if failed
    bool bind() const;

    /// @brief Release the VAO
    void release() const;

    /// @brief Initializes the VAO in GL if it does not exist and and binds it
    void initialize(bool bind_ = true);

    /// @brief Load a buffer into the given array attribute
    /// @note Assumes zero offset and stride equivalent to tupleSize * Real_t (float or double)
    /// @tparam VertexAttributesType The specialization of VertexAttributes to use to load in data
    /// @param[in] buffer The VBO
    /// @param[in] attributeIndex the index of the VBO in the VAO
    template<typename VertexAttributes>
    void loadAttributeBuffer(GlVertexBufferObject& buffer, Int32_t attributeIndex) {
        // Get buffer metadata
        static const VertexAttributes::TupleDataInfoArray& s_metadata = VertexAttributes::GetTupleData();
        const VertexAttributes::TupleDataInfo& attributeMetadata = s_metadata[attributeIndex];

        // Bind the buffer
        buffer.bind();

        // Load buffer contents into the specified attribute
        constexpr Uint32_t stride = 0;
        constexpr Uint32_t offset = 0;
        if (attributeMetadata.m_isIntegral) {
            loadAttribute<Int32_t>(attributeIndex, attributeMetadata.m_valueCount, stride, offset);
        }
        else
        {
            loadAttribute<Float32_t>(attributeIndex, attributeMetadata.m_valueCount, stride, offset);
        }
    }

    /// @brief The ID of the VAO
    GLuint vertexArrayObjectId() const { return m_vaoId; }

    /// @}

protected:

    /// @brief Loads a set of attribute data into the currently bound VAO
    /// @tparam Numeric type The numeric value type of the data, e.g. int, float, etc.
    /// @param[in] attributeIndex The index of the attribute in the VAO
    /// @param[in] tupleSize The tuple size of the underlying data in the buffer
    /// @param[in] stride The stride of the attributes packed into the buffer. 0 means that attributes are tightly packed, with no bytes between them
    /// @param[in] offset The byte offset at which the buffer should be pointed to in the VAO
    /// @see https://registry.khronos.org/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml
    template<typename NumericType>
    void loadAttribute(int attributeIndex, int tupleSize, int stride, int offset) {
        OpenGLFunctions& gl = *OpenGLFunctions::Functions();
        gl.glEnableVertexAttribArray(attributeIndex);
        if constexpr (std::is_same_v<NumericType, Int32_t>) {
            /// @todo Support other int data types
            gl.glVertexAttribIPointer(attributeIndex, // attribute number in VAO
                tupleSize, // tuple size
                GL_INT, // data type
                stride, // if attributes are packed tightly into array w/ no bytes between them, then 0
                (void*)offset // offset of data
            );
        }
        else if constexpr (std::is_same_v<NumericType, Float32_t>) {
            gl.glVertexAttribPointer(attributeIndex, // attribute number in VAO
                tupleSize, // tuple size
                GL_FLOAT, // data type
                false, // data not normalized
                stride, // if attributes are packed tightly into array w/ no bytes between them, then 0
                (void*)offset // offset of data
            );
        }
        else if constexpr (std::is_same_v<NumericType, Float64_t>) {
            gl.glVertexAttribLPointer(attributeIndex, // attribute number in VAO
                tupleSize, // tuple size
                GL_DOUBLE, // data type
                false, // data not normalized
                stride, // if attributes are packed tightly into array w/ no bytes between them, then 0
                (void*)offset // offset of data
            );
        }
        else {
            static_assert(false, "Unsupported numeric buffer type");
        }


    }

    GLuint m_vaoId{ s_invalidId }; ///< The ID of the VAO
};


}
} // End namespaces
