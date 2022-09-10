#pragma once

// Internal
#include "GShader.h"
#include "core/rendering/materials/GTexture.h"

#include "fortress/layer/framework/GFlags.h"

namespace rev { 

class UniformContainer;

/// @brief Class representing an openGL shader program
/// @detailed Note that QT's abstraction of the shader program is bizarre... "I can't tell what's worse: 
/// that they made setAttributeBuffer a member of a type that has absolutely nothing to do with vertex state"
class ShaderProgram: public Resource
{
public:

    /// @brief Struct containing the uniform IDs of built-in uniforms
    /// @todo Associate each uniform type with a unique ID, so these don't need to be so hard-coded. 
    struct UniformIdMappings {

        /// @brief Populate the uniform ID mappings from the given shader program
        void populateMappings(const ShaderProgram& sp);

        Int32_t m_worldMatrix{ -1 }; ///< The world matrix of the current renderable
        Int32_t m_isAnimated{ -1 }; ///< Whether or not the model being rendered is (skeletally) animated
        Int32_t m_globalInverseTransform{ -1 }; ///< The inverse of the top-level skeleton transform
        Int32_t m_inverseBindPoseTransforms{ -1 }; ///< The inverse bind pose (undoes T-Pose) of the animation
        Int32_t m_boneTransforms{ -1 }; ///< The pose of the skeletal animation, if animated

        Int32_t m_constantScreenThickness{ -1 }; ///< For lines only. Whether or not a line should have constant screen thickness
        Int32_t m_lineColor{ -1 }; ///< For lines only. The color of the line to render
        Int32_t m_lineThickness{ -1 }; ///< For lines only. The thickness of the line to render
        Int32_t m_useMiter{ -1 }; ///< For lines only. Whether or not to use a miter join
        Int32_t m_fadeWithDistance{ -1 }; ///< For lines only. Whether or not to fade with distance from the camera

        // Points
        Int32_t m_color{ -1 }; ///< For points only. The color of the points
        Int32_t m_pointSize{ -1 }; ///< For points only. The size of the points
        Int32_t m_screenPixelWidth{ -1 }; ///< The screen pixel width

        // Textures
        Int32_t m_textColor{ -1 }; ///< The text color
        std::array<Int32_t, (size_t)TextureUsageType::kMAX_TEXTURE_TYPE> m_textureUniforms; ///< Array of supported texture types and their associated uniform IDs for this shader program

        Int32_t m_materialSpecularity{ -1 }; ///< Material specularity
        Int32_t m_materialShininess{ -1 }; ///< Material shininess

        // Glyphs
        Int32_t m_perspectiveInverseScale{ -1 }; ///< Inverse of perspective scale matrix
        Int32_t m_scaleWithDistance{ -1 }; ///< Whether or not to scale billboard with distance
        Int32_t m_faceCamera{ -1 }; ///< Whether or not to lock the billboard to face the camera
        Int32_t m_onTop{ -1 }; ///< Whether or not to always render the canvas on top of other renderables
        Int32_t m_texOffset{ -1 }; ///< The offset of the canvas texture
        Int32_t m_texScale{ -1 }; ///< The scale of the canvas texture

        /// @todo Move to just activeClusterShader
        Int32_t m_depthTexture{ -1 }; ///< Depth texture used for light culling

        /// @todo Move to cubemap shader
        Int32_t m_cubeTexture{ -1 }; ///< Cube texture for cubemap shader
        Int32_t m_diffuseColor{ -1 }; ///< Diffuse color for the cubemap

        /// @todo Move to ssaoShader
        Int32_t m_noiseSize{ -1 }; ///< The size of the noise
        Int32_t m_offsets{ -1 }; ///< The offsets to use for the SSAO, also used for quad shader
        Int32_t m_scale{ -1 }; ///< The scale to use for the SSAO shader, also used for quad shader
        Int32_t m_kernelSize{ -1 }; ///< The kernel size
        Int32_t m_bias{ -1 }; ///< The bias for the SSAO shader
        Int32_t m_radius{ -1 }; ///< The radius of the SSAO effect

        // @todo For point light camera only
        Int32_t m_pointLightIndex{ -1 }; ///< The index of the point light

        // Internal
        Int32_t m_colorIdUniform{ -1 }; ///< Uniform setting color ID in a prepass
    };

    /// @name Static
    /// @{

    typedef tsl::robin_map<GStringView, ShaderInputInfo>::const_iterator UniformInfoIter;

    /// @brief Create a handle to a shader resource
    static std::shared_ptr<ResourceHandle> CreateHandle(CoreEngine* engine, const nlohmann::json& json, const GString& shaderName);

    static ShaderProgram* CurrentlyBound() { return s_boundShader; }

    /// @brief Whether or not a shader is built-in, given a shader name
    static bool isBuiltIn(const GString& name);

    /// @brief Whether or not the shader program uses any of the built-in buffers
    enum class BufferUsageFlag {
        kLightBuffer = 1 << 0,
        kShadowBuffer = 1 << 1
    };
    MAKE_FLAGS(BufferUsageFlag, BufferUsageFlags);

    /// @}

    /// @name Constructors/Destructors
    /// @{    
    ShaderProgram();
    ShaderProgram(const ShaderProgram& shaderProgram);
    //ShaderProgram(const nlohmann::json& json);
    ShaderProgram(const QString& vertfile, const QString& fragfile);
    ShaderProgram(const QString& vertSource, const QString& fragSource, double dummy);
    ShaderProgram(const QString& vertfile, const QString& fragfile, const QString& geometryFile);
    ShaderProgram(const QString& compFile);
    virtual ~ShaderProgram();
    /// @}


    /// @name Public Methods
    /// @{

    /// @brief Get the name of the uniform given its ID
    const GString& getUniformName(Uint32_t id) const;

    /// @brief Retrieve the info about the specified uniform 
    const ShaderInputInfo& getUniformInfo(Uint32_t id) const;

    /// @brief Obtain the ID of the uniform corresponding to the given string
    /// @see https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glUniform.xml
    GLuint getUniformId(const GStringView& uniformName) const;

    /// @brief Get the type of resource stored by this handle
    virtual GResourceType getResourceType() const override {
        return EResourceType::eShaderProgram;
    }

    size_t getProgramID() const { return m_programID; }

    /// @brief Obtain IDs for built-in shader uniforms
    const UniformIdMappings& uniformMappings() const {
        return m_uniformMappings;
    }

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

    /// @brief Add buffer to the shader program
    //void addBuffer(GlBuffer* buffer, const QString& bufferName = QString());

    /// @brief For compute shader only, dispatch compute
    void dispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ);

    /// @brief Get max work group counts in x, y, z dimensions
    Vector3i getMaxWorkGroupCounts();

    /// @brief Whether the shader has the buffer with the given name and type
    bool hasBufferInfo(const QString& name, gl::BufferBlockType type, int* outIndex = nullptr);

    /// @brief updates uniforms that are present in the queue
    void updateUniforms(const UniformContainer& uniformContainer, bool ignoreMismatch = false);

    /// @brief Copy uniforms from specified shader program to this one
    //void setUniforms(const ShaderProgram& program);

    /// @brief Generate the name of the shader program given it's subshaders
    GString createName() const;

    /// @brief Whether or not the given string is a valid uniform in the shader
    bool hasUniform(Uint32_t uniformId) const;
    bool hasUniform(const GStringView& uniformName) const;
    bool hasUniform(const GStringView& uniformName, UniformInfoIter& outIter, int* localIndex = nullptr) const;

    /// @brief Whether or not the given texture type has a valid uniform
    bool hasTextureUniform(Uint32_t textureType, Uint32_t& outId) const;

    /// @brief starts the program
    /// @detailed upon binding, any uniforms in the queue will be updated
    void bind() const;

    /// @brief releases the program
    void release() const;

    /// @brief Queue the uniform to update the shader.
    /// @note This function assumes that the uniform already has an associated value in a uniform contianer
    void setUniformValue(const Uniform& uniform);
    void setUniformValue(Uint32_t uniformId, const UniformData& uniformData);

    /// @brief Override resource's post-construction routine
    virtual void postConstruction(const ResourcePostConstructionData& postConstructData) override;

    /// @brief What action to perform on removal of the resource
    virtual void onRemoval(ResourceCache* /*cache*/ = nullptr) override;

    void clearUniforms();


    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const ShaderProgram& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, ShaderProgram& orObject);


    /// @}

protected:
    /// @name Friends
    /// @{
    friend class Renderer;
    friend class SceneObject;
    friend class Model;
    friend class Material;
    friend class CubeMap;
    friend class Ubo;
    friend class LoadProcess;
    friend class PostProcessingEffect;

    /// @}
	
	/// @name Protected methods
	/// @{

    /// @brief Obtain the ID of the uniform corresponding to the given string
    /// @see https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glUniform.xml
    inline GLuint getUniformIdGl(const GStringView& uniformName);

    /// @brief Sets the value of the given uniform in GL, given the string
    template<typename T>
    inline void setUniformValueGl(const GStringView& uniformName, const T& value) = delete;

    template<typename T>
    inline void setUniformValueGl(Uint32_t uniformId, const T& value) = delete;

    template<typename T>
    inline void setUniformValueGl(Uint32_t uniformId, const T* value, size_t valueCount) = delete;

    template<>
    inline void setUniformValueGl(const GStringView& uniformName, const unsigned int& value) {
        GLuint uniformId = getUniformId(uniformName);
        setUniformValueGl(uniformId, value);
    }

    template<>
    inline void setUniformValueGl(Uint32_t uniformId, const unsigned int& value) {
        static gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
        gl.glUniform1ui(uniformId, value);
    }

    template<>
    inline void setUniformValueGl(const GStringView& uniformName, const int& value) {
        GLuint uniformId = getUniformId(uniformName);
        setUniformValueGl(uniformId, value);
    }

    template<>
    inline void setUniformValueGl(Uint32_t uniformId, const int& value) {
        static gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
        gl.glUniform1i(uniformId, value);
    }

    template<>
    inline void setUniformValueGl(const GStringView& uniformName, const float& value) {
        GLuint uniformId = getUniformId(uniformName);
        setUniformValueGl(uniformId, value);
    }

    template<>
    inline void setUniformValueGl(Uint32_t uniformId, const float& value) {
        static gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
        gl.glUniform1f(uniformId, value);
    }

    template<>
    inline void setUniformValueGl(const GStringView& uniformName, const Vector2f& value) {
        GLuint uniformId = getUniformId(uniformName);
        setUniformValueGl(uniformId, value);
    }

    template<>
    inline void setUniformValueGl(Uint32_t uniformId, const Vector2f& value) {
        static gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
        gl.glUniform2fv(uniformId, 1, value.data());
    }

    template<>
    inline void setUniformValueGl(const GStringView& uniformName, const Vector3f& value) {
        GLuint uniformId = getUniformId(uniformName);
        setUniformValueGl(uniformId, value);
    }

    template<>
    inline void setUniformValueGl(Uint32_t uniformId, const Vector3f& value) {
        static gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
        gl.glUniform3fv(uniformId, 1, value.data());
    }

    template<>
    inline void setUniformValueGl(const GStringView& uniformName, const Vector4f& value) {
        GLuint uniformId = getUniformId(uniformName);
        setUniformValueGl(uniformId, value);
    }

    template<>
    inline void setUniformValueGl(Uint32_t uniformId, const Vector4f& value) {
        static gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
        gl.glUniform4fv(uniformId, 1, value.data());
    }

    template<>
    inline void setUniformValueGl(const GStringView& uniformName, const Matrix2x2& value) {
        GLuint uniformId = getUniformId(uniformName);
        setUniformValueGl(uniformId, value);
    }

    template<>
    inline void setUniformValueGl(Uint32_t uniformId, const Matrix2x2& value) {
        static gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
        gl.glUniformMatrix2fv(uniformId, 1, GL_FALSE, value.getData()); // uniform ID, count, transpose, value
    }

    template<>
    inline void setUniformValueGl(const GStringView& uniformName, const Matrix3x3& value) {
        GLuint uniformId = getUniformId(uniformName);
        setUniformValueGl(uniformId, value);
    }

    template<>
    inline void setUniformValueGl(Uint32_t uniformId, const Matrix3x3& value) {
        static gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
        gl.glUniformMatrix3fv(uniformId, 1, GL_FALSE, value.getData()); // uniform ID, count, transpose, value
    }

    template<>
    inline void setUniformValueGl(const GStringView& uniformName, const Matrix4x4& value) {
        GLuint uniformId = getUniformId(uniformName);
        setUniformValueGl(uniformId, value);
    }

    template<>
    inline void setUniformValueGl(Uint32_t uniformId, const Matrix4x4& value) {
        static gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
        gl.glUniformMatrix4fv(uniformId, 1, GL_FALSE, value.getData()); // uniform ID, count, transpose, value
    }

    template<>
    void setUniformValueGl(const GStringView& uniformName, const std::vector<Matrix4x4>& value);

    template<>
    inline void setUniformValueGl(Uint32_t uniformId, const Matrix4x4* value, size_t valueCount);

    template<>
    inline void setUniformValueGl(const GStringView& uniformName, const std::vector<float>& value) {
        GLuint uniformId = getUniformId(uniformName);
        setUniformValueGl(uniformId, value.data(), value.size());
    }

    template<>
    inline void setUniformValueGl(Uint32_t uniformId, const float* value, size_t valueCount) {
        static gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
        gl.glUniform1fv(uniformId, (GLsizei)valueCount, value);
    }

    template<>
    void setUniformValueGl(const GStringView& uniformName, const Vec3List& value);

    template<>
    void setUniformValueGl(Uint32_t uniformId, const Vector3* value, size_t valueCount);

    template<>
    void setUniformValueGl(const GStringView& uniformName, const Vec4List& value);

    template<>
    void setUniformValueGl(Uint32_t uniformId, const Vector4* value, size_t valueCount);

    /// @brief links inputs to the shader program to the attributes of a VAO
    virtual void bindAttributes() {}

    /// @brief links an input to the shader program to the attributes of a VAO
    void bindAttribute(int attribute, const QString& variableName);

    /// @brief initializes shader program from the given shaders
    bool initializeShaderProgram(RenderContext& context);

    /// @brief attach the given shader to the shader program
    void attachShader(const Shader& shader);

    /// @brief Detach shader from the shader program (can do once link is done)
    void detachShader(const Shader& shader);

    /// @brief Parses shader code to obtain valid map of uniforms
    void populateUniforms();

    /// @brief Parses shader code to obtain valid map of uniform buffer objects
    void populateUniformBuffers(RenderContext& context);

    /// @brief Get the uniform block index of the specified UBO
    /// @details See: https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL
    /// @note It is assumed that uniform block names are consistent across all shaders and UBOs
    uint32_t getUniformBlockIndex(const QString& blockName);

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
    GLint getNumActiveBuffers(gl::BufferBlockType type);

    /// @brief Get number of active attributes
    GLint getNumActiveAttributes();

	/// @}

	/// @name Protected members
	/// @{

	unsigned int m_programID; ///< GL ID of the shader program
    unsigned int m_newUniformCount = 0; ///< Number of uniforms in queue to actually use for update

    QMutex m_mutex; ///< Lock data for multithreading

    BufferUsageFlags m_bufferUsageFlags = 0;

    /// @todo Make an array
    /// @details Typically is just a vertex and a fragment shader
    std::vector<Shader> m_shaders; ///< Vector of all shaders associated with the shader program
	std::vector<Uniform> m_uniformQueue; ///< Queue for uniforms that need to be updated in GL
    tsl::robin_map<GStringView, ShaderInputInfo> m_uniformInfo; ///< Map of uniform names with corresponding GLSL info
    std::vector<ShaderInputInfo> m_uniformInfoVec; ///< Vector of uniform info
    std::vector<ShaderBufferInfo> m_bufferInfo; ///< Vector of buffer info

    /// @details Currently unused, no custom buffer capability
    //std::vector<GlBuffer*> m_buffers; ///< Buffers associated with the shader

    UniformIdMappings m_uniformMappings;

    /// @see https://stackoverflow.com/questions/34597260/stdhash-value-on-char-value-and-not-on-memory-address
    tsl::robin_map<QString, std::shared_ptr<Ubo>> m_uniformBuffers; ///< Map of uniform buffers associated with this shader program
    static ShaderProgram* s_boundShader; ///< The currently bound shader program

	/// @}
};
typedef std::shared_ptr<ShaderProgram> ShaderProgramPtr;


} // End namespaces
