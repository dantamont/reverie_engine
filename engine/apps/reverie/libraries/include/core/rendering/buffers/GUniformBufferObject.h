#pragma once
/// @see https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL

// Standard
#include <cstdlib>
#include <memory>
#include <vector>

// QT
#include <QObject>
#include <QString>

// Internal 
#include "enums/GCameraBufferUniformNameEnum.h"
#include "enums/GLightBufferUniformNameEnum.h"
#include "fortress/containers/GVariant.h"
#include "fortress/containers/math/GMatrix.h"
#include "fortress/containers/GContainerExtensions.h"
#include "core/rendering/GGLFunctions.h"
#include "core/rendering/shaders/GUniformContainer.h"
#include "core/rendering/shaders/GShaderProgram.h"

namespace rev {

typedef std::vector<Vector3> Vec3List;
typedef std::vector<Vector4> Vec4List;

class RenderContext;
struct ShaderBufferInfo;
template<typename EnumType, typename EnumWrapperType> class EnumeratedUbo;
typedef EnumeratedUbo<ECameraBufferUniformName, GCameraBufferUniformName> CameraUbo;
typedef EnumeratedUbo<ELightBufferUniformName, GLightBufferUniformName> LightUbo;


struct BufferUniformData {
    UniformData m_uniformData;
    size_t m_offset; ///< The offset in bytes of the uniform in the buffer
    size_t m_alignmentByteSize; // The alignment size (padded) of the uniform in the buffer
};

/// @brief Uniform and corresponding info about a uniform's offset in the uniform buffer
/// @todo Don't use Uniform, since there are redundant fields, and the ID is unused for UBOs
struct BufferUniform {
    BufferUniform();
    BufferUniform(const ShaderInputInfo& info, UniformContainer& outContainer);

    BufferUniformData m_data; ///< The uniform data
    ShaderInputInfo m_info; ///< Meta-data relating to the uniform itself

private:
    void initialize(const ShaderInputInfo& info, UniformContainer& outContainer);
};

///@brief Struct representing uniform buffer data
struct UniformBufferData {

    std::vector<BufferUniform> m_uniforms; ///< Uniforms in the buffer
    UniformContainer& m_uniformValues; ///< The values of the uniforms
    size_t m_byteSize{ 0 }; ///< The total byte size of the struct, taking into account alignment offsets
};


/// @brief Class representing a Uniform Buffer Object
// TODO: Move much of this into a generic GlBuffer object, see SSB (Shader Storage Buffer) as well
class Ubo : 
    public IdentifiableInterface,
    private gl::OpenGLFunctions{
public:
    /// @name Static
    /// @{

    /// @brief Obtain the UBO with the given buffer ID
    static std::shared_ptr<Ubo> Get(Uint32_t id, Int32_t& outIndex);
    static std::shared_ptr<Ubo> Get(const char* name);

    static std::shared_ptr<CameraUbo> GetCameraBuffer();
    static std::shared_ptr<LightUbo> GetLightSettingsBuffer();

    /// @brief Create a UBO
    static std::shared_ptr<Ubo> Create(RenderContext& context, const ShaderBufferInfo& ss);

    /// @brief Clear the static map of UBOs
    static void ClearUBOs(bool clearCore = false);

    /// @}

    /// @name Destructor
    /// @{
    /// @}

    ~Ubo();

    /// @}

    /// @name Public methods
    /// @{

    UniformBufferData& data() { return m_data; }

    bool isCore() const { return m_isCore; }
    void setCore(bool isCore) { m_isCore = isCore; }

    Uint32_t getBufferId() const { return m_bufferID; }

    const GString& getName() const { return m_name; }

    /// @brief Initialize from buffer info
    void initialize(const ShaderBufferInfo& info);

    /// @brief Whether or not the UBO is bound to a context
    inline bool isBound() const {
        if (!s_boundUBO) return false;
        return m_uuid == s_boundUBO->getUuid();
    }

    /// @brief Bind the UBO
    void bind();

    /// @brief Unbind the UBO
    void release();

    /// @brief Whether or not the UBO has the specified uniform
    bool hasBufferUniform(const GStringView& uniformName) const;

    /// @brief Uniform values
    const BufferUniformData& getBufferUniformData(const GStringView& uniformName) const;
    BufferUniformData& getBufferUniformData(const GStringView& uniformName);
    const BufferUniformData& getBufferUniformDataAtOffset(Uint32_t offset) const;
    BufferUniformData& getBufferUniformDataAtOffset(Uint32_t offset);

    template<typename T>
    const T& getBufferUniformValue(const GStringView& uniformName) const {
        return getBufferUniformData(uniformName).m_uniformData.getValue<T>(m_data.m_uniformValues);
    }

    void setBufferUniformValueFromName(const GStringView& uniformName, const UniformData& uniform);
    void setBufferUniformValue(const BufferUniformData& uniform);

    virtual void postConstruct(){}

    /// @}

protected:
    friend class ShaderProgram;
    friend struct UniformBufferData;

    /// @name Static
    /// @{

    static Ubo* s_boundUBO; ///< The currently bound UBO
    static std::vector<std::shared_ptr<Ubo>> s_uboList; ///< Vector of UBOs (UBOs should always use same name in shader)
    static tsl::robin_map<ShaderVariableType, size_t> s_alignedTypeSizes; ///< Map of type names to their sizes (according to std140 glsl sizing)

    /// @}

    /// @name Constructors
    /// @{
    /// @}

    /// @brief Is private so that UBOs can always be added to static map on construction
    Ubo(RenderContext& context, const ShaderBufferInfo& shaderStruct);

    /// @}

    /// @name Protected methods
    /// @{

    //void getUniformIndices() const;

    void initialize();

    /// @brief Update a uniform in the OpenGL buffer
    void refreshUniform(const BufferUniformData& bufferUniformData);

    /// @brief Allocate memory for the buffer in OpenGL
    void allocateMemory(size_t numBytes, int usageType = GL_STREAM_DRAW);

    /// @brief Bind to a binding point
    void bindToPoint(size_t bindingPoint);

    /// @brief Bind a part of the buffer to a binding point, starting at offset and for sizeInBytes bytes
    void bindToPoint(size_t bindingPoint, size_t offset, size_t sizeInBytes);

    /// @brief Generate spacing and size information from the given shader struct
    void populateUniforms(const ShaderBufferInfo& bufferInfo);

    /// @}

    /// @name Protected members
    /// @{

    unsigned int m_bufferID; ///< the GL ID of the UBO
    UniformBufferData m_data; ///< The uniform data for the buffer
    unsigned int m_bindingPoint; ///< The binding point of the UBO, should never change
    bool m_isCore = false; ///< Whether or not this UBO will be deleted on scenario reload
    GString m_name; ///< The name of the UBO
    
    static constexpr char* s_cameraBufferName = "CameraBuffer";
    static constexpr char* s_lightBufferName = "LightSettingsBuffer";

    /// @}

};

/// @brief Interface class for specializing a UBO for specific use cases
template<typename EnumType, typename EnumWrapperType>
class EnumeratedUbo : public Ubo {
private:

    static constexpr Int64_t s_indexCount = (Int64_t)EnumType::eCOUNT;

public:
    using Ubo::Ubo;
    using Ubo::setBufferUniformValue;

    /// @brief Return the uniform with the specified name
    template<EnumType UniformName>
    BufferUniformData& getBufferUniformData() {
        constexpr Uint32_t s_nameIndex = (Uint32_t)UniformName;
        Uint32_t index = m_bufferIndices[s_nameIndex];
        return m_data.m_uniforms[index].m_data;
    }

    template<EnumType UniformName>
    void setBufferUniformValue(const UniformData& uniformData) {
        constexpr Uint32_t s_nameIndex = (Uint32_t)UniformName;
        Uint32_t index = m_bufferIndices[s_nameIndex];
        BufferUniformData& info = m_data.m_uniforms[index].m_data;
        info.m_uniformData = uniformData;
        refreshUniform(info);
    }

    void postConstruct() override
    {
        for (Int64_t i = 0; i < s_indexCount; i++) {
            const GString name = EnumWrapperType::ToString(EnumType(i)).lowerCasedFirstLetter();
            auto iter = std::find_if(m_data.m_uniforms.begin(), m_data.m_uniforms.end(),
                [name](const BufferUniform& uniform) {
                    return uniform.m_info.m_name == name;
                }
            );
            assert(iter != m_data.m_uniforms.end() && "Uniform not found");
            Uint32_t index = iter - m_data.m_uniforms.begin();
            m_bufferIndices[i] = index;
        }
    }

private:
    std::array<Uint32_t, s_indexCount> m_bufferIndices; ///< Indices of uniforms in local Ubo vector
};

typedef EnumeratedUbo<ECameraBufferUniformName, GCameraBufferUniformName> CameraUbo;
typedef EnumeratedUbo<ELightBufferUniformName, GLightBufferUniformName> LightUbo;

} // End namespaces
