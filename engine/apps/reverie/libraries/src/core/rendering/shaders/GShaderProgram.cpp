#include "core/rendering/shaders/GShaderProgram.h"

// QT
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

// Internal
#include "core/readers/GFileReader.h"
#include "core/GSettings.h"
#include "core/rendering/buffers/GUniformBufferObject.h"
#include "core/rendering/lighting/GLightSettings.h"
#include "core/rendering/lighting/GShadowMap.h"
#include "core/rendering/lighting/GLightClusterGrid.h"
#include "core/GCoreEngine.h"
#include "core/rendering/renderer/GRenderContext.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/shaders/GUniformContainer.h"
#include "fortress/system/path/GPath.h"

#define GL_GLEXT_PROTOTYPES 

//#define CACHE_UNIFORM_VALUES
#define CHECK_UNIFORM_EXISTENCE

namespace rev {   

void ShaderProgram::UniformIdMappings::populateMappings(const ShaderProgram& sp)
{
    // Populate texture uniform information
    for (Uint32_t texUsageType = 0; texUsageType < (Uint32_t)TextureUsageType::kMAX_TEXTURE_TYPE; texUsageType++) {
        const GString& name = Texture::GetUniformName(TextureUsageType(texUsageType));
        if (sp.hasUniform(name)) {
            m_textureUniforms[texUsageType] = sp.getUniformId(name);
        }
        else {
            m_textureUniforms[texUsageType] = -1;
        }
    }

    /// @todo Prefix all of these with "g_"
    if (sp.hasUniform(Shader::s_worldMatrixUniformName)) {
        m_worldMatrix = sp.getUniformId(Shader::s_worldMatrixUniformName);
    }
    if (sp.hasUniform("isAnimated")) {
        m_isAnimated = sp.getUniformId("isAnimated");
    }
    if (sp.hasUniform("globalInverseTransform")) {
        m_globalInverseTransform = sp.getUniformId("globalInverseTransform");
    }
    if (sp.hasUniform("inverseBindPoseTransforms")) {
        m_inverseBindPoseTransforms = sp.getUniformId("inverseBindPoseTransforms");
    }
    if (sp.hasUniform("boneTransforms")) {
        m_boneTransforms = sp.getUniformId("boneTransforms");
    }

    if (sp.hasUniform("constantScreenThickness")) {
        m_constantScreenThickness = sp.getUniformId("constantScreenThickness");
    }
    if (sp.hasUniform("lineColor")) {
        m_lineColor = sp.getUniformId("lineColor");
    }
    if (sp.hasUniform("thickness")) {
        m_lineThickness = sp.getUniformId("thickness");
    }
    if (sp.hasUniform("useMiter")) {
        m_useMiter = sp.getUniformId("useMiter");
    }
    if (sp.hasUniform("fadeWithDistance")) {
        m_fadeWithDistance = sp.getUniformId("fadeWithDistance");
    }

    if (sp.hasUniform("color")) {
        m_color = sp.getUniformId("color");
    }
    if (sp.hasUniform("pointSize")) {
        m_pointSize = sp.getUniformId("pointSize");
    }
    if (sp.hasUniform("screenPixelWidth")) {
        m_screenPixelWidth = sp.getUniformId("screenPixelWidth");
    }

    if (sp.hasUniform("g_colorId")) {
        m_colorIdUniform = sp.getUniformId("g_colorId");
    }

    if (sp.hasUniform("material.specularity")) {
        m_materialSpecularity = sp.getUniformId("material.specularity");
    }
    if (sp.hasUniform("material.shininess")) {
        m_materialShininess = sp.getUniformId("material.shininess");
    }

    if (sp.hasUniform("textColor")) {
        m_textColor = sp.getUniformId("textColor");
    }

    if (sp.hasUniform("perspectiveInverseScale")) {
        m_perspectiveInverseScale = sp.getUniformId("perspectiveInverseScale");
    }
    if (sp.hasUniform("scaleWithDistance")) {
        m_scaleWithDistance = sp.getUniformId("scaleWithDistance");
    }
    if (sp.hasUniform("faceCamera")) {
        m_faceCamera = sp.getUniformId("faceCamera");
    }
    if (sp.hasUniform("onTop")) {
        m_onTop = sp.getUniformId("onTop");
    }
    if (sp.hasUniform("texOffset")) {
        m_texOffset = sp.getUniformId("texOffset");
    }
    if (sp.hasUniform("texScale")) {
        m_texScale = sp.getUniformId("texScale");
    }

    if (sp.hasUniform("depthTexture")) {
        m_depthTexture = sp.getUniformId("depthTexture");
    }

    if (sp.hasUniform("cubeTexture")) {
        m_cubeTexture = sp.getUniformId("cubeTexture");
    }
    if (sp.hasUniform("diffuseColor")) {
        m_diffuseColor = sp.getUniformId("diffuseColor");
    }

    if (sp.hasUniform("noiseSize")) {
        m_noiseSize = sp.getUniformId("noiseSize");
    }
    if (sp.hasUniform("offsets")) {
        m_offsets = sp.getUniformId("offsets");
    }
    if (sp.hasUniform("scale")) {
        m_scale = sp.getUniformId("scale");
    }
    if (sp.hasUniform("kernelSize")) {
        m_kernelSize = sp.getUniformId("kernelSize");
    }
    if (sp.hasUniform("bias")) {
        m_bias = sp.getUniformId("bias");
    }
    if (sp.hasUniform("radius")) {
        m_radius = sp.getUniformId("radius");
    }

    if (sp.hasUniform("pointLightIndex")) {
        m_pointLightIndex = sp.getUniformId("pointLightIndex");
    }
}

std::shared_ptr<ResourceHandle> ShaderProgram::CreateHandle(CoreEngine * engine,
    const nlohmann::json& json,
    const GString& shaderName)
{
    auto handle = ResourceHandle::Create(engine, (GResourceType)EResourceType::eShaderProgram);
    //handle->setResourceType(EResourceType::eShaderProgram);
    handle->setRuntimeGenerated(true);

    // Since not using load process, need to do everything here
    // TODO: Use loadprocess
    auto shaderProgram = std::make_unique<ShaderProgram>();
    json.get_to(*shaderProgram);
    handle->setName(shaderName);
    handle->setResource(std::move(shaderProgram), false);
    shaderProgram->postConstruction(ResourcePostConstructionData());
    return handle;
}

bool ShaderProgram::isBuiltIn(const GString & name)
{
    auto iter = std::find_if(Shader::s_builtins.begin(),
        Shader::s_builtins.end(),
        [&](const std::pair<QString, Shader::ShaderType>& shaderPair) {
        return GString(shaderPair.first.toStdString()) == name;
    });

    return iter != Shader::s_builtins.end();
}

ShaderProgram::ShaderProgram(): Resource()
{
}

ShaderProgram::ShaderProgram(const ShaderProgram & shaderProgram):
    m_shaders(shaderProgram.m_shaders),
    m_programID(shaderProgram.m_programID),
    m_uniformQueue(shaderProgram.m_uniformQueue),
    m_uniformInfo(shaderProgram.m_uniformInfo)
{
}

//ShaderProgram::ShaderProgram(const nlohmann::json& json):
//    Resource()
//{
//    loadFromJson(json);
//}

ShaderProgram::ShaderProgram(const QString& vertfile, const QString& fragfile) :
    Resource()
{
    if (!GPath::Exists(vertfile.toStdString().c_str()) && !vertfile.startsWith(":")) {
        Logger::Throw("Error, vertex shader has invalid filepath and is required to link shader program");
    }
    if (!GPath::Exists(fragfile.toStdString().c_str()) && !fragfile.startsWith(":")) {
        Logger::Throw("Error, fragment shader has invalid filepath and is required to link shader program");
    }

    //m_shaders.resize(4);
    Vec::Emplace(m_shaders, m_shaders.end(), vertfile, Shader::ShaderType::kVertex, false);
    Vec::Emplace(m_shaders, m_shaders.end(), fragfile, Shader::ShaderType::kFragment, false);

    // Initialize individual shaders
    for (auto& shader : m_shaders) {
        if (!shader.isValid()) {
            shader.initializeFromSourceFile();
        }
    }

}

ShaderProgram::ShaderProgram(const QString & vertSource,
    const QString & fragSource, 
    double dummy) :
    Resource()
{
    Q_UNUSED(dummy);

    //m_shaders.resize(4);
    Vec::Emplace(m_shaders, m_shaders.end(), Shader::ShaderType::kVertex);
    Vec::Emplace(m_shaders, m_shaders.end(), Shader::ShaderType::kFragment);

    // Initialize individual shaders
    m_shaders[0].initializeFromSource(vertSource);
    m_shaders[1].initializeFromSource(fragSource);

}

ShaderProgram::ShaderProgram(const QString& compFile) :
    Resource()
{
    if (!GPath::Exists(compFile.toStdString().c_str()) && !compFile.startsWith(":")) {
        Logger::Throw("Error, compute shader has invalid filepath and is required to link shader program");
    }

    Vec::Emplace(m_shaders, m_shaders.end(), compFile, Shader::ShaderType::kCompute, false);

    // Initialize individual shaders
    for (auto& shader : m_shaders) {
        if (!shader.isValid()) {
            shader.initializeFromSourceFile();
        }
    }

}

ShaderProgram::ShaderProgram(const QString & vertfile,
    const QString & fragfile, 
    const QString & geometryFile) :
    Resource()
{
    // Check shader files
    if (!GPath::Exists(vertfile.toStdString().c_str()) && !vertfile.startsWith(":")) {
        Logger::Throw("Error, vertex shader has invalid filepath and is required to link shader program");
    }
    if (!GPath::Exists(fragfile.toStdString().c_str()) && !fragfile.startsWith(":")) {
        Logger::Throw("Error, fragment shader has invalid filepath and is required to link shader program");
    }

    // Create any shaders
    Vec::Emplace(m_shaders, m_shaders.end(), vertfile, Shader::ShaderType::kVertex, true);
    Vec::Emplace(m_shaders, m_shaders.end(), fragfile, Shader::ShaderType::kFragment, true);
    if (!geometryFile.isEmpty()) {
        if (GPath::Exists(geometryFile.toStdString().c_str()) && !geometryFile.startsWith(":")) {
            Vec::Emplace(m_shaders, m_shaders.end(), geometryFile, Shader::ShaderType::kGeometry, true);
        }
    }

    // Initialize individual shaders in GL
    for (auto& shader : m_shaders) {
        if (!shader.isValid()) {
            shader.initializeFromSourceFile();
        }
    }

}

ShaderProgram::~ShaderProgram()
{
    // Clear buffers
    //m_buffers.clear();
    m_bufferUsageFlags = 0;

    // Delete shaders linked to the program
    release();

    // Already detached after link
	//for (const auto& shader: m_shaders) {
	//	m_gl.glDetachShader(m_programID, shader.getID());
	//}

    // Moved to shader destructor
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    gl.glDeleteProgram(m_programID);

#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error deleting shader program");
    if (error) {
        Logger::LogError("Error deleting shader program");
    }
#endif

}

bool ShaderProgram::isValid() const
{
    bool valid = true;
    for (const Shader& shaderPair : m_shaders) {
        valid &= shaderPair.isValid();
    }
    return valid;
}

bool ShaderProgram::isBound() const
{
    if (!s_boundShader) return false;
    return handle()->getUuid() == s_boundShader->handle()->getUuid();
}

const Shader* ShaderProgram::getFragShader() const
{
    return getShader(Shader::ShaderType::kFragment);
}

const Shader* ShaderProgram::getVertShader() const
{
    return getShader(Shader::ShaderType::kVertex);
}

const Shader * ShaderProgram::getGeometryShader() const
{
    return getShader(Shader::ShaderType::kGeometry);
}

const Shader * ShaderProgram::getComputeShader() const
{
    return getShader(Shader::ShaderType::kCompute);
}

const Shader * ShaderProgram::getShader(Shader::ShaderType type) const
{
    auto iter = std::find_if(m_shaders.begin(), m_shaders.end(),
        [&](const Shader& shader) {
        return shader.type() == type;
    });
    if (iter != m_shaders.end()) {
        const Shader& shader = *iter;
        return &shader;
    }
    else {
        return nullptr;
    }
}

GString ShaderProgram::createName() const
{
    GString outName = "";
    GString prevShaderName;
    for (const Shader& shaderPair : m_shaders) {
        // Iterate over shaders and combine names if different, otherwise take filename without extension
        const GString& shaderName = shaderPair.getName();
        if (shaderName != prevShaderName) {
            if (!prevShaderName.isEmpty()) {
                outName += "_";
            }
            prevShaderName = shaderName;
            outName += shaderName;
        }
    }

    return outName;
}

bool ShaderProgram::hasUniform(Uint32_t uniformId) const
{
    if (uniformId == -1) {
        return false;
    }

    for (const auto& uniformInfo : m_uniformInfoVec) {
        if (uniformInfo.m_uniformID == uniformId) {
            return true;
        }
    }
    return false;
}

bool ShaderProgram::hasUniform(const GStringView & uniformName) const
{
    UniformInfoIter iter;
    return hasUniform(uniformName, iter, nullptr);
}


bool ShaderProgram::hasUniform(const GStringView& uniformName, UniformInfoIter& outIter, int* localIndex) const
{
    // TODO: Maybe could set up a hash-lookup table to avoid using GStringView as the hash
    outIter = m_uniformInfo.find(uniformName.c_str());
    bool hasUniform = outIter != m_uniformInfo.end();
    if (hasUniform) {
        if (localIndex) {
            *localIndex = outIter->second.m_localIndex;
        }
    }
    return hasUniform;
}

bool ShaderProgram::hasTextureUniform(Uint32_t textureType, Uint32_t& outId) const
{
    outId = m_uniformMappings.m_textureUniforms[textureType];
    return outId != -1;
}

void ShaderProgram::bind() const
{
    // Optimization, don't bind in GL if already bound
    if (isBound()) return;

    // Bind shader program in GL
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    gl.glUseProgram(m_programID);

    // Bind buffers
    //for (GlBuffer* buffer : m_buffers) {
    //    if (buffer) {
    //        buffer->bindToPoint();
    //    }
    //}

    if (m_bufferUsageFlags.testFlag(BufferUsageFlag::kLightBuffer)) {
        // TODO: Pass in context so this isn't necessary
        CoreEngine* engine = m_handle->engine();
        RenderContext& context = engine->openGlRenderer()->renderContext();
        ShaderStorageBufferQueue& bufferQueue = context.lightingSettings().lightBuffers();
        ShaderStorageBuffer& lightBuffer = bufferQueue.readBuffer();
        lightBuffer.bindToPoint();
    }

    if (m_bufferUsageFlags.testFlag(BufferUsageFlag::kShadowBuffer)) {
        // TODO: Pass in context so this isn't necessary
        CoreEngine* engine = m_handle->engine();
        RenderContext& context = engine->openGlRenderer()->renderContext();
        ShaderStorageBufferQueue& bufferQueue = context.lightingSettings().shadowBuffers();
        ShaderStorageBuffer& shadowBuffer = bufferQueue.readBuffer();
        shadowBuffer.bindToPoint();
    }

    s_boundShader = const_cast<ShaderProgram*>(this);
}


void ShaderProgram::release() const
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error before releasing shader program");
    if (error) {
        Logger::Throw("Error before releasing shader program");
    }
#endif

    gl.glUseProgram(0);

    //for (GlBuffer* buffer : m_buffers) {
    //    if (buffer) {
    //        buffer->releaseFromPoint();
    //    }
    //}

    if (m_bufferUsageFlags.testFlag(BufferUsageFlag::kLightBuffer)) {
        // TODO: Pass in context so this isn't necessary
        CoreEngine* engine = m_handle->engine();
        RenderContext& context = engine->openGlRenderer()->renderContext();
        ShaderStorageBuffer& lightBuffer = context.lightingSettings().lightBuffers().readBuffer();
        lightBuffer.releaseFromPoint();
    }

    if (m_bufferUsageFlags.testFlag(BufferUsageFlag::kShadowBuffer)) {
        // TODO: Pass in context so this isn't necessary
        CoreEngine* engine = m_handle->engine();
        RenderContext& context = engine->openGlRenderer()->renderContext();
        ShaderStorageBuffer& shadowBuffer = context.lightingSettings().shadowBuffers().readBuffer();
        shadowBuffer.releaseFromPoint();
    }

    s_boundShader = nullptr;

#ifdef DEBUG_MODE
    error = gl.printGLError("Error releasing shader program");
    if (error) {
        Logger::Throw("Error releasing shader program");
    }
#endif
}

void ShaderProgram::setUniformValue(const Uniform & uniform)
{
#ifdef DEBUG_MODE
    if (uniform.getId() == -1) {
        Logger::Throw("Error, invalid uniform");
    }
#endif

    m_newUniformCount++;
    if (m_uniformQueue.size() < m_newUniformCount) {
        m_uniformQueue.resize(m_newUniformCount);
    }

    // Since the uniformQueue has the same size each frame for the compiled shader, and
    // uniforms automatically allocate storage when they first get a value set, this should 
    // automatically constrain the size of the UniformContainer.
    uint32_t uniformIndex = m_newUniformCount - 1;
    Uniform& currentUniform = m_uniformQueue[uniformIndex];
    currentUniform = uniform;
}

void ShaderProgram::setUniformValue(Uint32_t uniformId, const UniformData& uniformData)
{
#ifdef DEBUG_MODE
    if (uniformId == -1) {
        Logger::Throw("Error, invalid uniform ID");
    }
    if (uniformData.isEmpty()) {
        Logger::Throw("Error, invalid uniform");
    }
#endif

    m_newUniformCount++;
    if (m_uniformQueue.size() < m_newUniformCount) {
        m_uniformQueue.resize(m_newUniformCount);
    }

    // Since the uniformQueue has the same size each frame for the compiled shader, and
    // uniforms automatically allocate storage when they first get a value set, this should 
    // automatically constrain the size of the UniformContainer.
    uint32_t uniformIndex = m_newUniformCount - 1;
    Uniform& currentUniform = m_uniformQueue[uniformIndex];
    currentUniform = Uniform(uniformId, uniformData);
}

void ShaderProgram::postConstruction(const ResourcePostConstructionData& postConstructData)
{
    Resource::postConstruction(postConstructData);
    
    // Post-construction was happening too late, UBOs need to be initialized before LightComponent
    // This has been moved to LoadProcess:loadShaderProgram
    // Initialize the shader once its name has been set in handle
    //initializeShaderProgram();
}

void ShaderProgram::onRemoval(ResourceCache*)
{
}

void ShaderProgram::clearUniforms()
{
    // This function might be unused :(
    m_newUniformCount = 0;
    m_uniformQueue.clear();
}

const GString& ShaderProgram::getUniformName(Uint32_t id) const
{
    return getUniformInfo(id).m_name;
}

const ShaderInputInfo& ShaderProgram::getUniformInfo(Uint32_t id) const
{
    static ShaderInputInfo s_emptyInfo;
    for (const auto& uniformInfo : m_uniformInfoVec) {
        if (uniformInfo.m_uniformID == id) {
            return uniformInfo;
        }
    }

    return s_emptyInfo;
}

GLuint ShaderProgram::getUniformId(const GStringView& uniformName) const
{
    return m_uniformInfo.at(uniformName).m_uniformID;
}

inline GLuint ShaderProgram::getUniformIdGl(const GStringView& uniformName)
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    return gl.glGetUniformLocation(m_programID, uniformName.c_str());
}


void to_json(json& orJson, const ShaderProgram& korObject)
{
      // Add shaders to json
    json shaders;
    for (const auto& shader : korObject.m_shaders) {
        shaders[GString::FromNumber((size_t)shader.type()).c_str()] = shader;
    }
    orJson["shaders"] = shaders;
}

void from_json(const json& korJson, ShaderProgram& orObject)
{
    // Load shaders from JSON
    const json& shaders = korJson["shaders"];
    std::vector<std::string> keys;
    for (const auto& jsonPair : shaders.items()) {
        keys.push_back(jsonPair.key());
    }

    std::vector<std::string> sortedKeys(keys.begin(), keys.end());
    std::sort(sortedKeys.begin(), sortedKeys.end(),
        [&](const std::string& k1, const std::string& k2) {
        return shaders[k1].get<Int32_t>() < shaders[k2].get<Int32_t>();
    }); // Sort keys by ascending shader type

    orObject.m_shaders.resize(sortedKeys.size());
    for (const auto& key : sortedKeys) {
        //Shader::ShaderType shaderType = Shader::ShaderType(key.toInt());
        const json& shaderJson = korJson["shaders"][key];
        Vec::Emplace(orObject.m_shaders, orObject.m_shaders.end(), shaderJson);
    }

    // Initialize shaders in GL
    for (auto& shader : orObject.m_shaders) {
        shader.initializeFromSourceFile();
    }

    // Set name
    if (korJson.contains("name")) {
        orObject.m_handle->setName(korJson[JsonKeys::s_name].get_ref<const std::string&>().c_str());
    }
    else {
        orObject.m_handle->setName(orObject.createName());
    }
}

//void ShaderProgram::addBuffer(GlBuffer * buffer, const QString& bufferName)
//{
//    int bufferIndex;
//    if (!bufferName.isEmpty()) {
//        if (!hasBufferInfo(bufferName, gl::BufferBlockType::kShaderStorage, &bufferIndex)) {
//            Q_UNUSED(bufferIndex);
//            Logger::Throw("Error, buffer is not valid for the given shader");
//        }
//    }
//    m_buffers.push_back(buffer);
//}

void ShaderProgram::dispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
{
    // Check that number of work groups is valid
    Vector3i counts = getMaxWorkGroupCounts();
    if (numGroupsX > (size_t)counts.x()) {
        Logger::Throw("Error, too many X groups specified");
    }
    if (numGroupsY > (size_t)counts.y()) {
        Logger::Throw("Error, too many Y groups specified");
    }
    if (numGroupsZ > (size_t)counts.z()) {
        Logger::Throw("Error, too many Z groups specified");
    }

    if (m_shaders.size() > 1 || m_shaders[0].type() != Shader::ShaderType::kCompute) {
        Logger::Throw("Error, shader program is not a compute shader, cannot dispatch compute");
    }

    // Perform writes from shader
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    gl.glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);

#ifdef DEBUG_MODE
    bool error = gl.printGLError("Failed to dispatch compute");
    if (error) {
        Logger::Throw("Error, failed to dispatch compute");
    }
#endif

    // Ensure that writes are recognized by other OpenGL operations
    // TODO: Use this for other SSB writes, although those are fine if accessed within same shader
    /// \see https://www.khronos.org/opengl/wiki/Memory_Model (incoherent memory access)
    gl.glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

#ifdef DEBUG_MODE
    error = gl.printGLError("Failed at memory barrier");
    if (error) {
        Logger::Throw("Error, failed at memory barrier");
    }
#endif
}


Vector3i ShaderProgram::getMaxWorkGroupCounts()
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();

    Vector3i counts;
    gl.glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &counts[0]);
    gl.glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &counts[1]);
    gl.glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &counts[2]);

    return counts;
}


bool ShaderProgram::hasBufferInfo(const QString & name, gl::BufferBlockType type, int* outIndex)
{
    Q_UNUSED(type)
    auto iter = std::find_if(m_bufferInfo.begin(), m_bufferInfo.end(),
        [&](const ShaderBufferInfo& info) {
        return info.m_name == name;
    });
    if (outIndex) {
        *outIndex = iter - m_bufferInfo.begin();
    }
    return iter != m_bufferInfo.end();
}


void ShaderProgram::updateUniforms(const UniformContainer& uniformContainer, bool ignoreUnrecognized)
{
    QMutexLocker locker(&m_mutex);

    // Return if not constructed
    if (!m_handle->isConstructed()) {
        //return false;
        return;
    }


    if (!m_uniformQueue.size()) {
        //return updated;
        return;
    }

    for (size_t i = 0; i < m_newUniformCount; i++) {
        Uniform& uniform = m_uniformQueue[i];

        // Skip if uniform is invalid
        if (uniform.isEmpty()) { 
            Logger::LogWarning("Received empty uniform " + uniform.getName(*this));
            continue; 
        }

        // Skip if uniform value is unchanged
#ifdef CACHE_UNIFORM_VALUES
        // This check was actually pretty expensive, so ifdef out for now
        Uniform& prevUniform = uniforms[localIndex];
        if (prevUniform == uniform) {
            continue;
        }
#endif

#ifdef DEBUG_MODE
        if (uniform.m_id == -1) {
            Logger::Throw("Uniform has invalid ID");
        }

        const ShaderInputInfo& info = getUniformInfo(uniform.getId());
        bool matchesInfo = uniform.matchesInfo(info);
        if (!matchesInfo) {
            const std::type_index& classType = info.m_variableCType;
            GString uniformName = uniform.getName(*this);
            Logger::Throw("Error, uniform " + uniformName + " is not the correct type for GLSL");
        }
#endif

        // Update uniform in openGL
        switch (uniform.getType()) {
        case ShaderVariableType::kInt:
            setUniformValueGl(uniform.getId(), uniform.getValue<int>(uniformContainer));
            break;
        case ShaderVariableType::kBool:
            setUniformValueGl(uniform.getId(), (int)uniform.getValue<bool>(uniformContainer));
            break;
        case ShaderVariableType::kFloat:
            if (uniform.isArrayType()) {
                setUniformValueGl(uniform.getId(), &uniform.getValue<float>(uniformContainer), uniform.getCount());
            }
            else {
                setUniformValueGl(uniform.getId(), uniform.getValue<float>(uniformContainer));
            }
            break;
        case ShaderVariableType::kVec2:
            setUniformValueGl(uniform.getId(), uniform.getValue<Vector2>(uniformContainer));
            break;
        case ShaderVariableType::kVec3:
            if (uniform.isArrayType()) {
                setUniformValueGl(uniform.getId(), &uniform.getValue<Vector3>(uniformContainer), uniform.getCount());
            }
            else {
                setUniformValueGl(uniform.getId(), uniform.getValue<Vector3>(uniformContainer));
            }
            break;
        case ShaderVariableType::kVec4:
            if (uniform.isArrayType()) {
                setUniformValueGl(uniform.getId(), &uniform.getValue<Vector4>(uniformContainer), uniform.getCount());
            }
            else {
                setUniformValueGl(uniform.getId(), uniform.getValue<Vector4>(uniformContainer));
            }
            break;
        case ShaderVariableType::kMat2:
            setUniformValueGl(uniform.getId(), uniform.getValue<Matrix2x2>(uniformContainer));
            break;
        case ShaderVariableType::kMat3:
            setUniformValueGl(uniform.getId(), uniform.getValue<Matrix3x3>(uniformContainer));
            break;
        case ShaderVariableType::kMat4:
            if (uniform.isArrayType()) {
                setUniformValueGl(uniform.getId(), &uniform.getValue<Matrix4x4>(uniformContainer), uniform.getCount());
            }
            else {
                setUniformValueGl(uniform.getId(), uniform.getValue<Matrix4x4>(uniformContainer));
            }
            break;
        default:
            std::string tname = "UNKNOWN";
#ifdef DEBUG_MODE
            Logger::Throw("Error, uniform " + GString::FromNumber(uniform.getId()) +
                " is of type " + GString(tname) + ". This uniform type is not supported: ");
#else
            Logger::LogError("Error, uniform " + GString(uniform.getName()) +
                " is of type " + GString(tname)  +". This uniform type is not supported: ");
#endif
        }

        // Update uniform in local map
#ifdef CACHE_UNIFORM_VALUES
        prevUniform = std::move(uniform);
#endif
        //updated = true;
    }

    //m_uniformQueue.clear();
    m_newUniformCount = 0;

    //return updated;
}

// Template specialization for setting uniforms of an array of matrices
template<>
void ShaderProgram::setUniformValueGl(const GStringView& uniformName, const std::vector<Matrix4x4>& value)
{
    GLuint uniformId = getUniformId(uniformName);
    setUniformValueGl(uniformId, value.data(), value.size());
}

template<>
void ShaderProgram::setUniformValueGl(Uint32_t uniformId, const Matrix4x4* value, size_t valueCount)
{
    // Send to each location in the array
    /// @todo See if this can be done in one call, instead of a loop
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    for (uint32_t i = 0; i < valueCount; i++) {
        // Set uniform value
        gl.glUniformMatrix4fv(uniformId + i, 1, GL_FALSE, value[i].getData()); // uniform ID, count, transpose, value
    }
}

// Template specialization for setting uniforms of an array of vec3s
template<>
void ShaderProgram::setUniformValueGl(const GStringView& uniformName, const Vec3List & value)
{
    // Get ID of start of array
    GLuint uniformId = getUniformId(uniformName);
    setUniformValueGl(uniformId, value.data(), value.size());
}

template<>
void ShaderProgram::setUniformValueGl(Uint32_t uniformId, const Vector3* value, size_t valueCount)
{
    static gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();

    /// @todo See if this can be done in one call, instead of a loop
    for (uint32_t i = 0; i < valueCount; i++) {
        // Set uniform value
        gl.glUniform3fv(uniformId + i, 1, &value[i][0]);
    }
}

template<>
void ShaderProgram::setUniformValueGl(const GStringView& uniformName, const Vec4List & value)
{    
    GLuint uniformId = getUniformId(uniformName);
    setUniformValueGl(uniformId, value.data(), value.size());
}

template<>
void ShaderProgram::setUniformValueGl(Uint32_t uniformId, const Vector4* value, size_t valueCount)
{
    static gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();

    /// @todo See if this can be done in one call, instead of a loop
    for (uint32_t i = 0; i < valueCount; i++) {
        // Set uniform value
        gl.glUniform4fv(uniformId + i, 1, &value[i][0]);
    }
}

void ShaderProgram::bindAttribute(int attribute, const QString & variableName)
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    const char* variableChar = variableName.toStdString().c_str();
    gl.glBindAttribLocation(m_programID, attribute, variableChar);
}


bool ShaderProgram::initializeShaderProgram(RenderContext& context)
{
    // Create program, attach shaders, link
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    m_programID = gl.glCreateProgram();

	for (const auto& shader : m_shaders) {
		attachShader(shader);
	}
    gl.glLinkProgram(m_programID);

//#ifdef DEBUG_MODE
//    bool error = m_gl.printGLError(QStringLiteral("Error linking program");
//    if (error) {
//        Logger::Throw("Error linking program");
//    }
//#endif

    // Print error if shaders failed to load
    int status;
    gl.glGetProgramiv(m_programID, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        Logger::LogCritical("Caught error, Failed to link shader program");

        GLint maxNumUniformVector4s;
        gl.glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxNumUniformVector4s);

        GLint maxLength = 0;
        gl.glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<char> errorChar(maxLength);
        gl.glGetProgramInfoLog(m_programID, maxLength, &maxLength, errorChar.data());
        std::string errorStr(std::begin(errorChar), std::end(errorChar));
        Logger::LogError("Failed to link shader program: " + errorStr);

#ifdef DEBUG_MODE
        Logger::Throw("ShaderProgram::initializeShaderProgam:: Error, failed to link shader program");
#endif
    }

    // Validate program
    gl.glValidateProgram(m_programID);
    gl.glGetProgramiv(m_programID, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE)
    {
        Logger::LogCritical("Caught error, failed to validate shader program");
        gl.printGLError("Failed to validate shader program");
    }

    // Detach shaders
    for (const auto& shader : m_shaders) {
        detachShader(shader);
    }

    // Populate uniform map with valid keys
    populateUniforms();

    // Populate uniform buffer map
    populateUniformBuffers(context);

    // Bind the shader for use
    bind();

#ifndef QT_NO_DEBUG_OUTPUT
    gl.printGLError("Failed to bind shader");
#endif

    return status;
}

void ShaderProgram::attachShader(const Shader & shader)
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    gl.glAttachShader(m_programID, shader.getID());
}

void ShaderProgram::detachShader(const Shader & shader)
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    gl.glDetachShader(m_programID, shader.getID());
}

void ShaderProgram::populateUniforms()
{
    QMutexLocker locker(&m_mutex);

    // New approach, read directly from OpenGL
    getActiveUniforms(m_uniformInfoVec);

    m_uniformInfo.clear();

    // New uniform info generation approach
    // Must iterate over indices, since vector size can change during iteration
    for (size_t i = 0; i < m_uniformInfoVec.size(); i++) {
        const ShaderInputInfo& info = m_uniformInfoVec[i];

        // Remove index from array name for map indexing
        QString nonArrayName = QString((const char*)info.name()).split("[")[0];

        // Add both non-array and array names to info, both are valid for opengl
        // e.g., arrayOfValues[0] vs. arrayOfValues
        // This loop makes sure to preserve string pointers for use in GStringView
        GString nonArrayGName = nonArrayName.toStdString();
        if (nonArrayGName != info.name()) {
            // Insert into info vector
            m_uniformInfoVec.push_back(info);
            ShaderInputInfo& renamedInfo = m_uniformInfoVec.back();
            renamedInfo.m_name = nonArrayGName;
        }
    }

    //size_t mapSize0 = m_uniformInfo.size();
    for (size_t i = 0; i < m_uniformInfoVec.size(); i++) {
        const ShaderInputInfo& info = m_uniformInfoVec[i];
        m_uniformInfo[info.name()] = info;
    }

#ifdef DEBUG_MODE
    size_t mapSize = m_uniformInfo.size();
    if (mapSize != m_uniformInfoVec.size()) {
        std::vector<GStringView> keys;
        for (const auto& kv : m_uniformInfo) {
            keys.push_back(kv.first);
        }
        Logger::Throw("Error, size mismatch");
    }
#endif

    m_uniformMappings.populateMappings(*this);
}

void ShaderProgram::populateUniformBuffers(RenderContext& context)
{
    // Populate buffer metadata
    getActiveSSBs(m_bufferInfo);

    // Validate light buffer if it is used in the shader
    int lightBufferIndex;
    if (hasBufferInfo(LIGHT_BUFFER_NAME, gl::BufferBlockType::kShaderStorage, &lightBufferIndex)) {
        // Set light buffer if it is used in shader
        //CoreEngine* engine = CoreEngine::engines().begin()->second;
        //ShaderStorageBuffer& lightBuffer = engine->openGlRenderer()->renderContext().lightingSettings().lightBuffers().readBuffer();
        //m_buffers.push_back(&lightBuffer);
        m_bufferUsageFlags.setFlag(BufferUsageFlag::kLightBuffer, true);
        size_t lightSize = sizeof(Light);
        if (m_bufferInfo[lightBufferIndex].m_bufferDataSize % lightSize != 0) {
            Logger::Throw("Error, data-size mismatch between Light and SSB");
        }
    }

    // Validate shadow buffer if it is used in the shader
    int shadowBufferIndex;
    if (hasBufferInfo(SHADOW_BUFFER_NAME, gl::BufferBlockType::kShaderStorage, &shadowBufferIndex)) {
        Q_UNUSED(shadowBufferIndex);
        // Set shadow buffer if it is used in shader
        //CoreEngine* engine = CoreEngine::engines().begin()->second;
        //ShaderStorageBuffer& shadowBuffer = engine->openGlRenderer()->renderContext().lightingSettings().shadowBuffer();
        //m_buffers.push_back(&shadowBuffer);
        m_bufferUsageFlags.setFlag(BufferUsageFlag::kShadowBuffer, true);
        size_t shadowSize = sizeof(ShadowInfo);
        size_t bufferDataSize = m_bufferInfo[shadowBufferIndex].m_bufferDataSize;
        if (bufferDataSize % shadowSize != 0) {
            Logger::Throw("Error, data-size mismatch between ShadowInfo and SSBO");
        }
    }

    // Associate with uniform buffers as necessary
    for (const auto& shader : m_shaders) {
        for (const ShaderBufferInfo& ss : shader.m_uniformBuffers) {
            std::shared_ptr<Ubo> ubo = Ubo::Get(ss.m_name.c_str());
            if (!ubo) {
                // Create UBO if it has not been initialized by another shader
                ubo = Ubo::Create(context, ss);
            }

            // Set as a core UBO if it should not be deleted on scenario reload
            if (isBuiltIn(m_handle->getName())) {
                ubo->setCore(true);
            }
            
            // Add UBO to shader map
            m_uniformBuffers[ss.m_name.c_str()] = ubo;

            // Bind shader uniform block to the same binding point as the buffer
            bindUniformBlock(ss.m_name.c_str());
        }
    }
}

uint32_t ShaderProgram::getUniformBlockIndex(const QString & blockName)
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    uint32_t index = gl.glGetUniformBlockIndex(m_programID, blockName.toStdString().c_str());
    return index;
}

void ShaderProgram::bindUniformBlock(const QString & blockName)
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    uint32_t blockIndex = getUniformBlockIndex(blockName);
    const std::shared_ptr<Ubo>& ubo = m_uniformBuffers.at(blockName);
    gl.glUniformBlockBinding(m_programID, blockIndex, ubo->m_bindingPoint);
}

void ShaderProgram::getActiveUniforms(std::vector<ShaderInputInfo>& outInfo)
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    /// \see https://www.khronos.org/registry/OpenGL-Refpages/es3.1/html/glGetProgramResource.xhtml
    std::vector<GLchar> nameData(256);
    std::vector<GLenum> properties = {
        GL_NAME_LENGTH,
        GL_TYPE,
        GL_ARRAY_SIZE,
        GL_BLOCK_INDEX, // Offset in block or UBO
        GL_LOCATION
    };
    std::vector<GLint> values(properties.size());

    GLint numActiveUniforms = getNumActiveUniforms();
    for (int uniform = 0; uniform < numActiveUniforms; ++uniform)
    {
        // Get specified properties from program
        gl.glGetProgramResourceiv(m_programID,
            GL_UNIFORM,
            uniform, // index of uniform
            (uint32_t)properties.size(),
            &properties[0],
            (uint32_t)values.size(),
            NULL, 
            &values[0]);

        // Get the name from the program
        nameData.resize(values[0]); //The length of the name.
        gl.glGetProgramResourceName(m_programID,
            GL_UNIFORM,
            uniform, // index of uniform
            (uint32_t)nameData.size(),
            NULL,
            &nameData[0]);
        //QString name((char*)&nameData[0]);

        outInfo.push_back(ShaderInputInfo());
        outInfo.back().m_name = (char*)&nameData[0];
        outInfo.back().m_uniformID = values[4];
        outInfo.back().m_localIndex = uniform;
        outInfo.back().m_variableType = ShaderVariableType(values[1]);
        outInfo.back().m_variableCType = Uniform::s_uniformGLTypeMap.at(outInfo.back().m_variableType);
        outInfo.back().m_flags.setFlag(ShaderInputInfo::kIsArray, 
            values[2] < 2 ? false : true);
        outInfo.back().m_flags.setFlag(ShaderInputInfo::kInBlockOrBuffer, values[3] > -1);
        outInfo.back().m_arraySize = values[2];

    }
}

void ShaderProgram::getActiveSSBs(std::vector<ShaderBufferInfo>& outInfo)
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error before retrieving properties of the shader program");
    assert(!error && "Error before retrieving shader properties");
#endif

    /// \note Name length returned from OpenGL contains null character
    /// \see https://www.khronos.org/registry/OpenGL-Refpages/es3.1/html/glGetProgramResource.xhtml
    std::vector<GLchar> nameData(256);
    static constexpr Int32_t s_numProperties = 4;
    static const std::array<GLenum, s_numProperties> s_properties = {
        GL_NAME_LENGTH,
        GL_BUFFER_BINDING,
        GL_BUFFER_DATA_SIZE, //  If the final member of an active shader storage block is array with no declared size, the minimum buffer size is computed assuming the array was declared as an array with one element
        GL_NUM_ACTIVE_VARIABLES,
        //GL_ACTIVE_VARIABLES /// @todo Figure out why this parameter is problematic on ThinkPad. This is valid in OpenGL 4.3+
    };
    std::array<GLint, s_numProperties> values;

    GLint numActiveBuffers = getNumActiveBuffers(gl::BufferBlockType::kShaderStorage);
    GLsizei lengthWritten;
    for (int ssb = 0; ssb < numActiveBuffers; ++ssb)
    {
        // Get specified properties from program
        gl.glGetProgramResourceiv(m_programID,
            GL_SHADER_STORAGE_BLOCK,
            ssb, // index of shader storage block
            (GLsizei)s_numProperties,
            s_properties.data(),
            (GLsizei)s_numProperties,
            &lengthWritten,
            values.data());
#ifdef DEBUG_MODE
        bool getPropertiesError = gl.printGLError("Error retrieving properties of the shader program");
        assert(!getPropertiesError && "Error retrieving shader properties");
        assert(0 != lengthWritten && "Failed to write properties from OpenGL SSBO");
#endif
        // Get the name from the program
        nameData.resize(values.at(0)); //The length of the name.
        gl.glGetProgramResourceName(m_programID,
            GL_SHADER_STORAGE_BLOCK,
            ssb, // index of ssb
            (GLsizei)nameData.size(),
            NULL,
            nameData.data());
        GString name((char*)nameData.data());

        outInfo.emplace_back();
        ShaderBufferInfo& newestInfo = outInfo.back();
        newestInfo.m_name = name;
        newestInfo.m_bufferType = gl::BufferBlockType::kShaderStorage;
        newestInfo.m_bufferBinding = values.at(1);
        newestInfo.m_bufferDataSize = values.at(2);
        newestInfo.m_numActiveVariables = values.at(3);
    }
}

void ShaderProgram::getActiveAttributes(std::vector<ShaderInputInfo>& outInfo)
{
    Q_UNUSED(outInfo);
    Logger::Throw("Error, not implemented");
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();

    /// \see https://www.khronos.org/registry/OpenGL-Refpages/es3.1/html/glGetProgramResource.xhtml
    // Of interest, GL_BLOCK_INDEX, GL_OFFSET
    std::vector<GLchar> nameData(256);
    std::vector<GLenum> properties = {
        GL_NAME_LENGTH,
        GL_TYPE,
        GL_ARRAY_SIZE
    };
    std::vector<GLint> values((uint32_t)properties.size());

    GLint numActiveAttributes = getNumActiveAttributes();
    for (int attrib = 0; attrib < numActiveAttributes; ++attrib)
    {
        gl.glGetProgramResourceiv(m_programID,
            GL_PROGRAM_INPUT, // Since attributes are just vertex shader inputs
            attrib, // index of attribute
            (uint32_t)properties.size(),
            &properties[0], 
            (uint32_t)values.size(),
            NULL, 
            &values[0]);

        nameData.resize(values[0]); //The length of the name.
        gl.glGetProgramResourceName(m_programID,
            GL_PROGRAM_INPUT, 
            attrib, // index of attribute
            (uint32_t)nameData.size(),
            NULL,
            &nameData[0]);
        std::string name((char*)&nameData[0], nameData.size() - 1);
    }
}

GLint ShaderProgram::getNumActiveUniforms()
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    GLint numActiveUniforms = 0;
    gl.glGetProgramInterfaceiv(m_programID,
        GL_UNIFORM,
        GL_ACTIVE_RESOURCES,
        &numActiveUniforms);
    return numActiveUniforms;
}

GLint ShaderProgram::getNumActiveBuffers(gl::BufferBlockType bufferType)
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    GLint numActiveBuffers = 0;
    gl.glGetProgramInterfaceiv(m_programID,
        (size_t)bufferType,
        GL_ACTIVE_RESOURCES,
        &numActiveBuffers);
    return numActiveBuffers;
}

GLint ShaderProgram::getNumActiveAttributes()
{
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    GLint numActiveAttributes = 0;
    gl.glGetProgramInterfaceiv(m_programID,
        GL_PROGRAM_INPUT,
        GL_ACTIVE_RESOURCES,
        &numActiveAttributes);
    return numActiveAttributes;
}


ShaderProgram* ShaderProgram::s_boundShader = nullptr;




// End namespacing
}