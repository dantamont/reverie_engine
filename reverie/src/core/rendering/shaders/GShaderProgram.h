// See:
// https://www.trentreed.net/blog/qt5-opengl-part-1-basic-rendering/

#ifndef GB_SHADER_PROGRAM_H
#define GB_SHADER_PROGRAM_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard

// QT

// Internal
#include "GShader.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////////////////////
#define UNIFORM_CACHE_SIZE 128

namespace rev { 

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
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
    static std::shared_ptr<ResourceHandle> CreateHandle(CoreEngine* engine, const QJsonValue& json, const GString& shaderName);

    static ShaderProgram* CurrentlyBound() { return s_boundShader; }

    /// @brief Whether or not a shader is built-in, given a shader name
    static bool isBuiltIn(const GString& name);

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
    //ShaderProgram(const QJsonValue& json);
    ShaderProgram(const QString& vertfile, const QString& fragfile);
    ShaderProgram(const QString& vertSource, const QString& fragSource, double dummy);
    ShaderProgram(const QString& vertfile, const QString& fragfile, const QString& geometryFile);
    ShaderProgram(const QString& compFile);
    virtual ~ShaderProgram();
    /// @}


    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Get the type of resource stored by this handle
    virtual ResourceType getResourceType() const override {
        return ResourceType::kShaderProgram;
    }

    size_t getProgramID() const { return m_programID; }

    const std::vector<Uniform>& uniforms() const { return m_uniforms; }

    bool isValid() const;
    
    /// @brief Whether or not the shader is bound to a context
    bool isBound() const;

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
	/// @name rev::Object Overrides
	/// @{
	/// @property className
	virtual const char* className() const { return "ShaderProgram"; }

	/// @property namespaceName
	virtual const char* namespaceName() const { return "rev::ShaderProgram"; }
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

    /// @brief Generate the name of the shader program given it's subshaders
    GString createName() const;

    /// @brief Whether or not the given string is a valid uniform in the shader
    bool hasUniform(const GStringView& uniformName) const;
    bool hasUniform(const GStringView& uniformName, UniformInfoIter& outIter, int* localIndex = nullptr) const;
    //bool hasCachedUniform(const QString& uniformName, std::vector<Uniform>::const_iterator& iter) const;

    /// @brief Get the the given uniform
    // TODO: No longer caching uniforms, remove m_uniforms
    //inline const Uniform* getUniformValue(const GStringView& uniformName) const {
    //    int localIndex = -1;
    //    UniformInfoIter infoIter;
    //    if (!hasUniform(uniformName, infoIter, &localIndex)) {
    //        //std::vector<Uniform>::const_iterator iter;
    //        //if (!hasCachedUniform(uniformName, iter)) {
    //            return nullptr;
    //        //}
    //        //else {
    //        //    return &(*iter);
    //        //}
    //    }
    //    return &m_uniforms[localIndex];
    //}

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
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

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
    friend class LoadProcess;
    friend class PostProcessingEffect;

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
        GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
        return gl.glGetUniformLocation(m_programID, uniformName.c_str());
    }

    /// @brief Sets the value of the given uniform in GL, given the string
    template<typename T>
    inline void setUniformValueGL(const GStringView& uniformName, const T& value) = delete;

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const unsigned int& value) {
        GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
        GLuint uniformID = getUniformID(uniformName);
        gl.glUniform1ui(uniformID, value);
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const int& value) {
        GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
        GLuint uniformID = getUniformID(uniformName);
        gl.glUniform1i(uniformID, value);
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const float& value) {
        GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
        GLuint uniformID = getUniformID(uniformName);
        gl.glUniform1f(uniformID, value);
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const Vector2f& value) {
        GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
        GLuint uniformID = getUniformID(uniformName);
        gl.glUniform2fv(uniformID, 1, value.data());
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const Vector3f& value) {
        GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
        GLuint uniformID = getUniformID(uniformName);
        gl.glUniform3fv(uniformID, 1, value.data());
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const Vector4f& value) {
        GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
        GLuint uniformID = getUniformID(uniformName);
        gl.glUniform4fv(uniformID, 1, value.data());
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const Matrix2x2& value) {
        GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
        GLuint uniformID = getUniformID(uniformName);
        gl.glUniformMatrix2fv(uniformID, 1, GL_FALSE, value.m_mtx[0].data()); // uniform ID, count, transpose, value
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const Matrix3x3& value) {
        GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
        GLuint uniformID = getUniformID(uniformName);
        gl.glUniformMatrix3fv(uniformID, 1, GL_FALSE, value.m_mtx[0].data()); // uniform ID, count, transpose, value
    }

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const Matrix4x4& value) {
        GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
        GLuint uniformID = getUniformID(uniformName);
        gl.glUniformMatrix4fv(uniformID, 1, GL_FALSE, value.m_mtx[0].data()); // uniform ID, count, transpose, value
    }

    template<>
    void setUniformValueGL(const GStringView& uniformName, const std::vector<Matrix4x4>& value);

    template<>
    inline void setUniformValueGL(const GStringView& uniformName, const std::vector<float>& value) {
        GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
        GLuint uniformID = getUniformID(uniformName);
        gl.glUniform1fv(uniformID, (GLsizei)value.size(), value.data());
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

    /// @brief Map of all shaders associated with the shader program
    /// @details Typically is just a vertex and a fragment shader
    std::vector<Shader> m_shaders;

	/// @brief Queue for uniforms that need to be updated in GL
	std::array<Uniform, UNIFORM_CACHE_SIZE> m_uniformQueue;

    /// @brief Vector of uniforms names and their values
    // TODO: Remove this, pretty sure it's unused now
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


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif