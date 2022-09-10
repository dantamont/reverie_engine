#pragma once
/// @see https://www.trentreed.net/blog/qt5-opengl-part-1-basic-rendering/

// QT
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMutex>

// Internal
#include "core/rendering/GGLFunctions.h"
#include "core/rendering/buffers/GGlBuffer.h"
#include "GUniform.h"
#include "fortress/string/GStringView.h"
#include "fortress/containers/GContainerExtensions.h"
#include "core/resource/GResourceHandle.h"

namespace rev { 

class SceneObject;
class ShaderProgram;
class CubeMap;
class Model;
class Ubo;
class Light;
class GlBuffer;

/// @brief struct containing a uniform's name and type
struct ShaderInputInfo {

    /// @name Static
    /// @{

    enum class InputType {
        kNone = -1,
        kUniform,
        kIn,
        kOut,
        BufferField
    };

    /// @brief Determines if the given value is a valid ShaderVariableType
    static bool IsValidGLType(int typeInt);

    enum class PrecisionQualifier {
        kNone = -1,
        kHigh,
        kMedium,
        kLow
    };

    enum ShaderInputFlag {
        kIsArray = 1 << 0,
        kInBlockOrBuffer = 1 << 1
    };

    /// @}

    /// @name Construction/Destruction
    /// @{

    ShaderInputInfo();
    ShaderInputInfo(const ShaderInputInfo& info);
    ShaderInputInfo(const GString& name, const ShaderVariableType& type, InputType inputType);
    ShaderInputInfo(const GString& name, const ShaderVariableType& type, InputType inputType, bool isArray);
    ShaderInputInfo(const GString& name, const ShaderVariableType& type, InputType inputType, bool isArray, int id);
    ~ShaderInputInfo();

    /// @}

    /// @name Properties
    /// @{
    bool isArray() const { return m_flags.testFlag(kIsArray); }
    bool inBlockOrBuffer() const { return m_flags.testFlag(kInBlockOrBuffer); }

    const GString& name() const { return m_name; }

    /// @}

    /// @name Methods
    /// @{

    /// @brief Return GLSL representation of the buffer
    GString asGLSL() const;

    /// @}

    /// @name Members
    /// @{

    GString m_name;
    //QString m_typeStr = ""; // variable type in glsl, e.g. vec3, vec2, float
    ShaderVariableType m_variableType = ShaderVariableType::kNone; // GLSL type of uniform, supports GL_UNIFORM, GL_PROGRAM_INPUT, GL_PROGRAM_OUTPUT, GL_TRANSFORM_FEEDBACK_VARYING, GL_BUFFER_VARIABLE
    std::type_index m_variableCType; // C++ type info for the uniform
    GString m_variableTypeStr; // Null unless custom type
    InputType m_inputType = InputType::kNone;
    QFlags<ShaderInputFlag> m_flags; // whether the uniform is an array or not
    int m_uniformID = -1; // ID (location) to be set when bound to a shader
    int m_localIndex = -1; // Index of the uniform in the m_uniforms vector of corresponding ShaderProgram object
    int m_arraySize = -1;
    PrecisionQualifier m_precision = PrecisionQualifier::kNone;

    /// @}

private:
};

/// @brief struct containing a shader buffer's metadata
struct ShaderBufferInfo {

    /// @brief Return GLSL representation of the buffer
    GString asGLSL() const;

    GString m_name; // Name of the buffer (internally)
    GString m_variableName; // Name of buffer for use as glsl variable, e.g. lightBuffer.data
    gl::BufferMemoryQualifier m_memoryQualifier = gl::BufferMemoryQualifier::kNone;
    gl::BufferBlockType m_bufferType = gl::BufferBlockType::kShaderStorage;
    gl::ShaderBlockLayout m_blockLayout = gl::ShaderBlockLayout::kStd140;
    int m_bufferBinding = -1; // the index of the buffer binding point associated with the active shader storage block
    unsigned long m_bufferDataSize = 0; // the minimum size in bytes to hold active variables 
    size_t m_numActiveVariables = 0; // the number of active variables associated with the block
    std::vector<ShaderInputInfo> m_fields; // Info about fields of buffer
};

/// @brief struct containing a shader struct's name and fields
struct ShaderStruct {
    GString m_name;
    std::vector<ShaderInputInfo> m_fields;
};


/// @brief Struct containing metadata about a shader function
struct ShaderFunction {

    GString m_name; // Name of the functiion
    ShaderVariableType m_returnType; // Return type of the function
    GString m_definition; // The entire definition of the function
    GString m_definitionStart; // The beginning of the function definition
    GString m_body; // The body of the function (contents of brackets)
};


/// @brief Class representing an openGL shader
/// @detailed Note that QT's abstraction of the shader program is bizarre... "I can't tell what's worse: 
/// that they made setAttributeBuffer a member of a type that has absolutely nothing to do with vertex state"
/// See: https://stackoverflow.com/questions/37999609/combining-vertex-array-handler-with-vertex-buffer-index-buffer
/// @{ When writing shaders, use the precision rule:
/// - highp for vertex positions,
/// - mediump for texture coordinates,
/// - lowp for colors.
/// @}
class Shader : public NameableInterface, public LoadableInterface{
public:
    /// @name Static
    /// @{

    /// @brief Builtin uniform names
    static GString s_worldMatrixUniformName;


    enum ShaderFlags
    {
        kVertexFlag = 0x0001,
        kFragmentFlag = 0x0002,
        kGeometryFlag = 0x0004, //  (requires OpenGL >= 4.0 or OpenGL ES >= 3.2).
        kTessellationControlFlag = 0x0008, //  (requires OpenGL >= 4.0 or OpenGL ES >= 3.2).
        kTessellationEvaluationFlag = 0x0010, //  (requires OpenGL >= 4.0 or OpenGL ES >= 3.2).
        kComputeFlag = 0x0020 // (requires OpenGL >= 4.3 or OpenGL ES >= 3.1).
    };

    enum class ShaderType
    {
        kVertex = 0,
        kFragment = 1,
        kGeometry = 2, //  (requires OpenGL >= 4.0 or OpenGL ES >= 3.2).
        kCompute = 3, // (requires OpenGL >= 4.3 or OpenGL ES >= 3.1).
        kTessellationControl = 4, //  (requires OpenGL >= 4.0 or OpenGL ES >= 3.2).
        kTessellationEvaluation = 5 //  (requires OpenGL >= 4.0 or OpenGL ES >= 3.2).
    };

    /** @brief Obtain map of ShaderType to corresponding GL type
        @param[in]
        @return s_shaderTypeToGLMap
    */
    static std::vector<int>& shaderTypeToGLMap() { return s_shaderTypeToGLMap; }


    /// @brief Make this take in a set of shader names and types, this is insufficient
    // FIXME: See ResourceCache::initializeResources
    static std::vector<std::pair<QString, ShaderType>> Builtins() { return s_builtins; }

    /// @brief Combine shaders into a single source file
    static GString CombineEffectFragmentShaders(std::vector<const Shader*>& shaders);
    static GString CombineEffectVertexShaders(std::vector<const Shader*>& shaders);

    /// @}

    /// @name Constructors/Destructors
    /// @{
    Shader();
    Shader(const nlohmann::json& json);
    Shader(ShaderType type);
    Shader(const QString& file, ShaderType type, bool deferConstruction=true);
    ~Shader();
    /// @{

    /// @name Properties
    /// @{

    const ShaderType& type() const { return m_type; }

    unsigned int getID() const { return m_shaderID; }

    bool isValid() const { return m_isValid; }

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Initialize with source
    void initializeFromSource(const QString& source);

    /// @brief load in a shader from a source file
    /// \see https://stackoverflow.com/questions/6047527/how-to-pass-an-stdstring-to-glshadersource
    /// \see https://wiki.qt.io/Using-QOpenGLFunctions-and-QOpenGLContext
    bool initializeFromSourceFile();


    /// @brief Get source code of the shader
    QString source() const;

    /// @brief Get Uniform info for this shader
    std::vector<ShaderInputInfo>& uniformInfo() { return m_uniforms; }
    const std::vector<ShaderInputInfo>& uniformInfo() const { return m_uniforms; }

    /// @brief Get functions, in the order that they appear in the source code
    void getFunctions(std::vector<ShaderFunction>& outFunctions) const;
    void getFunctions(tsl::robin_map<QString, ShaderFunction>& outFunctions) const;

    /// @brief Get inputs and outputs from the shader
    void getIO(std::vector<ShaderInputInfo>& inputs, std::vector<ShaderInputInfo>& outputs) const;

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Shader& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Shader& orObject);


    /// @}

protected:
    /// @name Friends
    /// @{
    friend class ShaderProgram;

    /// @}

	/// @name Protected methods
	/// @{

    void initializeGL();

    /// @brief Parses shader code to obtain vector of uniform info
    void parseForUniforms();
    void parseUniform(const QRegularExpressionMatch& match);

    /// @brief Parses shader code for defines
    void parseForDefines();
    void parseDefine(const QRegularExpressionMatch& match);

    /// @brief Parses shader code to obtain  map of structs
    void parseForStructs();
    void parseStruct(const QRegularExpressionMatch& match);

    /// @brief Parses shader code to obtain uniform buffers
    void parseForBuffers();
    void parseBuffer(const QRegularExpressionMatch& bufferStr);

    /// @brief Remove comments from the given string
    void removeComments(QString& str);
    void removeComments(std::string& str);

	/// @}

	/// @name Protected members
	/// @{

    /// @brief Whether or not the shader loaded successfully
    bool m_isValid = false;

    ShaderType m_type;

    int m_shaderID = -1;

    /// @brief The structs in the shader
    std::vector<ShaderStruct> m_structs;

    /// @brief the uniform buffers in the shader
    std::vector<ShaderBufferInfo> m_uniformBuffers;

    /// @brief Non-uniform buffers used by the shader
    std::vector<ShaderBufferInfo> m_buffers;

    /// @brief The uniforms in the shader
    std::vector<ShaderInputInfo> m_uniforms;

    /// @brief Define values (integers needed for array sizes)
    tsl::robin_map<QString, int> m_defines;

    /// @brief Source code of the shader 
    /// @detailed Stored as a standard string to avoid dangling pointer in c_str() call
    std::string m_source;

    /// @brief Index is shader type
    static std::vector<int> s_shaderTypeToGLMap;
    static std::vector<QString> s_shaderTypeToStringMap;

    static std::vector<std::pair<QString, ShaderType>> s_builtins;

    // Regex for a uniform:      uniform[\s\t\n\r]*[\w\d]+[\s\t\n\r]*[\w\d]+
    // Regex for a layout block: layout[\s\t\n\r]*\([\w\d]+\)[\s\t\n\r]*uniform[\s\t\n\r]+[\w\d]+[\s\t\n\r]*[\s\t\n\r]*\{([\w\d\s\t\n\r;\/\,]*)\}
    // Regex for a struct:       struct[\s\t\n\r]*[\w\d]+[\s\t\n\r]*\{([\w\s\d\t\n\r;\/\,]*)\}
    static QString UNIFORM_REGEX;
    static QString STRUCT_REGEX;
    static QString UNIFORM_BUFFER_REGEX;
    static QString BUFFER_REGEX;
    static QString COMMENT_REGEX;
    static QString DEFINE_REGEX;
    static QString FUNCTION_START_REGEX;
    static QString OUT_SHADER_REGEX; // e.g., out vec4 fcolor;
    static QString IN_SHADER_REGEX; // e.g., in vec2 texCoords;

	/// @{
};


} // End namespaces
