#pragma once

//#define GL_GLEXT_PROTOTYPES // Gain multisampling, doesn't work since Qt is compiled with Angle
#include <QOpenGLExtraFunctions>
#include <QDebug>

namespace rev {  

class GString;
class GStringView;

namespace gl {

enum class UsagePattern: uint32_t
{
    kStreamDraw = GL_STREAM_DRAW, 
    kStreamRead = GL_STREAM_READ, // GL_STREAM_READ
    kStreamCopy = GL_STREAM_COPY, // GL_STREAM_COPY
    kStaticDraw = GL_STATIC_DRAW, // GL_STATIC_DRAW
    kStaticRead = GL_STATIC_READ, // GL_STATIC_READ
    kStaticCopy = GL_STATIC_COPY, // GL_STATIC_COPY
    kDynamicDraw = GL_DYNAMIC_DRAW, // GL_DYNAMIC_DRAW
    kDynamicRead = GL_DYNAMIC_READ, // GL_DYNAMIC_READ
    kDynamicCopy = GL_DYNAMIC_COPY  // GL_DYNAMIC_COPY
};

/// @brief Block layout options
/// @details Each variable type in GLSL such as int, float and bool are defined to be 
/// four-byte quantities with each entity of 4 bytes represented as N. 
/// Scalar e.g. int or bool: 	 Each scalar has a base alignment of N.
/// Vector 	Either 2N or 4N;     This means that a vec3 has a base alignment of 4N.
/// Array of scalars or vectors: Each element has a base alignment equal to that of a vec4.
/// Matrices:                  	 Stored as a large array of column vectors, where each of those vectors has a base alignment of vec4.
/// Struct: 	                 Equal to the computed size of its elements according to the previous rules, but padded to a multiple of the size of a vec4.
enum class ShaderBlockLayout{
    kShared, // his layout type works like packed, with two exceptions. First, it guarantees that all of the variables defined in the block are considered active; this means nothing is optimized out. Second, it guarantees that the members of the block will have the same layout as a block definition in another program
    kPacked, // This layout type means that the implementation determines everything about how the fields are laid out in the block
    kStd140, // This layout alleviates the need to query the offsets for definitions. The rules of std140 layout explicitly state the layout arrangement of any interface block declared with this layout
    kStd430 // Like Std140, but with some optimizations, namely no longer rounding up to a multiple of 16 bytes
};

/// @brief For loading 3D geometry (models) into memory
/// @detailed Stores positional data of a model in a VAO
class OpenGLFunctions: public QOpenGLExtraFunctions{
public:

    /// @name Static
    /// @{

    static const std::shared_ptr<OpenGLFunctions>& Functions();

    /// @brief Obtain max allowed number of texture units per shader
    static size_t MaxNumTextureUnitsPerShader();

    /// @}

    /// @name Constructors and Destructors
    /// @{
    OpenGLFunctions(bool initialize = true);
    ~OpenGLFunctions();
    /// @}

    /** @name Public Methods
        @{
    */
    /// @brief Print all errors that have been raised in GL
    static bool printGLError(const char* errorMessage);
    static bool printGLError(const GStringView& errorMessage);
    
    /// @brief Callback for printing OpenGL debug messages
    static void QOPENGLF_APIENTRY printMessageCallBack(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

    /// @}

protected:

    /// @brief Static instantiation of gl functions for error reporting
    static std::shared_ptr<OpenGLFunctions> s_glFunctions;

    /// @brief Cached value of max number of texture units
    static GLint s_maxTextureUnits;
};

        
        
}
} // End namespaces
