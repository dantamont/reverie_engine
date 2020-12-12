#include "GbUniformBufferObject.h"

// QT

// Internal
#include "../../utils/GbMemoryManager.h"
#include "../lighting/GbLightSettings.h"
#include "../shaders/GbShaders.h"

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
    m_uniform = Uniform(info.name());
    switch (info.m_variableType) {
    case ShaderVariableType::kInt:
        if (info.isArray()) throw("Error, arrays of int not accepted, try array of vec4");
        m_uniform.set<int>(-1);
        break;
    case ShaderVariableType::kBool:
        if (info.isArray()) throw("Error, arrays of bool not accepted, try array of vec4");
        m_uniform.set<bool>(true);
        break;
    case ShaderVariableType::kFloat:
        if (info.isArray()) throw("Error, arrays of float not accepted, try array of vec4");
        m_uniform.set<float>(0.0f);
        break;
    case ShaderVariableType::kUVec2:
        m_uniform.set<Vector<size_t, 2>>();
        break;
    case ShaderVariableType::kVec4:
        if (info.isArray()) {
            Vec4List vec;
            vec.resize(info.m_arraySize);
            m_uniform.set<Vec4List>(vec);
        }
        else {
            m_uniform.set<Vector4>(Vector4());
        }
        break;
    case ShaderVariableType::kMat4:
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
        throw("Uniform type is invalid for uniform " + QString(info.name()));
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<UBO> UBO::get(const QString & name)
{
    if (!Map::HasKey(s_UBOMap, name)) {
        return nullptr;
    }
    else {
        return s_UBOMap.at(name);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<UBO> UBO::getCameraBuffer()
{
    return get(CAMERA_BUFFER_NAME);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<UBO> UBO::getLightSettingsBuffer()
{
    return get(LIGHT_SETTINGS_BUFFER_NAME);
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<UBO> UBO::create(const ShaderBufferInfo & ss)
{
    auto ubo = prot_make_shared<UBO>(ss);
    UBO::s_UBOMap[ss.m_name] = ubo;
    return ubo;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::clearUBOs()
{
    auto endIter = s_UBOMap.end();
    for (auto it = s_UBOMap.begin(); it != endIter;) {
        if (!it->second->isCore())
        {
            // Erase UBO if not a core req
            s_UBOMap.erase(it++);    
        }
        else {
            // Skip if is a core resource
            // FIXME: Clear UBOs in a nice way, something is not right on scenario switch
            it++;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
UBO::UBO(const ShaderBufferInfo& shaderStruct):
    Object(),
    Serializable()
{
    initialize(shaderStruct);
}

/////////////////////////////////////////////////////////////////////////////////////////////
UBO::~UBO()
{
    glDeleteBuffers(1, &m_bufferID);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::initialize(const ShaderBufferInfo & info)
{
    initialize(); // Create the UBO
    populateUniforms(info);
    allocateMemory(m_data.m_byteSize);
    bindToPoint(s_UBOMap.size());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::bind()
{
    // Optimization, don't bind in GL if already bound
    if (isBound()) return;

    glBindBuffer(GL_UNIFORM_BUFFER, m_bufferID);
    s_boundUBO = this;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::release()
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    s_boundUBO = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
const Uniform & UBO::getUniformValue(const GStringView & uniformName) const
{
    if (!Map::HasKey(m_data.m_uniforms, uniformName)) {
        throw("Error, uniform " + uniformName + " does not exist in UBO " + GString(m_name));
    }
    return m_data.m_uniforms.at(uniformName).m_uniform;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Uniform UBO::getUniformBufferValue(const GString & uniformName)
{
    bind();

    Uniform newUniform(uniformName);
    const BufferUniform& bufferUniform = m_data.m_uniforms[uniformName];
    const Uniform& uniform = bufferUniform.m_uniform;
    //Uniform::UniformType type = (Uniform::UniformType)bufferUniform.m_info.m_uniformType;
    
    // Get uniform value from GL
    // See: https://antongerdelan.net/blog/formatted/2014_06_04_glmapbufferrange.html
    size_t size = s_alignedTypeSizes[bufferUniform.m_info.m_variableType];

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
    else if (uniform.is<Matrix4x4>()) {
        float* data = (float*)glMapBufferRange(GL_UNIFORM_BUFFER,
            bufferUniform.m_offset,
            size,
            GL_MAP_READ_BIT);
        newUniform.set<Matrix4x4g>(Matrix4x4g(data));
    }
    else if (uniform.is<std::vector<Matrix4x4>>()) {
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
        std::vector<Vector4> newVec;
        for (size_t i = 0; i < floatVec.size(); i += 4) {
            Vec::EmplaceBack(newVec, Vector4{ floatVec[i], floatVec[i + 1], floatVec[i + 2], floatVec[i + 3] });
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
        throw("Error, uniform " + QString(uniform.getName()) +
            " is of type " + QString::fromStdString(tname) + ". This uniform type is not supported: ");
#else
        logError("Error, uniform " + QString(uniform.getName()) +
            " is of type " + QString::fromStdString(tname) + ". This uniform type is not supported: ");
#endif
    }

    // Must be called, or things will break (changes won't be pushed to GL)
    glUnmapBuffer(GL_UNIFORM_BUFFER);

    return newUniform;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::setUniformValue(const Uniform & uniform)
{
    BufferUniform& bufferUniform = m_data.m_uniforms[uniform.getName()];
    //if (bufferUniform.m_uniform != uniform) {
        bufferUniform.m_uniform = uniform;
        refreshUniform(bufferUniform);
    //}
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::initialize() {
    // Generate the Uniform Buffer Object
    glGenBuffers(1, &m_bufferID);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void UBO::refreshUniform(const GString& uniformName)
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
    else if (uniform.is<Vector<size_t, 2>>()) {
        glBufferSubData(GL_UNIFORM_BUFFER, bufferUniform.m_offset, bufferUniform.m_alignmentByteSize, &uniform.get<Vector<size_t, 2>>());
    }
    else if (uniform.is<Vector4f>()) {
        glBufferSubData(GL_UNIFORM_BUFFER, bufferUniform.m_offset, bufferUniform.m_alignmentByteSize, &uniform.get<Vector4f>());
    }
    else if (uniform.is<Matrix4x4>()) {
        glBufferSubData(GL_UNIFORM_BUFFER, bufferUniform.m_offset, bufferUniform.m_alignmentByteSize, uniform.get<Matrix4x4g>().getData());
    }
    else if (uniform.is<std::vector<Matrix4x4>>()) {
        const std::vector<Matrix4x4g>& value = uniform.get<std::vector<Matrix4x4g>>();
        size_t offset = bufferUniform.m_offset;
        size_t size = s_alignedTypeSizes[ShaderVariableType::kMat4];
        for (const Matrix4x4g& mat : value) {
            glBufferSubData(GL_UNIFORM_BUFFER, offset, size, mat.getData());
            offset += size;
        }
    }
    else if (uniform.is<Vec4List>()) {
        const Vec4List& value = uniform.get<Vec4List>();
        size_t offset = bufferUniform.m_offset;
        size_t size = s_alignedTypeSizes[ShaderVariableType::kVec4];
        for (const Vector4f& vec : value) {
            glBufferSubData(GL_UNIFORM_BUFFER, offset, size, vec.getData());
            offset += size;
        }
    }
    else {
        std::string tname = uniform.typeInfo().name();
        logError("Structures that are not 16-byte aligned are not supported by this UBO implementation");
#ifdef DEBUG_MODE
        throw("Error, uniform " + QString(uniform.getName()) +
            " is of type " + QString::fromStdString(tname) + ". This uniform type is not supported: ");
#else
        logError("Error, uniform " + QString(uniform.getName()) +
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
void UBO::populateUniforms(const ShaderBufferInfo& bufferInfo)
{
    m_data.m_uniforms.clear();

    // FIXME: Will not work correctly if there is, for example, a vec3 followed by a float.
    // This will incorrectly add padding for a vec3, assuming a byteSize of 16, when in reality, we will have 12 + 4, with no padding
    // SOLUTION: Do NOT use vec3s or similarly unaligned types in uniform blocks
    m_name = bufferInfo.m_name;
    size_t vec4Size = s_alignedTypeSizes[ShaderVariableType::kVec4];
    m_data.m_byteSize = 0;
    for (const ShaderInputInfo& info: bufferInfo.m_fields) {
        m_data.m_uniforms[info.name()] = BufferUniform(info);
        BufferUniform& uniform = m_data.m_uniforms[info.name()];
        size_t typeSize = s_alignedTypeSizes[info.m_variableType];
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
void UBO::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(json)
    Q_UNUSED(context)
}
/////////////////////////////////////////////////////////////////////////////////////////////
UBO* UBO::s_boundUBO = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////////
tsl::robin_map<QString, std::shared_ptr<UBO>> UBO::s_UBOMap = {};

/////////////////////////////////////////////////////////////////////////////////////////////
/// Scalar e.g. int or bool: 	 Each scalar has a base alignment of N.
/// Vector 	Either 2N or 4N;     This means that a vec3 has a base alignment of 4N.
/// Array of scalars or vectors: Each element has a base alignment equal to that of a vec4.
/// Matrices:                  	 Stored as a large array of column vectors, where each of those vectors has a base alignment of vec4.
/// Struct: 	                 Equal to the computed size of its elements according to the previous rules, but padded to a multiple of the size of a vec4.
tsl::robin_map<ShaderVariableType, size_t> UBO::s_alignedTypeSizes = {
    {ShaderVariableType::kBool,        sizeof(GLfloat)},
    {ShaderVariableType::kInt,         sizeof(GLfloat)},
    {ShaderVariableType::kFloat,       sizeof(GLfloat)},
    {ShaderVariableType::kDouble,      sizeof(GLfloat)},
    {ShaderVariableType::kVec2,        2 * sizeof(GLfloat)},
    {ShaderVariableType::kVec3,        4 * sizeof(GLfloat)},
    {ShaderVariableType::kVec4,        4 * sizeof(GLfloat)},
    {ShaderVariableType::kMat2,        2 * 4 * sizeof(GLfloat)},
    {ShaderVariableType::kMat3,        3 * 4 * sizeof(GLfloat)},
    {ShaderVariableType::kMat4,        4 * 4 * sizeof(GLfloat)},
    {ShaderVariableType::kSamplerCube, sizeof(GLfloat)},
    {ShaderVariableType::kSampler2D,   sizeof(GLfloat)}
};



/////////////////////////////////////////////////////////////////////////////////////////////
// End namespacing
}