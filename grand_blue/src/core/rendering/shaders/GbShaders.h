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
#include "GbUniform.h"
#include "../../containers/GbContainerExtensions.h"

namespace Gb { 

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class SceneObject;
class ShaderProgram;
class CubeMap;
class Model;
class UBO;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
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
class Shader : public Gb::Object, public Serializable{
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Typedefs
    /// @{
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /** @property className
        @brief The name of this class
    */
    virtual const char* className() const { return "Shader"; }

    /** @property namespaceName
        @brief The full namespace for this class
    */
    virtual const char* namespaceName() const { return "Gb::GL::Shader"; }
    /// @}

    //---------------------------------------------------------------------------------------/
    /// @name Statics
    /// @{
    enum ShaderType
    {
        kVertex = 0x0001,
        kFragment = 0x0002,
        kGeometry = 0x0004, //  (requires OpenGL >= 4.0 or OpenGL ES >= 3.2).
        kTessellationControl = 0x0008, //  (requires OpenGL >= 4.0 or OpenGL ES >= 3.2).
        kTessellationEvaluation = 0x0010, //  (requires OpenGL >= 4.0 or OpenGL ES >= 3.2).
        kCompute = 0x0020 // (requires OpenGL >= 4.3 or OpenGL ES >= 3.1).
    };

    /** @brief Obtain map of ShaderType to corresponding GL type
        @param[in]
        @return s_shaderTypeToGLMap
    */
    static std::unordered_map<ShaderType, int>& shaderTypeToGLMap() { return SHADER_TYPE_TO_GL_MAP; }

    /// @}
    //---------------------------------------------------------------------------------------//
    /// @name Constructors/Destructors
    /// @{
    Shader(){}
    Shader(const QJsonValue& json);
    Shader(const QString& file, ShaderType type);
    ~Shader();
    /// @{
    //---------------------------------------------------------------------------------------/
    /// @name Properties
    /// @{
    unsigned int getID() const { return m_shaderID; }

    const QString& getPath() const { return m_filePath; }

    bool isValid() const { return m_isValid; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

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
    bool m_isValid;

    ShaderType m_type;

    unsigned int m_shaderID;

    QString m_filePath;

    /// @brief For all OpenGL operations
    GL::OpenGLFunctions m_gl;

    /// @brief The structs in the shader
    std::vector<ShaderStruct> m_structs;

    /// @brief the uniform buffers in the shader
    std::vector<ShaderStruct> m_uniformBuffers;

    /// @brief The uniforms in the shader
    std::vector<UniformInfo> m_uniforms;

    /// @brief Define values (integers needed for array sizes)
    std::unordered_map<QString, int> m_defines;

    /// @brief Source code of the shader 
    /// @detailed Stored as a standard string to avoid dangling pointer in c_str() call
    std::string m_source;

    static std::unordered_map<ShaderType, int> SHADER_TYPE_TO_GL_MAP;
    static std::unordered_map<ShaderType, QString> SHADER_TYPE_TO_STRING_MAP;

    // Regex for a uniform:      uniform[\s\t\n\r]*[\w\d]+[\s\t\n\r]*[\w\d]+
    // Regex for a layout block: layout[\s\t\n\r]*\([\w\d]+\)[\s\t\n\r]*uniform[\s\t\n\r]+[\w\d]+[\s\t\n\r]*[\s\t\n\r]*\{([\w\d\s\t\n\r;\/\,]*)\}
    // Regex for a struct:       struct[\s\t\n\r]*[\w\d]+[\s\t\n\r]*\{([\w\s\d\t\n\r;\/\,]*)\}
    static QString UNIFORM_REGEX;
    static QString STRUCT_REGEX;
    static QString BUFFER_REGEX;
    static QString COMMENT_REGEX;
    static QString DEFINE_REGEX;

	/// @{
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Shader program type (based on source files)
enum ShaderProgramType {
    kGeneric = -1,
    kBasic,
    kCubemap
};

/// @brief Class representing an openGL shader program
/// @detailed Note that QT's abstraction of the shader program is bizarre... "I can't tell what's worse: 
/// that they made setAttributeBuffer a member of a type that has absolutely nothing to do with vertex state"
class ShaderProgram: 
    public Object,
    public Serializable
    //std::enable_shared_from_this<ShaderProgram> 
{
public:

    //---------------------------------------------------------------------------------------//
    /// @name Static
    /// @{

    /// @brief Create from JSON
    static std::shared_ptr<ShaderProgram> create(const QJsonValue& json);

    /// @}

    //---------------------------------------------------------------------------------------//
    /// @name Constructors/Destructors
    /// @{    
    ShaderProgram();
    ShaderProgram(const ShaderProgram& shaderProgram);
    ShaderProgram(const QJsonValue& json);
    ShaderProgram(const QString& vertfile, const QString& fragfile, ShaderProgramType type=kGeneric);
    virtual ~ShaderProgram();
    /// @}


    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    bool isValid() const;
    
    /// @brief Whether or not the shader is bound to a context
    inline bool isBound() const { 
        if (!BOUND_SHADER) return false;
        return m_uuid == BOUND_SHADER->getUuid(); 
    }

    /// @brief Return type of shader
    inline ShaderProgramType programType() { return m_programType; }

    /// @brief Return the fragment shader
    const Shader* getFragShader() const;

    /// @brief Return the vertex shader
    const Shader* getVertShader() const;

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

    /// @brief updates uniforms that are present in the queue
    bool updateUniforms();

    /// @brief Set the name of the shader program given it's subshaders
    void setName();

    /// @brief Whether or not the given string is a valid uniform in the shader
    bool hasUniform(const QString& uniformName) const;
    bool hasCachedUniform(const QString& uniformName, std::vector<Uniform>::const_iterator& iter) const;

    /// @brief Get the the given uniform
    inline const Uniform* getUniformValue(const QString& uniformName) const {
        if (!hasUniform(uniformName)) {
            std::vector<Uniform>::const_iterator iter;
            if (!hasCachedUniform(uniformName, iter)) {
                return nullptr;
            }
            else {
                return &(*iter);
            }
        }
        return &m_uniforms.at(uniformName);
    }
    /// @brief starts the program
    /// @detailed upon binding, any uniforms in the queue will be updated
    void bind();

    /// @brief releases the program
    void release();

    /// @brief Adds uniform value to be set on next updateUniforms() call, given the string
    /// @detailed Valid for any type that can be be converted to a Uniform
    template<class VariantType> 
    inline void setUniformValue(const QString& uniformName, const VariantType& value, bool updateGL = false) {
        Vec::EmplaceBack(m_uniformQueue, uniformName, value);
        if (updateGL) updateUniforms();
    }
    inline void setUniformValue(const Uniform& uniform, bool updateGL = false) {
        Vec::EmplaceBack(m_uniformQueue, uniform);
        if (updateGL) updateUniforms();
    }

    /// @brief Sets the value of the given uniform in GL, given the uniform ID
    // TODO: Implement

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

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
    inline GLuint getUniformID(const QString& uniformName) {
        int id;
        if (m_uniformInfo.find(uniformName) != m_uniformInfo.end()) {
            // If uniform name found in uniform info (will be found unless is an array uniform)
            id = m_uniformInfo.at(uniformName).m_uniformID;
            if (id < 0) {
                // If id not assigned, assign
                id = m_gl.glGetUniformLocation(m_programID, uniformName.toStdString().c_str());
                if (id > 0) {
                    m_uniformInfo.at(uniformName).m_uniformID = id;
                }
            }
        }
        else {
            id = m_gl.glGetUniformLocation(m_programID, uniformName.toStdString().c_str());
            m_uniformInfo[uniformName] = {uniformName, "", false, id};
        }
        return id;
    }

    /// @brief Sets the value of the given uniform in GL, given the string
    template<typename T>
    inline void setUniformValueGL(const QString& uniformName, const T& value) = delete;

    template<>
    inline void setUniformValueGL(const QString& uniformName, const int& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniform1i(uniformID, value);
    }

    template<>
    inline void setUniformValueGL(const QString& uniformName, const float& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniform1f(uniformID, value);
    }

    template<>
    inline void setUniformValueGL(const QString& uniformName, const Vector2f& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniform2fv(uniformID, 1, value.data());
    }

    template<>
    inline void setUniformValueGL(const QString& uniformName, const Vector3f& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniform3fv(uniformID, 1, value.data());
    }

    template<>
    inline void setUniformValueGL(const QString& uniformName, const Vector4f& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniform4fv(uniformID, 1, value.data());
    }

    template<>
    inline void setUniformValueGL(const QString& uniformName, const Matrix2x2f& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniformMatrix2fv(uniformID, 1, GL_FALSE, value.m_mtx[0].data()); // uniform ID, count, transpose, value
    }

    template<>
    inline void setUniformValueGL(const QString& uniformName, const Matrix3x3f& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniformMatrix3fv(uniformID, 1, GL_FALSE, value.m_mtx[0].data()); // uniform ID, count, transpose, value
    }

    template<>
    inline void setUniformValueGL(const QString& uniformName, const Matrix4x4f& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniformMatrix4fv(uniformID, 1, GL_FALSE, value.m_mtx[0].data()); // uniform ID, count, transpose, value
    }

    template<>
    void setUniformValueGL(const QString& uniformName, const std::vector<Matrix4x4f>& value);

    template<>
    inline void setUniformValueGL(const QString& uniformName, const std::vector<float>& value) {
        GLuint uniformID = getUniformID(uniformName);
        m_gl.glUniform1fv(uniformID, value.size(), value.data());
    }

    template<>
    void setUniformValueGL(const QString& uniformName, const Vec3List& value);

    template<>
    void setUniformValueGL(const QString& uniformName, const Vec4List& value);


    /// @brief links inputs to the shader program to the attributes of a VAO
    virtual void bindAttributes() {}

    /// @brief links an input to the shader program to the attributes of a VAO
    void bindAttribute(int attribute, const QString& variableName);

    /// @brief initializes shader program from the given shaders
    bool initializeShaderProgram();

    /// @brief attach the given shader to the shader program
    void attachShader(const Shader& shader);

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

	/// @}

	//---------------------------------------------------------------------------------------
	/// @name Protected members
	/// @{

	/// @brief GL ID of the shader program
	unsigned int m_programID;

    /// @brief Lock data for multithreading
    QMutex m_mutex;

    /// @brief Type of shader program
    ShaderProgramType m_programType;

    /// @brief For all OpenGL operations
    GL::OpenGLFunctions m_gl;

    /// @brief Map of all shaders associated with the shader program
    /// @details Typically is just a vertex and a fragment shader
    std::unordered_map<Shader::ShaderType, Shader> m_shaders;

	/// @brief Queue for uniforms that need to be updated in GL
	std::vector<Uniform> m_uniformQueue;

    /// @brief Map of uniforms names and their values
    std::unordered_map<QString, Uniform> m_uniforms;

    /// @brief Map of uniforms names with corresponding GLSL info
    std::unordered_map<QString, UniformInfo> m_uniformInfo;

    /// @brief Map of uniform buffers associated with this shader program
    std::unordered_map<QString, std::shared_ptr<UBO>> m_uniformBuffers;

    /// @brief The currently bound shader program
    static ShaderProgram* BOUND_SHADER;

	/// @}
};
typedef std::shared_ptr<ShaderProgram> ShaderProgramPtr;
Q_DECLARE_METATYPE(ShaderProgramPtr)

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief BasicShaderProgram
/// @brief Class for a basic vertex-coloring shader program
class BasicShaderProgram : public ShaderProgram {
public:
	//---------------------------------------------------------------------------------------//
	/// @name Static
	/// @{
	/// @brief Shader program mode (based on uniforms)
	enum BasicShaderMode {
		kSimple = 0,
		kColor,
		kTextureColor,
        kNormal,
        kTextureNormal
	};

	/// @}

	//---------------------------------------------------------------------------------------//
	/// @name Constructors/Destructors
	/// @{    
    BasicShaderProgram();
    BasicShaderProgram(const QJsonValue& json);
    ~BasicShaderProgram();
	/// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property className
    /// @brief The name of this class
    virtual const char* className() const { return "BasicShaderProgram"; }

    /// @property namespaceName
    /// @brief The full namespace for this class
    virtual const char* namespaceName() const { return "Gb::GL::BasicShaderProgram"; }
    /// @}

	//---------------------------------------------------------------------------------------//
	/// @name Public Methods
	/// @{   
	/// @}

protected:
};
Q_DECLARE_METATYPE(BasicShaderProgram)

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Class for shading cubemaps
class CubemapShaderProgram : public ShaderProgram {
public:
    //---------------------------------------------------------------------------------------//
    /// @name Static
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------//
    /// @name Constructors/Destructors
    /// @{    
    CubemapShaderProgram();
    CubemapShaderProgram(const QJsonValue& json);
    ~CubemapShaderProgram();
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /** @property className
        @brief The name of this class
    */
    virtual const char* className() const { return "CubemapShaderProgram"; }

    /** @property namespaceName
        @brief The full namespace for this class
    */
    virtual const char* namespaceName() const { return "Gb::CubemapShaderProgram"; }
    /// @}

    //---------------------------------------------------------------------------------------//
    /// @name Public Methods
    /// @{   

    /// @}
protected:

};
Q_DECLARE_METATYPE(CubemapShaderProgram)


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif