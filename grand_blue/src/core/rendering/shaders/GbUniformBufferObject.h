// See:
// https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL

#ifndef GB_UNIFORM_BUFFER_OBJECT_H
#define GB_UNIFORM_BUFFER_OBJECT_H

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
#include "GbUniform.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// TypeDefs
/////////////////////////////////////////////////////////////////////////////////////////////
typedef std::vector<Vector3g> Vec3List;
typedef std::vector<Vector4g> Vec4List;


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
struct ShaderStruct;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

///@brief Uniform and corresponding info about a uniform's offset in the uniform buffer
struct BufferUniform {
    BufferUniform();
    BufferUniform(const UniformInfo& info);

    Uniform m_uniform;
    size_t m_offset; // The offset of the uniform in the buffer
    size_t m_alignmentByteSize; // The alignment size (padded) of the uniform in the buffer
    UniformInfo m_info; // Info relating to the uniform itself

private:
    void initialize(const UniformInfo& info);
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

///@brief Struct representing uniform buffer data
struct UniformBufferData {

    /// @brief Map of uniforms in the buffer
    std::unordered_map<QString, BufferUniform> m_uniforms;

    /// @brief The total byte size of the struct, taking into account alignment offsets
    size_t m_byteSize;
};



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class representing a Uniform Buffer Object
class UBO : public Object, 
    public Serializable, 
    private GL::OpenGLFunctions{
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Block layout options
    /// @details Each variable type in GLSL such as int, float and bool are defined to be 
    /// four-byte quantities with each entity of 4 bytes represented as N. 
    /// Scalar e.g. int or bool: 	 Each scalar has a base alignment of N.
    /// Vector 	Either 2N or 4N;     This means that a vec3 has a base alignment of 4N.
    /// Array of scalars or vectors: Each element has a base alignment equal to that of a vec4.
    /// Matrices:                  	 Stored as a large array of column vectors, where each of those vectors has a base alignment of vec4.
    /// Struct: 	                 Equal to the computed size of its elements according to the previous rules, but padded to a multiple of the size of a vec4.
    enum BlockLayout {
        kShared,
        kPacked,
        kStd140
    };

    /// @brief Obtain the UBO with the given name
    static std::shared_ptr<UBO> get(const QString& name);
    static std::shared_ptr<UBO> getCameraBuffer() {
        return get(QStringLiteral("CameraMatrices"));
    }
    static std::shared_ptr<UBO> getLightBuffer() {
        return get(QStringLiteral("LightBuffer"));
    }

    /// @brief Create a UBO
    static std::shared_ptr<UBO> create(const ShaderStruct& ss);

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

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Whether or not the UBO is bound to a context
    inline bool isBound() const {
        if (!BOUND_UBO) return false;
        return m_uuid == BOUND_UBO->getUuid();
    }

    /// @brief Bind the UBO
    void bind();

    /// @brief Unbind the UBO
    void release();

    /// @brief Whether or not the UBO has the specified uniform
    bool hasUniform(const QString& uniformName) const {
        return Map::HasKey(m_data.m_uniforms, uniformName);
    }

    /// @brief Uniform values
    const Uniform& getUniformValue(const QString& uniformName) const {
        if (!Map::HasKey(m_data.m_uniforms, uniformName)) {
            throw("Error, uniform " + uniformName + " does not exist in UBO " + m_name);
        }
        return m_data.m_uniforms.at(uniformName).m_uniform;
    }

    /// @brief Uniform values from buffer
    Uniform getUniformBufferValue(const QString& uniformName);

    template<class VariantType>
    inline void setUniformValue(const QString& uniformName, const VariantType& value) {
        BufferUniform& bufferUniform = m_data.m_uniforms[uniformName];
        bufferUniform.m_uniform = Uniform(uniformName, value);
        refreshUniform(bufferUniform);
    }
    void setUniformValue(const Uniform& uniform) {
        BufferUniform& bufferUniform = m_data.m_uniforms[uniform.getName()];
        bufferUniform.m_uniform = uniform;
        refreshUniform(bufferUniform);
    }

    /// @brief Set a value in the specified list-like uniform, offset from the start of the uniform by a specified amount
    template<class T>
    void setUniformSubValue(const QString& uniformName, size_t index, const T& value) {
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
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

protected:
    friend class ShaderProgram;
    friend struct UniformBufferData;

    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief The currently bound UBO
    static UBO* BOUND_UBO;

    /// @brief Static map of UBOs, indexed by name (UBOs should always use same name in shader)
    static std::unordered_map<QString, std::shared_ptr<UBO>> UBO_MAP;

    /// @brief Map of type names to their sizes (according to std140 glsl sizing)
    static std::unordered_map<QString, size_t> ALIGNED_TYPE_SIZES;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors
    /// @{
    /// @}

    /// @brief Is private so that UBOs can always be added to static map on construction
    UBO(const ShaderStruct& shaderStruct);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    //void getUniformIndices() const;

    void initialize();

    /// @brief Update a uniform in the OpenGL buffer
    void refreshUniform(const QString& uniformName);
    void refreshUniform(const BufferUniform& bufferUniform);

    /// @brief Allocate memory for the buffer in OpenGL
    void allocateMemory(size_t numBytes, int usageType = GL_STREAM_DRAW);

    /// @brief Bind to a binding point
    void bindToPoint(size_t bindingPoint);

    /// @brief Bind a part of the buffer to a binding point, starting at offset and for sizeInBytes bytes
    void bindToPoint(size_t bindingPoint, size_t offset, size_t sizeInBytes);

    /// @brief Generate spacing and size information from the given shader struct
    void populateUniforms(const ShaderStruct& shaderStruct);

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
    
    /// @}

};



    /////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif