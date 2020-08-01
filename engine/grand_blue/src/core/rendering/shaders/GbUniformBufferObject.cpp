#include "GbUniformBufferObject.h"

// QT

// Internal
#include "../../utils/GbMemoryManager.h"
#include "../lighting/GbLight.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
BufferUniform::BufferUniform()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
BufferUniform::BufferUniform(const ShaderInputInfo & info):
    m_info(info)
{
    initialize(info);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void BufferUniform::initialize(const ShaderInputInfo & info)
{
    // Set default value for uniform from info
    m_uniform = Uniform(info.m_name);
    switch (info.m_inputType) {
    case ShaderInputType::kInt:
        if (info.isArray()) throw("Error, arrays of int not accepted, try array of vec4");
        m_uniform.set<int>(-1);
        break;
    case ShaderInputType::kBool:
        if (info.isArray()) throw("Error, arrays of bool not accepted, try array of vec4");
        m_uniform.set<bool>(true);
        break;
    case ShaderInputType::kFloat:
        if (info.isArray()) throw("Error, arrays of float not accepted, try array of vec4");
        m_uniform.set<float>(0.0f);
        break;
    case ShaderInputType::kVec4:
        if (info.isArray()) {
            Vec4List vec;
            vec.resize(info.m_arraySize);
            m_uniform.set<Vec4List>(vec);
        }
        else {
            m_uniform.set<Vector4g>(Vector4g());
        }
        break;
    case ShaderInputType::kMat4:
        if (info.isArray()) {
            std::vector<Matrix4x4g> vec;
            vec.resize(info.m_arraySize);
            m_uniform.set<std::vector<Matrix4x4g>>(vec);
        }
        else {
            m_uniform.set<Matrix4x4g>(Matrix4x4g());
        }
        break;
    default:
        throw("Uniform type is invalid for uniform " + info.m_name);
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<UBO> UBO::get(const QString & name)
{
    if (!Map::HasKey(UBO_MAP, name)) {
        return nullptr;
    }
    else {
        return UBO_MAP.at(name);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<UBO> UBO::getLightBuffer()
{
    return get(LIGHT_SETTINGS_BUFFER_NAME);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<UBO> UBO::create(const ShaderStruct & ss)
{
    auto ubo = prot_make_shared<UBO>(ss);
    UBO::UBO_MAP[ss.m_name] = ubo;
    return ubo;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::clearUBOs()
{
    auto endIter = UBO_MAP.end();
    for (auto it = UBO_MAP.begin(); it != endIter;) {
        if (!it->second->isCore())
        {
            // Erase UBO if not a core req
            UBO_MAP.erase(it++);    
        }
        else {
            // Skip if is a core resource
            // FIXME: Clear UBOs in a nice way, something is not right on scenario switch
            it++;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
UBO::UBO(const ShaderStruct& shaderStruct):
    Object(),
    Serializable()
{
    initialize(); // Create the UBO
    populateUniforms(shaderStruct);
    allocateMemory(m_data.m_byteSize);
    bindToPoint(UBO_MAP.size());
}

/////////////////////////////////////////////////////////////////////////////////////////////
UBO::~UBO()
{
    glDeleteBuffers(1, &m_bufferID);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::bind()
{
    // Optimization, don't bind in GL if already bound
    if (isBound()) return;

    glBindBuffer(GL_UNIFORM_BUFFER, m_bufferID);
    BOUND_UBO = this;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::release()
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    BOUND_UBO = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Uniform UBO::getUniformBufferValue(const QString & uniformName)
{
    bind();

    Uniform newUniform(uniformName);
    const BufferUniform& bufferUniform = m_data.m_uniforms[uniformName];
    const Uniform& uniform = bufferUniform.m_uniform;
    //Uniform::UniformType type = (Uniform::UniformType)bufferUniform.m_info.m_uniformType;
    
    // Get uniform value from GL
    // See: https://antongerdelan.net/blog/formatted/2014_06_04_glmapbufferrange.html
    size_t size = ALIGNED_TYPE_SIZES[bufferUniform.m_info.m_inputType];

    // Case based on type
    if (uniform.is<int>()) {
        int* data = (int*)glMapBufferRange(GL_UNIFORM_BUFFER,
            bufferUniform.m_offset,
            size,
            GL_MAP_READ_BIT);
        int count = *data;
        newUniform.set<int>(count);
    }
    else if (uniform.is<bool>()) {
        int* data = (int*)glMapBufferRange(GL_UNIFORM_BUFFER,
            bufferUniform.m_offset,
            size,
            GL_MAP_READ_BIT);
        newUniform.set<bool>(bool(*data));
    }
    else if (uniform.is<float>()) {
        float* data = (float*)glMapBufferRange(GL_UNIFORM_BUFFER,
            bufferUniform.m_offset,
            size,
            GL_MAP_READ_BIT);
        newUniform.set<float>(*data);
    }
    else if (uniform.is<Vector4f>()) {
        float* data = (float*)glMapBufferRange(GL_UNIFORM_BUFFER,
            bufferUniform.m_offset,
            size,
            GL_MAP_READ_BIT);
        newUniform.set<Vector4f>(Vector4f(data));
    }
    else if (uniform.is<Matrix4x4f>()) {
        float* data = (float*)glMapBufferRange(GL_UNIFORM_BUFFER,
            bufferUniform.m_offset,
            size,
            GL_MAP_READ_BIT);
        newUniform.set<Matrix4x4g>(Matrix4x4g(data));
    }
    else if (uniform.is<std::vector<Matrix4x4f>>()) {
        const std::vector<Matrix4x4g>& value = uniform.get<std::vector<Matrix4x4g>>();
        float* data = (float*)glMapBufferRange(GL_UNIFORM_BUFFER,
            bufferUniform.m_offset,
            size * value.size(),
            GL_MAP_READ_BIT);
        std::vector<Matrix4x4g> mats;
        for (size_t i = 0; i < value.size() * 16; i += 16) {
            Vec::EmplaceBack(mats, Matrix4x4g(data + i)); // untested
        }
        newUniform.set<std::vector<Matrix4x4g>>(mats);
    }
    else if (uniform.is<Vec4List>()) {
        const Vec4List& value = uniform.get<Vec4List>();
        float* data = (float*)glMapBufferRange(GL_UNIFORM_BUFFER,
            bufferUniform.m_offset,
            size * value.size(),
            GL_MAP_READ_BIT);
        std::vector<float> floatVec(data, data + value.size() * 4);
        std::vector<Vector4g> newVec;
        for (size_t i = 0; i < floatVec.size(); i += 4) {
            Vec::EmplaceBack(newVec, Vector4g{ floatVec[i], floatVec[i + 1], floatVec[i + 2], floatVec[i + 3] });
        }
        //logInfo(bufferUniform.m_uniform.getName() + " ---------------------------------");
        //for (size_t i = 0; i < value.size(); i++) {
        //    logInfo(QString(newVec[i]) + ", " + QString(value[i]));
        //}
        newUniform.set<Vec4List>(newVec);
    }
    else {
        std::string tname = uniform.typeInfo().name();
        logError("Structures that are not 16-byte aligned are not supported by this UBO implementation");
#ifdef DEBUG_MODE
        throw("Error, uniform " + uniform.getName() +
            " is of type " + QString::fromStdString(tname) + ". This uniform type is not supported: ");
#else
        logError("Error, uniform " + uniform.getName() +
            " is of type " + QString::fromStdString(tname) + ". This uniform type is not supported: ");
#endif
    }

    // Must be called, or things will break
    glUnmapBuffer(GL_UNIFORM_BUFFER);

    return newUniform;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::initialize() {
    // Generate the Uniform Buffer Object
    glGenBuffers(1, &m_bufferID);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::refreshUniform(const QString & uniformName)
{
    if (!m_data.m_uniforms.count(uniformName)) {
#ifdef DEBUG_MODE
        throw("Error, uniform name not recognized: " + uniformName);
#else
        logError("Error, uniform name not recognized: " + uniformName);
#endif
    }
    bind();
    BufferUniform& uniform = m_data.m_uniforms.at(uniformName);
    refreshUniform(uniform);

}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::refreshUniform(const BufferUniform & bufferUniform)
{
    bind();

    const Uniform& uniform = bufferUniform.m_uniform;
    // Update uniform in openGL
    if (uniform.is<int>()) {
        glBufferSubData(GL_UNIFORM_BUFFER, bufferUniform.m_offset, bufferUniform.m_alignmentByteSize, &uniform.get<int>());
    }
    else if (uniform.is<bool>()) {
        glBufferSubData(GL_UNIFORM_BUFFER, bufferUniform.m_offset, bufferUniform.m_alignmentByteSize, &uniform.get<bool>());
    }
    else if (uniform.is<float>()) {
        glBufferSubData(GL_UNIFORM_BUFFER, bufferUniform.m_offset, bufferUniform.m_alignmentByteSize, &uniform.get<float>());
    }
    else if (uniform.is<Vector4f>()) {
        glBufferSubData(GL_UNIFORM_BUFFER, bufferUniform.m_offset, bufferUniform.m_alignmentByteSize, &uniform.get<Vector4f>());
    }
    else if (uniform.is<Matrix4x4f>()) {
        glBufferSubData(GL_UNIFORM_BUFFER, bufferUniform.m_offset, bufferUniform.m_alignmentByteSize, uniform.get<Matrix4x4g>().getData());
    }
    else if (uniform.is<std::vector<Matrix4x4f>>()) {
        const std::vector<Matrix4x4g>& value = uniform.get<std::vector<Matrix4x4g>>();
        size_t offset = bufferUniform.m_offset;
        size_t size = ALIGNED_TYPE_SIZES[ShaderInputType::kMat4];
        for (const Matrix4x4g& mat : value) {
            glBufferSubData(GL_UNIFORM_BUFFER, offset, size, mat.getData());
            offset += size;
        }
    }
    else if (uniform.is<Vec4List>()) {
        const Vec4List& value = uniform.get<Vec4List>();
        size_t offset = bufferUniform.m_offset;
        size_t size = ALIGNED_TYPE_SIZES[ShaderInputType::kVec4];
        for (const Vector4f& vec : value) {
            glBufferSubData(GL_UNIFORM_BUFFER, offset, size, vec.getData());
            offset += size;
        }
    }
    else {
        std::string tname = uniform.typeInfo().name();
        logError("Structures that are not 16-byte aligned are not supported by this UBO implementation");
#ifdef DEBUG_MODE
        throw("Error, uniform " + uniform.getName() +
            " is of type " + QString::fromStdString(tname) + ". This uniform type is not supported: ");
#else
        logError("Error, uniform " + uniform.getName() +
            " is of type " + QString::fromStdString(tname) + ". This uniform type is not supported: ");
#endif
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::allocateMemory(size_t numBytes, int usageType) {
    bind();
    glBufferData(GL_UNIFORM_BUFFER, numBytes, NULL, usageType);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::bindToPoint(size_t bindingPoint) {
    bind();
    m_bindingPoint = bindingPoint;
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_bufferID);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::bindToPoint(size_t bindingPoint, size_t offset, size_t sizeInBytes) {
    bind();
    m_bindingPoint = bindingPoint;
    glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, m_bufferID, offset, sizeInBytes);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::populateUniforms(const ShaderStruct& shaderStruct)
{
    // FIXME: Will not work correctly if there is, for example, a vec3 followed by a float.
    // This will incorrectly add padding for a vec3, assuming a byteSize of 16, when in reality, we will have 12 + 4, with no padding
    // SOLUTION: Do NOT use vec3s or similarly unaligned types in uniform blocks
    m_name = shaderStruct.m_name;
    size_t vec4Size = ALIGNED_TYPE_SIZES[ShaderInputType::kVec4];
    m_data.m_byteSize = 0;
    for (const ShaderInputInfo& info: shaderStruct.m_fields) {
        m_data.m_uniforms[info.m_name] = BufferUniform(info);
        BufferUniform& uniform = m_data.m_uniforms[info.m_name];
        size_t typeSize = ALIGNED_TYPE_SIZES[info.m_inputType];
        if (info.isArray()) {
            // Sizing according to std140 spacing
            uniform.m_alignmentByteSize = vec4Size * info.m_arraySize;
        }
        else {
            uniform.m_alignmentByteSize = typeSize;
        }

        size_t amountOverAlignment = m_data.m_byteSize % 16;
        if (amountOverAlignment != 0) {
            // The current offset is not a multiple of 16 as required
            size_t requiredPadding = 16 - amountOverAlignment;
            if (uniform.m_alignmentByteSize > requiredPadding) {
                // Need to pad, since the current uniform size will not pad to a multiple of 16
                m_data.m_byteSize += requiredPadding;
            }
        }
        uniform.m_offset = m_data.m_byteSize;
        m_data.m_byteSize += uniform.m_alignmentByteSize;
    }

    // Final check that size is a multiple of 16
    size_t requiredPadding = m_data.m_byteSize % 16 == 0? 0: 16 - (m_data.m_byteSize % 16);
    m_data.m_byteSize += requiredPadding;
}

/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue UBO::asJson() const {
    QJsonObject object;

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::loadFromJson(const QJsonValue& json) {

}
/////////////////////////////////////////////////////////////////////////////////////////////
UBO* UBO::BOUND_UBO = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////////
std::unordered_map<QString, std::shared_ptr<UBO>> UBO::UBO_MAP = {};

/////////////////////////////////////////////////////////////////////////////////////////////
/// Scalar e.g. int or bool: 	 Each scalar has a base alignment of N.
/// Vector 	Either 2N or 4N;     This means that a vec3 has a base alignment of 4N.
/// Array of scalars or vectors: Each element has a base alignment equal to that of a vec4.
/// Matrices:                  	 Stored as a large array of column vectors, where each of those vectors has a base alignment of vec4.
/// Struct: 	                 Equal to the computed size of its elements according to the previous rules, but padded to a multiple of the size of a vec4.
std::unordered_map<ShaderInputType, size_t> UBO::ALIGNED_TYPE_SIZES = {
    {ShaderInputType::kBool,        sizeof(GLfloat)},
    {ShaderInputType::kInt,         sizeof(GLfloat)},
    {ShaderInputType::kFloat,       sizeof(GLfloat)},
    {ShaderInputType::kDouble,      sizeof(GLfloat)},
    {ShaderInputType::kVec2,        2 * sizeof(GLfloat)},
    {ShaderInputType::kVec3,        4 * sizeof(GLfloat)},
    {ShaderInputType::kVec4,        4 * sizeof(GLfloat)},
    {ShaderInputType::kMat2,        2 * 4 * sizeof(GLfloat)},
    {ShaderInputType::kMat3,        3 * 4 * sizeof(GLfloat)},
    {ShaderInputType::kMat4,        4 * 4 * sizeof(GLfloat)},
    {ShaderInputType::kSamplerCube, sizeof(GLfloat)},
    {ShaderInputType::kSampler2D,   sizeof(GLfloat)}
};



/////////////////////////////////////////////////////////////////////////////////////////////
// End namespacing
}