// See:
// https://www.trentreed.net/blog/qt5-opengl-part-1-basic-rendering/

#ifndef GB_SHADERS_H
#define GB_SHADERS_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard

// QT
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMutex>

// Internal
#include "../../GbObject.h"
#include "../GbGLFunctions.h"
#include "../buffers/GbGLBuffer.h"
#include "GbUniform.h"
#include "../../containers/GbStringView.h"
#include "../../containers/GbContainerExtensions.h"
#include "../../resource/GbResource.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////////////////////
#define TEXT_SHADER_NAME QStringLiteral("text")
#define UNIFORM_CACHE_SIZE 128

namespace Gb { 

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class SceneObject;
class ShaderProgram;
class CubeMap;
class Model;
class UBO;
class Light;
class GLBuffer;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief struct containing a uniform's name and type
struct ShaderInputInfo {

    //---------------------------------------------------------------------------------------
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

    //---------------------------------------------------------------------------------------
    /// @name Construction/Destruction
    /// @{

    ShaderInputInfo();
    ShaderInputInfo(const ShaderInputInfo& info);
    ShaderInputInfo(const QString& name, const ShaderVariableType& type, InputType inputType);
    ShaderInputInfo(const QString& name, const ShaderVariableType& type, InputType inputType, bool isArray);
    ShaderInputInfo(const QString& name, const ShaderVariableType& type, InputType inputType, bool isArray, int id);
    ~ShaderInputInfo();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    bool isArray() const { return m_flags.testFlag(kIsArray); }
    bool inBlockOrBuffer() const { return m_flags.testFlag(kInBlockOrBuffer); }

    const GString& name() const { return m_name; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Methods
    /// @{

    /// @brief Return GLSL representation of the buffer
    QString asGLSL() const;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Members
    /// @{

    GString m_name;
    //QString m_typeStr = ""; // variable type in glsl, e.g. vec3, vec2, float
    ShaderVariableType m_variableType = ShaderVariableType::kNone; // GLSL type of uniform, supports GL_UNIFORM, GL_PROGRAM_INPUT, GL_PROGRAM_OUTPUT, GL_TRANSFORM_FEEDBACK_VARYING, GL_BUFFER_VARIABLE
    std::type_index m_variableCType; // C++ type info for the uniform
    QString m_variableTypeStr; // Null unless custom type
    InputType m_inputType = InputType::kNone;
    QFlags<ShaderInputFlag> m_flags; // whether the uniform is an array or not
    int m_uniformID = -1; // ID (location) to be set when bound to a shader
    int m_localIndex = -1; // Index of the uniform in the m_uniforms vector of corresponding ShaderProgram object
    int m_arraySize = -1;
    PrecisionQualifier m_precision = PrecisionQualifier::kNone;

    /// @}

private:
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief struct containing a shader buffer's metadata
struct ShaderBufferInfo {

    /// @brief Return GLSL representation of the buffer
    QString asGLSL() const;

    QString m_name; // Name of the buffer (internally)
    QString m_variableName; // Name of buffer for use as glsl variable, e.g. lightBuffer.data
    GL::BufferMemoryQualifier m_memoryQualifier = GL::BufferMemoryQualifier::kNone;
    GL::BufferBlockType m_bufferType = GL::BufferBlockType::kShaderStorage;
    GL::ShaderBlockLayout m_blockLayout = GL::ShaderBlockLayout::kStd140;
    int m_bufferBinding = -1; // the index of the buffer binding point associated with the active shader storage block
    unsigned long m_bufferDataSize = 0; // the minimum size in bytes to hold active variables 
    size_t m_numActiveVariables = 0; // the number of active variables associated with the block
    std::vector<ShaderInputInfo> m_fields; // Info about fields of buffer
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief struct containing a shader struct's name and fields
struct ShaderStruct {
    QString m_name;
    std::vector<ShaderInputInfo> m_fields;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Struct containing metadata about a shader function
struct ShaderFunction {

    QString m_name; // Name of the functiion
    ShaderVariableType m_returnType; // Return type of the function
    QString m_definition; // The entire definition of the function
    QString m_definitionStart; // The beginning of the function definition
    QString m_body; // The body of the function (contents of brackets)
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
///// @brief Struct containing metadata about a parameter going in or out of a shader
//struct ShaderIO {
//
//
//    QString m_name; // Name of the variable
//    ShaderVariableType m_type; // Type of the variable
//    ShaderIO::IOType m_inOrOut; // Whether input or output
//};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class representing an openGL shader
/// @detailed Note that QT's abstraction of the shader program is bizarre... "I can't tell what's worse: 
/// that they made setAttributeBuffer a member of a type that has absolutely nothing to do with vertex state"
/// See: https://stackoverflow.com/questions/37999609/combining-vertex-array-handler-with-vertex-buffer-index-buffer
/// @{ When writing shaders, use the precision rule:
/// - highp for vertex positions,
/// - mediump for texture coordinates,
/// - lowp for colors.
/// @}
class Shader : public Gb::Object, public Loadable{
public:
    //---------------------------------------------------------------------------------------
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

    static std::vector<std::pair<QString, ShaderType>> Builtins() { return s_builtins; }

    /// @brief Combine shaders into a single source file
    static QString CombineEffectFragmentShaders(std::vector<const Shader*>& shaders);
    static QString CombineEffectVertexShaders(std::vector<const Shader*>& shaders);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Typedefs
    /// @{
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /// @property className
    /// @brief The name of this class
    virtual const char* className() const { return "Shader"; }

    /// @property namespaceName
    /// @brief The full namespace for this class
    virtual const char* namespaceName() const { return "Gb::GL::Shader"; }
    /// @}

    //---------------------------------------------------------------------------------------//
    /// @name Constructors/Destructors
    /// @{
    Shader();
    Shader(const QJsonValue& json);
    Shader(ShaderType type);
    Shader(const QString& file, ShaderType type, bool deferConstruction=true);
    ~Shader();
    /// @{
    //---------------------------------------------------------------------------------------/
    /// @name Properties
    /// @{

    const ShaderType& type() const { return m_type; }

    unsigned int getID() const { return m_shaderID; }

    bool isValid() const { return m_isValid; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Initialize with source
    void initializeFromSource(const QString& source);

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

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Friends
    /// @{
    friend class ShaderProgram;

    /// @}

	//---------------------------------------------------------------------------------------
	/// @name Protected methods
	/// @{

    /// @brief load in a shader from a source file
    /// See: https://stackoverflow.com/questions/6047527/how-to-pass-an-stdstring-to-glshadersource
    /// https://wiki.qt.io/Using-QOpenGLFunctions-and-QOpenGLContext
    bool initializeFromSourceFile();

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

	//---------------------------------------------------------------------------------------
	/// @name Protected members
	/// @{

    /// @brief Whether or not the shader loaded successfully
    bool m_isValid = false;

    ShaderType m_type;

    int m_shaderID = -1;

    /// @brief For all OpenGL operations
    GL::OpenGLFunctions m_gl;

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


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
///// @struct ShaderProgramState
///// @brief Set of uniforms and buffers to set the state of a shader program
//struct ShaderProgramState {
//    std::vector<Uniform> m_uniforms;
//    std::vector<GLBuffer*> m_buffers;
//};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class representing an openGL shader program
/// @detailed Note that QT's abstraction of the shader program is bizarre... "I can't tell what's worse: 
/// that they made setAttributeBuffer a member of a type that has absolutely nothing to do with vertex state"
class ShaderProgram: 
    public Resource,
    public Serializable
    //std::enable_shared_from_this<ShaderProgram> 
{
public:
    //---------------------------------------------------------------------------------------//
    /// @name Static
    /// @{

    typedef tsl::robin_map<GStringView, ShaderInputInfo>::const_iterator UniformInfoIter;

    /// @brief Create a handle to a shader resource
    static std::shared_ptr<ResourceHandle> createHandle(CoreEngine* engine, const QJsonValue& json);

    static ShaderProgram* CurrentlyBound() { return s_boundShader; }

    /// @brief Whether or not a shader is built-in, given a shader name
    static bool isBuiltIn(const QString& name);

    /// @brief Whether or not the shader program uses any of the built-in buffers
    enum class BufferUsageFlag {
        kLightBuffer = 1 << 0,
        kShadowBuffer = 1 << 1
    };
    typedef QFlags<BufferUsageFlag> BufferUsageFlags;

    /// @}

    //---------------------------------------------------------------------------------------//
    /// @name Constructors/Destructors
    /// @{    
    ShaderProgram();
    ShaderProgram(const ShaderProgram& shaderProgram);
    ShaderProgram(const QJsonValue& json);
    ShaderProgram(const QString& vertfile, const QString& fragfile);
    ShaderProgram(const QString& name, const QString& vertSource, const QString& fragSource, double dummy);
    ShaderProgram(const QString& vertfile, const QString& fragfile, const QString& geometryFile);
    ShaderProgram(const QString& compFile);
    virtual ~ShaderProgram();
    /// @}


    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Get the type of resource stored by this handle
    virtual Resource::ResourceType getResourceType() const override {
        return Resource::kShaderProgram;
    }

    size_t getProgramID() const { return m_programID; }

    const std::vector<Uniform>& uniforms() const { return m_uniforms; }

    bool isValid() const;
    
    /// @brief Whether or not the shader is bound to a context
    inline bool isBound() const { 
        if (!s_boundShader) return false;
        return m_uuid == s_boundShader->getUuid(); 
    }

    /// @brief Return the fragment shader
    const Shader* getFragShader() const;

    /// @brief Return the vertex shader
    const Shader* getVertShader() const;

    /// @brief Return the geometry shader
    const Shader* getGeometryShader() const;

    /// @brief Return the compute shader
    const Shader* getComputeShader() const;

    /// @brief Return the shader of the given type
    const Shader* getShader(Shader::ShaderType type) const;


    /// @}

	//---------------------------------------------------------------------------------------
	/// @name Gb::Object Overrides
	/// @{
	/// @property className
	virtual const char* className() const { return "ShaderProgram"; }

	/// @property namespaceName
	virtual const char* namespaceName() const { return "Gb::ShaderProgram"; }
	/// @}


    //---------------------------------------------------------------------------------------//
    /// @name Public Methods
    /// @{   

    /// @brief Add buffer to the shader program
    //void addBuffer(GLBuffer* buffer, const QString& bufferName = QString());

    /// @brief For compute shader only, dispatch compute
    void dispatchCompute(size_t numGroupsX, size_t numGroupsY, size_t numGroupsZ);

    /// @brief Get max work group counts in x, y, z dimensions
    Vector3i getMaxWorkGroupCounts();

    /// @brief Whether the shader has the buffer with the given name and type
    bool hasBufferInfo(const QString& name, GL::BufferBlockType type, int* outIndex = nullptr);

    /// @brief updates uniforms that are present in the queue
    void updateUniforms(bool ignoreMismatch = false);

    /// @brief Copy uniforms from specified shader program to this one
    //void setUniforms(const ShaderProgram& program);

    /// @brief Set the name of the shader program given it's subshaders
    void setName();

    /// @brief Whether or not the given string is a valid uniform in the shader
    bool hasUniform(const GStringView& uniformName) const;
    bool hasUniform(const GStringView& uniformName, UniformInfoIter& outIter, int* localIndex = nullptr) const;
    //bool hasCachedUniform(const QString& uniformName, std::vector<Uniform>::const_iterator& iter) const;

    /// @brief Get the the given uniform
    inline const Uniform* getUniformValue(const GStringView& uniformName) const {
        int localIndex = -1;
        UniformInfoIter infoIter;
        if (!hasUniform(uniformName, infoIter, &localIndex)) {
            //std::vector<Uniform>::const_iterator iter;
            //if (!hasCachedUniform(uniformName, iter)) {
                return nullptr;
            //}
            //else {
            //    return &(*iter);
            //}
        }
        return &m_uniforms[localIndex];
    }
    /// @brief starts the program
    /// @detailed upon binding, any uniforms in the queue will be updated
    void bind();

    /// @brief releases the program
    void release();

    /// @brief Adds uniform value to be set on next updateUniforms() call, given the string
    /// @detailed Valid for any type that can be be converted to a Uniform
    template<class VariantType> 
    inline void setUniformValue(const GStringView& uniformName, const VariantType& value, bool updateGL = false) {
        m_uniformQueue[m_newUniformCount++] = { uniformName, value };
        //Vec::EmplaceBack(m_uniformQueue, uniformName, value);
#ifdef DEBUG_MODE
        if (m_uniformQueue[m_newUniformCount - 1].getName().isEmpty()) {
            throw("Error, empty uniform name");
        }
#endif
        if (updateGL) updateUniforms();
    }
    void setUniformValue(const Uniform& uniform, bool updateGL = false);

    /// @brief Sets the value of the given uniform in GL, given the uniform ID
    // TODO: Implement

    /// @brief Override resource's post-construction routine
    virtual void postConstruction() override;

    /// @brief What action to perform on removal of the resource
    virtual void onRemoval(ResourceCache* cache = nullptr) override {
        Q_UNUSED(cache)
    }

    void clearUniforms();


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Friends
    /// @{
    friend class Renderer;
    friend class SceneObject;
    friend class Model;
    friend class Material;
    friend class CubeMap;
    friend class UBO;

    /// @}
	//---------------------------------------------------------------------------------------
	/// @name Protected methods
	/// @{

    /// @brief Obtain the ID of the uniform corresponding to the given string
    /// See: https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glUniform.xml
    inline GLuint getUniformID(const GStringView& uniformName) {
        return m_uniformInfo.at(uniformName).m_uniformID;
    }

    /// @brief Obtain the ID of the uniform corresponding to the given string
    /// See: https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glUniform.xml
    inline GLuint getUniformIDGL(const GStringView& uniformName) {
        return m_gl.glGetUniformLocation(m_programID, uniformName.c_str());
    }

    /// @brief Sets the value of the given uniform in GL, given the string
    template<typename T>
    inline void setUniformValueGL(const GStringView& uniformName, const T& value) = delete;

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const size_t& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniform1ui(uniformID, value);
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const int& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniform1i(uniformID, value);
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const float& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniform1f(uniformID, value);
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const Vector2f& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniform2fv(uniformID, 1, value.data());
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const Vector3f& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniform3fv(uniformID, 1, value.data());
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const Vector4f& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniform4fv(uniformID, 1, value.data());
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const Matrix2x2& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniformMatrix2fv(uniformID, 1, GL_FALSE, value.m_mtx[0].data()); // uniform ID, count, transpose, value
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const Matrix3x3& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniformMatrix3fv(uniformID, 1, GL_FALSE, value.m_mtx[0].data()); // uniform ID, count, transpose, value
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const Matrix4x4& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniformMatrix4fv(uniformID, 1, GL_FALSE, value.m_mtx[0].data()); // uniform ID, count, transpose, value
    }

    template<>
    void setUniformValueGL(const GStringView& uniformName, const std::vector<Matrix4x4>& value);

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const std::vector<float>& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniform1fv(uniformID, value.size(), value.data());
    }

    template<>
    void setUniformValueGL(const GStringView& uniformName, const Vec3List& value);

    template<>
    void setUniformValueGL(const GStringView& uniformName, const Vec4List& value);


    /// @brief links inputs to the shader program to the attributes of a VAO
    virtual void bindAttributes() {}

    /// @brief links an input to the shader program to the attributes of a VAO
    void bindAttribute(int attribute, const QString& variableName);

    /// @brief initializes shader program from the given shaders
    bool initializeShaderProgram();

    /// @brief attach the given shader to the shader program
    void attachShader(const Shader& shader);

    /// @brief Detach shader from the shader program (can do once link is done)
    void detachShader(const Shader& shader);

    /// @brief Parses shader code to obtain valid map of uniforms
    void populateUniforms();

    /// @brief Parses shader code to obtain valid map of uniform buffer objects
    void populateUniformBuffers();

    /// @brief Get the uniform block index of the specified UBO
    /// @details See: https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL
    /// @note It is assumed that uniform block names are consistent across all shaders and UBOs
    size_t getUniformBlockIndex(const QString& blockName);

    /// @brief Bind the specified shader uniform block to the specified binding point
    /// @details See: https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL
    void bindUniformBlock(const QString& blockName);

    /// @brief Get uniform info
    // See: https://stackoverflow.com/questions/440144/in-opengl-is-there-a-way-to-get-a-list-of-all-uniforms-attribs-used-by-a-shade
    void getActiveUniforms(std::vector<ShaderInputInfo>& outInfo);

    /// @brief Get active shader storage blocks
    // See: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgramResource.xhtml
    void getActiveSSBs(std::vector<ShaderBufferInfo>& outInfo);

    /// @brief Get attribute info
    // See: https://stackoverflow.com/questions/440144/in-opengl-is-there-a-way-to-get-a-list-of-all-uniforms-attribs-used-by-a-shade
    void getActiveAttributes(std::vector<ShaderInputInfo>& outInfo);

    /// @brief Get number of active uniforms
    GLint getNumActiveUniforms();

    /// @brief Get number of active buffers of the specified type
    GLint getNumActiveBuffers(GL::BufferBlockType type);

    /// @brief Get number of active attributes
    GLint getNumActiveAttributes();

	/// @}

	//---------------------------------------------------------------------------------------
	/// @name Protected members
	/// @{

	/// @brief GL ID of the shader program
	unsigned int m_programID;

    /// @brief Number of uniforms in queue to actually use for update
    unsigned int m_newUniformCount = 0;

    /// @brief Lock data for multithreading
    QMutex m_mutex;

    BufferUsageFlags m_bufferUsageFlags;

    /// @brief For all OpenGL operations
    GL::OpenGLFunctions m_gl;

    /// @brief Map of all shaders associated with the shader program
    /// @details Typically is just a vertex and a fragment shader
    std::vector<Shader> m_shaders;

	/// @brief Queue for uniforms that need to be updated in GL
	std::array<Uniform, UNIFORM_CACHE_SIZE> m_uniformQueue;

    /// @brief Vector of uniforms names and their values
    std::vector<Uniform> m_uniforms;

    /// @brief Map of uniform names with corresponding GLSL info
    tsl::robin_map<GStringView, ShaderInputInfo> m_uniformInfo;
    std::vector<ShaderInputInfo> m_uniformInfoVec;

    /// @brief Vector of buffer info
    std::vector<ShaderBufferInfo> m_bufferInfo;

    /// @brief Buffers associated with the shader
    /// @details Currently unused, no custom buffer capability
    //std::vector<GLBuffer*> m_buffers;

    /// @brief Map of uniform buffers associated with this shader program
    // See: https://stackoverflow.com/questions/34597260/stdhash-value-on-char-value-and-not-on-memory-address
    tsl::robin_map<QString, std::shared_ptr<UBO>> m_uniformBuffers;

    /// @brief The currently bound shader program
    static ShaderProgram* s_boundShader;

	/// @}
};
typedef std::shared_ptr<ShaderProgram> ShaderProgramPtr;
Q_DECLARE_METATYPE(ShaderProgramPtr)



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif