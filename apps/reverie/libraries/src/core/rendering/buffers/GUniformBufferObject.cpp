#include "core/rendering/buffers/GUniformBufferObject.h"

// QT

// Internal
#include "fortress/system/memory/GPointerTypes.h"
#include "core/rendering/lighting/GLightSettings.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/renderer/GRenderContext.h"

namespace rev {

BufferUniform::BufferUniform()
{
}

BufferUniform::BufferUniform(const ShaderInputInfo & info, UniformContainer& outContainer):
    m_info(info)
{
    initialize(info, outContainer);
}

void BufferUniform::initialize(const ShaderInputInfo & info, UniformContainer& outContainer)
{
    // Set default value for uniform from info
    m_data.m_uniformData = UniformData{ info.m_variableType, (Uint32_t)(-1)/*index*/, -1/*count*/};
    switch (info.m_variableType) {
    case ShaderVariableType::kInt:
        if (info.isArray()) Logger::Throw("Error, arrays of int not accepted, try array of vec4");
        UniformData::AddValue<int>(m_data.m_uniformData, -1, outContainer);
        break;
    case ShaderVariableType::kBool:
        if (info.isArray()) Logger::Throw("Error, arrays of bool not accepted, try array of vec4");
        UniformData::AddValue<bool>(m_data.m_uniformData, true, outContainer);
        break;
    case ShaderVariableType::kFloat:
        if (info.isArray()) Logger::Throw("Error, arrays of float not accepted, try array of vec4");
        UniformData::AddValue<float>(m_data.m_uniformData, 0.0f, outContainer);
        break;
    case ShaderVariableType::kUVec2:
        UniformData::AddValue<Vector2u>(m_data.m_uniformData, Vector2u(), outContainer);
        break;
    case ShaderVariableType::kVec4:
        if (info.isArray()) {
            Vec4List vec;
            vec.resize(info.m_arraySize);
            UniformData::AddValue<Vec4List>(m_data.m_uniformData, vec, outContainer);
        }
        else {
            UniformData::AddValue<Vector4>(m_data.m_uniformData, Vector4(), outContainer);
        }
        break;
    case ShaderVariableType::kMat4:
        if (info.isArray()) {
            std::vector<Matrix4x4> vec;
            vec.resize(info.m_arraySize);
            UniformData::AddValue<std::vector<Matrix4x4>>(m_data.m_uniformData, vec, outContainer);
        }
        else {
            UniformData::AddValue<Matrix4x4>(m_data.m_uniformData, Matrix4x4(), outContainer);
        }
        break;
    default:
        Logger::Throw("Uniform type is invalid for uniform " + info.name());
        break;
    }
}


std::shared_ptr<Ubo> Ubo::Get(Uint32_t id, Int32_t& outIndex)
{
    auto iter = std::find_if(s_uboList.begin(), s_uboList.end(),
        [id](const auto& ubo) {
            return ubo->getBufferId() == id;
        }
    );

    if (iter == s_uboList.end()) {
        outIndex = -1;
        return nullptr;
    }

    outIndex = iter - s_uboList.begin();
    return *iter;
}

std::shared_ptr<Ubo> Ubo::Get(const char* name)
{
    auto iter = std::find_if(s_uboList.begin(), s_uboList.end(),
        [name](const auto& ubo) {
            return ubo->getName() == name;
        }
    );

    if (iter == s_uboList.end()) {
        return nullptr;
    }

    return *iter;
}


std::shared_ptr<CameraUbo> Ubo::GetCameraBuffer()
{
    static Uint32_t s_bufferId;
    static Int32_t s_bufferIndex{ -1 };
    if (s_bufferIndex < 0) {
        std::shared_ptr<Ubo> ubo = Get(s_cameraBufferName);
        if (ubo) {
            s_bufferId = ubo->getBufferId();
        }
    }
    return std::static_pointer_cast<CameraUbo>(Get(s_bufferId, s_bufferIndex));
}

std::shared_ptr<LightUbo> Ubo::GetLightSettingsBuffer()
{
    static Uint32_t s_bufferId;
    static Int32_t s_bufferIndex{ -1 };
    if (s_bufferIndex < 0) {
        std::shared_ptr<Ubo> ubo = Get(s_lightBufferName);
        if (ubo) {
            s_bufferId = ubo->getBufferId();
        }
    }
    return std::static_pointer_cast<LightUbo>(Get(s_bufferId, s_bufferIndex));
}

std::shared_ptr<Ubo> Ubo::Create(RenderContext& context, const ShaderBufferInfo & ss)
{
    std::shared_ptr<Ubo> ubo;
    if (ss.m_name == s_cameraBufferName) {
        ubo = prot_make_shared<CameraUbo>(context, ss);
    }
    else if (ss.m_name == s_lightBufferName) {
        ubo = prot_make_shared<LightUbo>(context, ss);
    }
    else {
        ubo = prot_make_shared<Ubo>(context, ss);
    }
    ubo->postConstruct();
    s_uboList.push_back(ubo);
    return ubo;
}

void Ubo::ClearUBOs(bool clearCore)
{
    if (clearCore) {
        s_uboList.clear();
    }
    else {
        std::vector<std::shared_ptr<Ubo>> remainingUbos;
        auto endIter = s_uboList.end();
        for (const auto& ubo: s_uboList) {
            if (ubo->isCore())
            {
                remainingUbos.push_back(ubo);
            }
        }
        s_uboList.swap(remainingUbos);
    }
}

Ubo::Ubo(RenderContext& context, const ShaderBufferInfo& shaderStruct):
    m_data({ std::vector<BufferUniform>{}, context.uniformContainer(), 0 })
{
    initialize(shaderStruct);
}


Ubo::~Ubo()
{
    glDeleteBuffers(1, &m_bufferID);
}

void Ubo::initialize(const ShaderBufferInfo & info)
{
    m_name = info.m_name;
    initialize(); // Create the UBO
    populateUniforms(info);
    allocateMemory(m_data.m_byteSize);
    bindToPoint(s_uboList.size());
}

void Ubo::bind()
{
    // Optimization, don't bind in GL if already bound
    if (isBound()) return;

    glBindBuffer(GL_UNIFORM_BUFFER, m_bufferID);
    s_boundUBO = this;
}


void Ubo::release()
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    s_boundUBO = nullptr;
}

bool Ubo::hasBufferUniform(const GStringView& uniformName) const
{
    auto iter = std::find_if(m_data.m_uniforms.begin(), m_data.m_uniforms.end(),
        [uniformName](const BufferUniform& uniform) {
            return uniformName == uniform.m_info.m_name;
        });

    return iter != m_data.m_uniforms.end();
}

const BufferUniformData & Ubo::getBufferUniformData(const GStringView & uniformName) const
{
    auto iter = std::find_if(m_data.m_uniforms.begin(), m_data.m_uniforms.end(),
        [uniformName](const BufferUniform& uniform) {
            return uniformName == uniform.m_info.m_name;
        });

    assert(iter != m_data.m_uniforms.end() && "Error, uniform not found");
    return iter->m_data;
}

BufferUniformData& Ubo::getBufferUniformData(const GStringView& uniformName)
{
    auto iter = std::find_if(m_data.m_uniforms.begin(), m_data.m_uniforms.end(),
        [uniformName](const BufferUniform& uniform) {
            return uniformName == uniform.m_info.m_name;
        });

    assert(iter != m_data.m_uniforms.end() && "Error, uniform not found");
    return iter->m_data;
}

BufferUniformData& Ubo::getBufferUniformDataAtOffset(Uint32_t offset)
{
    auto iter = std::find_if(m_data.m_uniforms.begin(), m_data.m_uniforms.end(),
        [offset](const BufferUniform& uniform) {
            return offset == uniform.m_data.m_offset;
        });

    assert(iter != m_data.m_uniforms.end() && "Error, uniform not found");
    return iter->m_data;
}

const BufferUniformData& Ubo::getBufferUniformDataAtOffset(Uint32_t offset) const
{
    auto iter = std::find_if(m_data.m_uniforms.begin(), m_data.m_uniforms.end(),
        [offset](const BufferUniform& uniform) {
            return offset == uniform.m_data.m_offset;
        });

    assert(iter != m_data.m_uniforms.end() && "Error, uniform not found");
    return iter->m_data;
}


//Uniform UBO::getUniformValueFromBuffer(const GString & uniformName)
//{
//    bind();
//
//    Uniform newUniform;
//    const BufferUniform& bufferUniform = m_data.m_uniforms[uniformName];
//    const Uniform& uniform = bufferUniform.m_uniform;
//    
//    // Get uniform value from GL
//    /// \see https://antongerdelan.net/blog/formatted/2014_06_04_glmapbufferrange.html
//    size_t size = s_alignedTypeSizes[bufferUniform.m_info.m_variableType];
//
//    // Case based on type
//    if (uniform.is<int>()) {
//        int* data = (int*)glMapBufferRange(GL_UNIFORM_BUFFER,
//            bufferUniform.m_data.m_offset,
//            size,
//            GL_MAP_READ_BIT);
//        int count = *data;
//        newUniform.set<int>(count);
//    }
//    else if (uniform.is<bool>()) {
//        int* data = (int*)glMapBufferRange(GL_UNIFORM_BUFFER,
//            bufferUniform.m_data.m_offset,
//            size,
//            GL_MAP_READ_BIT);
//        newUniform.set<bool>(bool(*data));
//    }
//    else if (uniform.is<float>()) {
//        float* data = (float*)glMapBufferRange(GL_UNIFORM_BUFFER,
//            bufferUniform.m_data.m_offset,
//            size,
//            GL_MAP_READ_BIT);
//        newUniform.set<float>(*data);
//    }
//    else if (uniform.is<Vector4f>()) {
//        float* data = (float*)glMapBufferRange(GL_UNIFORM_BUFFER,
//            bufferUniform.m_data.m_offset,
//            size,
//            GL_MAP_READ_BIT);
//        newUniform.set<Vector4f>(Vector4f(data));
//    }
//    else if (uniform.is<Matrix4x4>()) {
//        float* data = (float*)glMapBufferRange(GL_UNIFORM_BUFFER,
//            bufferUniform.m_data.m_offset,
//            size,
//            GL_MAP_READ_BIT);
//        newUniform.set<Matrix4x4g>(Matrix4x4g(data));
//    }
//    else if (uniform.is<std::vector<Matrix4x4>>()) {
//        const std::vector<Matrix4x4g>& value = uniform.get<std::vector<Matrix4x4g>>();
//        float* data = (float*)glMapBufferRange(GL_UNIFORM_BUFFER,
//            bufferUniform.m_data.m_offset,
//            size * value.size(),
//            GL_MAP_READ_BIT);
//        std::vector<Matrix4x4g> mats;
//        for (size_t i = 0; i < value.size() * 16; i += 16) {
//            Vec::EmplaceBack(mats, Matrix4x4g(data + i)); // untested
//        }
//        newUniform.set<std::vector<Matrix4x4g>>(mats);
//    }
//    else if (uniform.is<Vec4List>()) {
//        const Vec4List& value = uniform.get<Vec4List>();
//        float* data = (float*)glMapBufferRange(GL_UNIFORM_BUFFER,
//            bufferUniform.m_data.m_offset,
//            size * value.size(),
//            GL_MAP_READ_BIT);
//        std::vector<float> floatVec(data, data + value.size() * 4);
//        std::vector<Vector4> newVec;
//        for (size_t i = 0; i < floatVec.size(); i += 4) {
//            Vec::EmplaceBack(newVec, Vector4{ floatVec[i], floatVec[i + 1], floatVec[i + 2], floatVec[i + 3] });
//        }
//        //logInfo(bufferUniform.m_uniform.getName() + " ---------------------------------");
//        //for (size_t i = 0; i < value.size(); i++) {
//        //    logInfo(QString(newVec[i]) + ", " + QString(value[i]));
//        //}
//        newUniform.set<Vec4List>(newVec);
//    }
//    else {
//        std::string tname = uniform.typeInfo().name();
//        Logger::LogError("Structures that are not 16-byte aligned are not supported by this UBO implementation");
//#ifdef DEBUG_MODE
//        Logger::Throw("Error, uniform " + bufferUniform.m_info.m_name +
//            " is of type " + tname + ". This uniform type is not supported: ");
//#else
//        Logger::LogError("Error, uniform " + bufferUniform.m_info.m_name +
//            " is of type " + tname + ". This uniform type is not supported: ");
//#endif
//    }
//
//    // Must be called, or things will break (changes won't be pushed to GL)
//    glUnmapBuffer(GL_UNIFORM_BUFFER);
//
//    return newUniform;
//}

void Ubo::setBufferUniformValueFromName(const GStringView& uniformName, const UniformData& uniform)
{
    BufferUniformData& bufferUniformData = getBufferUniformData(uniformName);
    bufferUniformData.m_uniformData = uniform;
    refreshUniform(bufferUniformData);
}

void Ubo::setBufferUniformValue(const BufferUniformData& uniform)
{
    BufferUniformData& bufferUniformData = getBufferUniformDataAtOffset(uniform.m_offset);
    bufferUniformData.m_uniformData = uniform.m_uniformData;
    refreshUniform(bufferUniformData);
}

void Ubo::initialize() {
    // Generate the Uniform Buffer Object
    glGenBuffers(1, &m_bufferID);
}

void Ubo::refreshUniform(const BufferUniformData & bufferUniformData)
{
    bind();

    // Update uniform in openGL
    const UniformData& uniformData = bufferUniformData.m_uniformData;
    switch (uniformData.m_uniformType) {
    case ShaderVariableType::kInt:
        glBufferSubData(GL_UNIFORM_BUFFER, bufferUniformData.m_offset, bufferUniformData.m_alignmentByteSize, &UniformData::GetValue<int>(uniformData, m_data.m_uniformValues));
        break;
    case ShaderVariableType::kBool:
        glBufferSubData(GL_UNIFORM_BUFFER, bufferUniformData.m_offset, bufferUniformData.m_alignmentByteSize, &UniformData::GetValue<bool>(uniformData, m_data.m_uniformValues));
        break;
    case ShaderVariableType::kFloat:
        glBufferSubData(GL_UNIFORM_BUFFER, bufferUniformData.m_offset, bufferUniformData.m_alignmentByteSize, &UniformData::GetValue<float>(uniformData, m_data.m_uniformValues));
        break;
    case ShaderVariableType::kUVec2:
        glBufferSubData(GL_UNIFORM_BUFFER, bufferUniformData.m_offset, bufferUniformData.m_alignmentByteSize, &UniformData::GetValue<Vector2u>(uniformData, m_data.m_uniformValues));
        break;
    case ShaderVariableType::kVec4:
        if (uniformData.m_count < 0) {
            glBufferSubData(GL_UNIFORM_BUFFER, bufferUniformData.m_offset, bufferUniformData.m_alignmentByteSize, &UniformData::GetValue<Vector4f>(uniformData, m_data.m_uniformValues));
        }
        else {
            /// @todo See if alignment lets me get away with a single glBufferSubData call
            const Vector4* value = &UniformData::GetValue<Vector4>(uniformData, m_data.m_uniformValues);
            size_t offset = bufferUniformData.m_offset;
            size_t size = s_alignedTypeSizes[ShaderVariableType::kVec4];
            for (size_t i = 0; i < uniformData.m_count; i++) {
                const Vector4& vec = value[i];
                glBufferSubData(GL_UNIFORM_BUFFER, offset, size, vec.getData());
                offset += size;
            }
        }
        break;
    case ShaderVariableType::kMat4:
        if (uniformData.m_count < 0) {
            glBufferSubData(GL_UNIFORM_BUFFER, bufferUniformData.m_offset, bufferUniformData.m_alignmentByteSize, UniformData::GetValue<Matrix4x4g>(uniformData, m_data.m_uniformValues).getData());
        }
        else {
            /// @todo See if alignment lets me get away with a single glBufferSubData call
            const Matrix4x4* value = &UniformData::GetValue<Matrix4x4>(uniformData, m_data.m_uniformValues);
            size_t offset = bufferUniformData.m_offset;
            size_t size = s_alignedTypeSizes[ShaderVariableType::kMat4];
            for (size_t i = 0; i < uniformData.m_count; i++) {
                const Matrix4x4& mat = value[i];
                glBufferSubData(GL_UNIFORM_BUFFER, offset, size, mat.getData());
                offset += size;
            }
        }
        break;
    default:
        Logger::LogError("Structures that are not 16-byte aligned are not supported by this UBO implementation");
#ifdef DEBUG_MODE
        Logger::Throw("Error, uniform is of unsupported type. This uniform type is not supported: ");
#else
        Logger::LogError("Error, uniform is of type. This uniform type is not supported: ");
#endif
    }
}

void Ubo::allocateMemory(size_t numBytes, int usageType) {
    bind();
    glBufferData(GL_UNIFORM_BUFFER, numBytes, NULL, usageType);
}

void Ubo::bindToPoint(size_t bindingPoint) {
    bind();
    m_bindingPoint = (unsigned int)bindingPoint;
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_bufferID);
}

void Ubo::bindToPoint(size_t bindingPoint, size_t offset, size_t sizeInBytes) {
    bind();
    m_bindingPoint = (unsigned int)bindingPoint;
    glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, m_bufferID, offset, sizeInBytes);
}

void Ubo::populateUniforms(const ShaderBufferInfo& bufferInfo)
{
    m_data.m_uniforms.clear();

    // FIXME: Will not work correctly if there is, for example, a vec3 followed by a float.
    // This will incorrectly add padding for a vec3, assuming a byteSize of 16, when in reality, we will have 12 + 4, with no padding
    // SOLUTION: Do NOT use vec3s or similarly unaligned types in uniform blocks
    m_name = bufferInfo.m_name;
    size_t vec4Size = s_alignedTypeSizes[ShaderVariableType::kVec4];
    m_data.m_byteSize = 0;
    for (const ShaderInputInfo& info: bufferInfo.m_fields) {
        m_data.m_uniforms.push_back(BufferUniform(info, m_data.m_uniformValues));
        BufferUniform& uniform = m_data.m_uniforms.back();
        size_t typeSize = s_alignedTypeSizes[info.m_variableType];
        if (info.isArray()) {
            // Sizing according to std140 spacing
            uniform.m_data.m_alignmentByteSize = vec4Size * info.m_arraySize;
        }
        else {
            uniform.m_data.m_alignmentByteSize = typeSize;
        }

        size_t amountOverAlignment = m_data.m_byteSize % 16;
        if (amountOverAlignment != 0) {
            // The current offset is not a multiple of 16 as required
            size_t requiredPadding = 16 - amountOverAlignment;
            if (uniform.m_data.m_alignmentByteSize > requiredPadding) {
                // Need to pad, since the current uniform size will not pad to a multiple of 16
                m_data.m_byteSize += requiredPadding;
            }
        }
        uniform.m_data.m_offset = m_data.m_byteSize;
        m_data.m_byteSize += uniform.m_data.m_alignmentByteSize;
    }

    // Final check that size is a multiple of 16
    size_t requiredPadding = m_data.m_byteSize % 16 == 0? 0: 16 - (m_data.m_byteSize % 16);
    m_data.m_byteSize += requiredPadding;
}

Ubo* Ubo::s_boundUBO = nullptr;


std::vector<std::shared_ptr<Ubo>> Ubo::s_uboList = {};


/// Scalar e.g. int or bool: 	 Each scalar has a base alignment of N.
/// Vector 	Either 2N or 4N;     This means that a vec3 has a base alignment of 4N.
/// Array of scalars or vectors: Each element has a base alignment equal to that of a vec4.
/// Matrices:                  	 Stored as a large array of column vectors, where each of those vectors has a base alignment of vec4.
/// Struct: 	                 Equal to the computed size of its elements according to the previous rules, but padded to a multiple of the size of a vec4.
tsl::robin_map<ShaderVariableType, size_t> Ubo::s_alignedTypeSizes = {
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


// End namespacing
}