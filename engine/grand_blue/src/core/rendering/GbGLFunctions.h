/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_GL_EXTRA_FUNCTIONS_H
#define GB_GL_EXTRA_FUNCTIONS_H

//#define GL_GLEXT_PROTOTYPES // Gain multisampling, doesn't work since Qt is compiled with Angle
#include <QOpenGLExtraFunctions>
#include <QDebug>

namespace Gb {  
namespace GL {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Statics
/////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////


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

enum class BufferAccessType {
    kRead = GL_READ_ONLY,
    kWrite = GL_WRITE_ONLY,
    kReadWrite = GL_READ_WRITE
};

enum class RangeBufferAccessType {
    kRead = GL_MAP_READ_BIT,
    kWrite = GL_MAP_WRITE_BIT,
    kReadWrite = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT
};

enum class BufferType {
    kUniformBuffer = GL_UNIFORM_BLOCK, // Storage comes from a UBO
    kShaderStorage = GL_SHADER_STORAGE_BLOCK // Storage comes from an SSB
};

enum class DrawDataMode {
    kStream = GL_STREAM_DRAW,
    kStatic = GL_STATIC_DRAW,
    kDynamic = GL_DYNAMIC_DRAW
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief For loading 3D geometry (models) into memory
/// @detailed Stores positional data of a model in a VAO
class OpenGLFunctions: public QOpenGLExtraFunctions{
     
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /** @property className
        @brief The name of this class
        @details Every subclass should redefine/override this property to
            return its name
    */
    virtual const char* className() const { return "OpenGLFunctions"; }

    /** @property namespaceName
        @brief The full namespace for this class
        @details Every subclass should redefine/override this property to
            return its full namespace.  The built in logging methods will
            use this value for the message category
    */
    virtual const char* namespaceName() const { return "Gb::GL::OpenGLFunctions"; }
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    OpenGLFunctions();
    ~OpenGLFunctions();
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /** @name Public Methods
        @{
    */
    /// @brief Print all errors that have been raised in GL
    static bool printGLError(const QString& errorMessage);

    /// @}
protected:

};

        
        
}} // End namespaces

#endif