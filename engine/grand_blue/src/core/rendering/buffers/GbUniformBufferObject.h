// See:
// https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL

#ifndef GB_UNIFORM_BUFFER_OBJECT_H
#define GB_UNIFORM_BUFFER_OBJECT_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Defines
/////////////////////////////////////////////////////////////////////////////////////////////
#define CAMERA_BUFFER_NAME QStringLiteral("CameraBuffer")

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard
#include <cstdlib>
#include <memory>
#include <vector>

// QT
#include <QObject>
#include <QString>

// Internal 
#include "../../GbObject.h"
#include "../../containers/GbVariant.h"
#include "../../geometry/GbMatrix.h"
#include "../../containers/GbContainerExtensions.h"
#include "../GbGLFunctions.h"
#include "../shaders/GbUniform.h"
#include "../shaders/GbShaders.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// TypeDefs
/////////////////////////////////////////////////////////////////////////////////////////////
typedef std::vector<Vector3> Vec3List;
typedef std::vector<Vector4> Vec4List;


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
//struct ShaderStruct;
struct ShaderBufferInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

///@brief Uniform and corresponding info about a uniform's offset in the uniform buffer
struct BufferUniform {
    BufferUniform();
    BufferUniform(const ShaderInputInfo& info);

    Uniform m_uniform;
    size_t m_offset; // The offset of the uniform in the buffer
    size_t m_alignmentByteSize; // The alignment size (padded) of the uniform in the buffer
    ShaderInputInfo m_info; // Info relating to the uniform itself

private:
    void initialize(const ShaderInputInfo& info);
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

///@brief Struct representing uniform buffer data
struct UniformBufferData {

    /// @brief Map of uniforms in the buffer
    tsl::robin_map<GStringView, BufferUniform> m_uniforms;

    /// @brief The total byte size of the struct, taking into account alignment offsets
    size_t m_byteSize;
};



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class representing a Uniform Buffer Object
// TODO: Move much of this into a generic GLBuffer object, see SSB (Shader Storage Buffer) as well
class UBO : public Object, 
    public Serializable, 
    private GL::OpenGLFunctions{
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Obtain the UBO with the given name
    static std::shared_ptr<UBO> get(const QString& name);
    static std::shared_ptr<UBO> getCameraBuffer();
    static std::shared_ptr<UBO> getLightSettingsBuffer();

    /// @brief Create a UBO
    static std::shared_ptr<UBO> create(const ShaderBufferInfo& ss);

    /// @brief Clear the static map of UBOs
    static void clearUBOs();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Destructor
    /// @{
    /// @}

    ~UBO();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    UniformBufferData& data() { return m_data; }

    bool isCore() const { return m_isCore; }
    void setCore(bool isCore) { m_isCore = isCore; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

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
    bool hasUniform(const GStringView& uniformName) const {
        return Map::HasKey(m_data.m_uniforms, uniformName);
    }

    /// @brief Uniform values
    const Uniform& getUniformValue(const GStringView& uniformName) const;

    /// @brief Uniform values from buffer
    Uniform getUniformBufferValue(const GString& uniformName);

    template<class VariantType>
    inline void setUniformValue(const GStringView& uniformName, const VariantType& value) {
        BufferUniform& bufferUniform = m_data.m_uniforms[uniformName];
        // This check was causing problems, so it is no bueno.
        //if (bufferUniform.m_uniform.get<VariantType>() != value) {
            bufferUniform.m_uniform = Uniform(uniformName, value);
            refreshUniform(bufferUniform);
        //}
    }
    void setUniformValue(const Uniform& uniform);

    /// @brief Set a value in the specified list-like uniform, offset from the start of the uniform by a specified amount
    template<class T>
    void setUniformSubValue(const GStringView& uniformName, size_t index, const T& value) {
        BufferUniform& bufferUniform = m_data.m_uniforms[uniformName];
        //QString typeName(bufferUniform.m_uniform.typeInfo().name());
        if (!bufferUniform.m_uniform.is<std::vector<T>>()) {
            throw("Error, uniform is not a vector type");
        }
        std::vector<T>& uniformVal = bufferUniform.m_uniform.get<std::vector<T>>();
        uniformVal[index] = value;
        // TODO: Replace this with a less obliterative call
        refreshUniform(bufferUniform);
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    friend class ShaderProgram;
    friend struct UniformBufferData;

    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief The currently bound UBO
    static UBO* s_boundUBO;

    /// @brief Static map of UBOs, indexed by name (UBOs should always use same name in shader)
    static tsl::robin_map<QString, std::shared_ptr<UBO>> s_UBOMap;

    /// @brief Map of type names to their sizes (according to std140 glsl sizing)
    static tsl::robin_map<ShaderVariableType, size_t> s_alignedTypeSizes;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors
    /// @{
    /// @}

    /// @brief Is private so that UBOs can always be added to static map on construction
    UBO(const ShaderBufferInfo& shaderStruct);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    //void getUniformIndices() const;

    void initialize();

    /// @brief Update a uniform in the OpenGL buffer
    void refreshUniform(const GString& uniformName);
    void refreshUniform(const BufferUniform& bufferUniform);

    /// @brief Allocate memory for the buffer in OpenGL
    void allocateMemory(size_t numBytes, int usageType = GL_STREAM_DRAW);

    /// @brief Bind to a binding point
    void bindToPoint(size_t bindingPoint);

    /// @brief Bind a part of the buffer to a binding point, starting at offset and for sizeInBytes bytes
    void bindToPoint(size_t bindingPoint, size_t offset, size_t sizeInBytes);

    /// @brief Generate spacing and size information from the given shader struct
    void populateUniforms(const ShaderBufferInfo& bufferInfo);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief the GL ID of the UBO
    unsigned int m_bufferID;

    /// @brief The uniform data for the buffer
    UniformBufferData m_data;

    /// @brief The binding point of the UBO, should never change
    unsigned int m_bindingPoint;

    /// @brief Whether or not this UBO will be deleted on scenario reload
    bool m_isCore = false;
    
    /// @}

};



    /////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif