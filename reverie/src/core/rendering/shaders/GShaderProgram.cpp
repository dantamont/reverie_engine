#include "GShaderProgram.h"

// QT
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

// Internal
#include "../../readers/GFileReader.h"
#include "../../GSettings.h"
#include "../buffers/GUniformBufferObject.h"
#include "../lighting/GLightSettings.h"
#include "../lighting/GShadowMap.h"
#include "../lighting/GLightClusterGrid.h"
#include "../../GCoreEngine.h"
#include "../renderer/GRenderContext.h"
#include "../renderer/GMainRenderer.h"

#define GL_GLEXT_PROTOTYPES 

//#define CACHE_UNIFORM_VALUES
#define CHECK_UNIFORM_EXISTENCE

namespace rev {   
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// ShaderProgram
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ShaderProgram::CreateHandle(CoreEngine * engine,
    const QJsonValue & json,
    const GString& shaderName)
{
    auto handle = ResourceHandle::create(engine,
        ResourceType::kShaderProgram);
    //handle->setResourceType(ResourceType::kShaderProgram);
    handle->setRuntimeGenerated(true);

    // Since not using load process, need to do everything here
    // TODO: Use loadprocess
    auto shaderProgram = std::make_unique<ShaderProgram>();
    shaderProgram->loadFromJson(json);
    handle->setName(shaderName);
    handle->setResource(std::move(shaderProgram), false);
    shaderProgram->postConstruction();
    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::isBuiltIn(const GString & name)
{
    auto iter = std::find_if(Shader::s_builtins.begin(),
        Shader::s_builtins.end(),
        [&](const std::pair<GString, Shader::ShaderType>& shaderPair) {
        return shaderPair.first == name;
    });

    return iter != Shader::s_builtins.end();
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(): Resource()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(const ShaderProgram & shaderProgram):
    m_shaders(shaderProgram.m_shaders),
    m_programID(shaderProgram.m_programID),
    m_uniformQueue(shaderProgram.m_uniformQueue),
    m_uniforms(shaderProgram.m_uniforms),
    m_uniformInfo(shaderProgram.m_uniformInfo)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////
//ShaderProgram::ShaderProgram(const QJsonValue & json):
//    Resource()
//{
//    loadFromJson(json);
//}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(const QString& vertfile, const QString& fragfile) :
    Resource()
{
    if (!FileReader::FileExists(vertfile)) {
        throw("Error, vertex shader has invalid filepath and is required to link shader program");
    }
    if (!FileReader::FileExists(fragfile)) {
        throw("Error, fragment shader has invalid filepath and is required to link shader program");
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
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(const QString& compFile) :
    Resource()
{
    if (!FileReader::FileExists(compFile)) {
        throw("Error, compute shader has invalid filepath and is required to link shader program");
    }

    Vec::Emplace(m_shaders, m_shaders.end(), compFile, Shader::ShaderType::kCompute, false);

    // Initialize individual shaders
    for (auto& shader : m_shaders) {
        if (!shader.isValid()) {
            shader.initializeFromSourceFile();
        }
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(const QString & vertfile,
    const QString & fragfile, 
    const QString & geometryFile) :
    Resource()
{
    // Check shader files
    if (!FileReader::FileExists(vertfile)) {
        throw("Error, vertex shader has invalid filepath and is required to link shader program");
    }
    if (!FileReader::FileExists(fragfile)) {
        throw("Error, fragment shader has invalid filepath and is required to link shader program");
    }

    // Create any shaders
    Vec::Emplace(m_shaders, m_shaders.end(), vertfile, Shader::ShaderType::kVertex, true);
    Vec::Emplace(m_shaders, m_shaders.end(), fragfile, Shader::ShaderType::kFragment, true);
    if (!geometryFile.isEmpty()) {
        if (FileReader::FileExists(geometryFile)) {
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
/////////////////////////////////////////////////////////////////////////////////////////////
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
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    gl.glDeleteProgram(m_programID);

#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error deleting shader program");
    if (error) {
        logError("Error deleting shader program");
    }
#endif

}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::isValid() const
{
    bool valid = true;
    for (const Shader& shaderPair : m_shaders) {
        valid &= shaderPair.isValid();
    }
    return valid;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::isBound() const
{
    if (!s_boundShader) return false;
    return handle()->getUuid() == s_boundShader->handle()->getUuid();
}
/////////////////////////////////////////////////////////////////////////////////////////////
const Shader* ShaderProgram::getFragShader() const
{
    return getShader(Shader::ShaderType::kFragment);
}
/////////////////////////////////////////////////////////////////////////////////////////////
const Shader* ShaderProgram::getVertShader() const
{
    return getShader(Shader::ShaderType::kVertex);
}
/////////////////////////////////////////////////////////////////////////////////////////////
const Shader * ShaderProgram::getGeometryShader() const
{
    return getShader(Shader::ShaderType::kGeometry);
}
/////////////////////////////////////////////////////////////////////////////////////////////
const Shader * ShaderProgram::getComputeShader() const
{
    return getShader(Shader::ShaderType::kCompute);
}
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::hasUniform(const GStringView & uniformName) const
{
    UniformInfoIter iter;
    return hasUniform(uniformName, iter, nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::hasUniform(const GStringView& uniformName, UniformInfoIter& outIter, int* localIndex) const
{
    // TODO: Maybe could set up a hash-lookup table to avoid using qstring as the hash
    outIter = m_uniformInfo.find(uniformName.c_str());
    bool hasUniform = outIter != m_uniformInfo.end();
    if (hasUniform) {
        if (localIndex) {
            *localIndex = outIter->second.m_localIndex;
        }
    }
    return hasUniform;
}
/////////////////////////////////////////////////////////////////////////////////////////////
//bool ShaderProgram::hasCachedUniform(const QString & uniformName, std::vector<Uniform>::const_iterator & iter) const
//{
//    iter = std::find_if(m_uniformQueue.begin(), m_uniformQueue.end(),
//        [&](const Uniform& uniform) {
//        return uniform.getName() == uniformName;
//    });
//    return iter != m_uniformQueue.end();
//}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::bind()
{
    // Optimization, don't bind in GL if already bound
    if (isBound()) return;

    // Bind shader program in GL
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    gl.glUseProgram(m_programID);

    // Bind buffers
    //for (GLBuffer* buffer : m_buffers) {
    //    if (buffer) {
    //        buffer->bindToPoint();
    //    }
    //}

    if (m_bufferUsageFlags.testFlag(BufferUsageFlag::kLightBuffer)) {
        // TODO: Pass in context so this isn't necessary
        CoreEngine* engine = m_handle->engine();
        RenderContext& context = engine->mainRenderer()->renderContext();
        ShaderStorageBuffer& lightBuffer = context.lightingSettings().lightBuffers().readBuffer();
        lightBuffer.bindToPoint();
    }

    if (m_bufferUsageFlags.testFlag(BufferUsageFlag::kShadowBuffer)) {
        // TODO: Pass in context so this isn't necessary
        CoreEngine* engine = m_handle->engine();
        RenderContext& context = engine->mainRenderer()->renderContext();
        ShaderStorageBuffer& shadowBuffer = context.lightingSettings().shadowBuffers().readBuffer();
        shadowBuffer.bindToPoint();
    }

    s_boundShader = this;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::release()
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error before releasing shader program");
    if (error) {
        throw("Error before releasing shader program");
    }
#endif

    gl.glUseProgram(0);

    //for (GLBuffer* buffer : m_buffers) {
    //    if (buffer) {
    //        buffer->releaseFromPoint();
    //    }
    //}

    if (m_bufferUsageFlags.testFlag(BufferUsageFlag::kLightBuffer)) {
        // TODO: Pass in context so this isn't necessary
        CoreEngine* engine = m_handle->engine();
        RenderContext& context = engine->mainRenderer()->renderContext();
        ShaderStorageBuffer& lightBuffer = context.lightingSettings().lightBuffers().readBuffer();
        lightBuffer.releaseFromPoint();
    }

    if (m_bufferUsageFlags.testFlag(BufferUsageFlag::kShadowBuffer)) {
        // TODO: Pass in context so this isn't necessary
        CoreEngine* engine = m_handle->engine();
        RenderContext& context = engine->mainRenderer()->renderContext();
        ShaderStorageBuffer& shadowBuffer = context.lightingSettings().shadowBuffers().readBuffer();
        shadowBuffer.releaseFromPoint();
    }

    s_boundShader = nullptr;

#ifdef DEBUG_MODE
    error = gl.printGLError("Error releasing shader program");
    if (error) {
        throw("Error releasing shader program");
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::setUniformValue(const Uniform & uniform, bool updateGL)
{
    m_uniformQueue[m_newUniformCount++] = uniform;
    //Vec::EmplaceBack(m_uniformQueue, uniform);
#ifdef DEBUG_MODE
    if (m_uniformQueue[m_newUniformCount - 1].getName().isEmpty()) {
        throw("Error, empty uniform name");
    }
#endif
    if (updateGL) { 
        updateUniforms();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::postConstruction()
{
    Resource::postConstruction();
    
    // Post-construction was happening too late, UBOs need to be initialized before LightComponent
    // This has been moved to LoadProcess:loadShaderProgram
    // Initialize the shader once its name has been set in handle
    //initializeShaderProgram();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::clearUniforms()
{
    // Need to preserve size of uniforms vector
    size_t size = m_uniforms.size();
    m_uniforms.clear();
    m_uniforms.resize(size);

    m_newUniformCount = 0;
    //m_uniformQueue.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ShaderProgram::asJson(const SerializationContext& context) const
{
    QJsonObject object;

    // Add shaders to json
    QJsonObject shaders;
    for (const auto& shader : m_shaders) {
        shaders.insert(QString::number((size_t)shader.m_type), shader.asJson());
    }
    object.insert("shaders", shaders);

    // Set name
    //object.insert("name", m_name.c_str());

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    const QJsonObject& object = json.toObject();

    // Load shaders from JSON
    QJsonObject shaders = object["shaders"].toObject();
    QVector<QString> keys = shaders.keys().toVector();
    std::vector<QString> sortedKeys(keys.begin(), keys.end());
    std::sort(sortedKeys.begin(), sortedKeys.end(),
        [&](const QString& k1, const QString& k2) {
        return shaders[k1].toInt() < shaders[k2].toInt();
    }); // Sort keys by ascending shader type

    m_shaders.resize(shaders.keys().size());
    for (const auto& key : sortedKeys) {
        //Shader::ShaderType shaderType = Shader::ShaderType(key.toInt());
        QJsonValue shaderJson = object["shaders"].toObject()[key];
        //Shader shader = Shader(shaderJson);
        //m_shaders[(int)shaderType] = shader;
        Vec::Emplace(m_shaders, m_shaders.end(), shaderJson);
    }

    // Initialize shaders in GL
    for (auto& shader : m_shaders) {
        shader.initializeFromSourceFile();
    }

    // Set name
    if (object.contains("name")) {
        m_handle->setName(object["name"].toString());
    }
    else {
        m_handle->setName(createName());
    }

    // Initialize shader program in GL
    //initializeShaderProgram(m_handle->getName());
    
    //// Load uniform values into uniform queue for update
    //QJsonObject uniforms = object["uniforms"].toObject();
    //m_uniformQueue.clear();
    //for (const auto& key : uniforms.keys()) {
    //    QJsonObject uniformJson = uniforms[key].toObject();
    //    Vec::EmplaceBack(m_uniformQueue, uniformJson);
    //}

    // Update uniforms
    //updateUniforms();

}
///////////////////////////////////////////////////////////////////////////////////////////////
//void ShaderProgram::addBuffer(GLBuffer * buffer, const QString& bufferName)
//{
//    int bufferIndex;
//    if (!bufferName.isEmpty()) {
//        if (!hasBufferInfo(bufferName, GL::BufferBlockType::kShaderStorage, &bufferIndex)) {
//            Q_UNUSED(bufferIndex);
//            throw("Error, buffer is not valid for the given shader");
//        }
//    }
//    m_buffers.push_back(buffer);
//}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::dispatchCompute(size_t numGroupsX, size_t numGroupsY, size_t numGroupsZ)
{
    // Update uniforms before dispatch
    updateUniforms();

    // Check that number of work groups is valid
    Vector3i counts = getMaxWorkGroupCounts();
    if (numGroupsX > (size_t)counts.x()) {
        throw("Error, too many X groups specified");
    }
    if (numGroupsY > (size_t)counts.y()) {
        throw("Error, too many Y groups specified");
    }
    if (numGroupsZ > (size_t)counts.z()) {
        throw("Error, too many Z groups specified");
    }

    if (m_shaders.size() > 1 || m_shaders[0].type() != Shader::ShaderType::kCompute) {
        throw("Error, shader program is not a compute shader, cannot dispatch compute");
    }

    // Perform writes from shader
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    gl.glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);

#ifdef DEBUG_MODE
    bool error = gl.printGLError("Failed to dispatch compute");
    if (error) {
        throw("Error, failed to dispatch compute");
    }
#endif

    // Ensure that writes are recognized by other OpenGL operations
    // TODO: Use this for other SSB writes, although those are fine if accessed within same shader
    // See: https://www.khronos.org/opengl/wiki/Memory_Model (incoherent memory access)
    gl.glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

#ifdef DEBUG_MODE
    error = gl.printGLError("Failed at memory barrier");
    if (error) {
        throw("Error, failed at memory barrier");
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
Vector3i ShaderProgram::getMaxWorkGroupCounts()
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();

    Vector3i counts;
    gl.glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &counts[0]);
    gl.glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &counts[1]);
    gl.glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &counts[2]);

    return counts;
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::hasBufferInfo(const QString & name, GL::BufferBlockType type, int* outIndex)
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

/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::updateUniforms(bool ignoreUnrecognized)
{
    QMutexLocker locker(&m_mutex);

    // Return if not constructed
    if (!m_handle->isConstructed()) {
        //return false;
        return;
    }

    // Iterate through queue to update uniforms in local map and open GL
    //std::vector<Uniform>& uniforms = m_uniforms;
    //bool updated = false;

    if (!m_uniformQueue.size()) {
        //return updated;
        return;
    }

    for (size_t i = 0; i < m_newUniformCount; i++) {
        Uniform& uniform = m_uniformQueue[i];

        // Skip if uniform is invalid
        if (uniform.isEmpty()) { 
            Logger::LogWarning("Received empty uniform " + uniform.getName());
            continue; 
        }

        // Check that uniform type matches
        const GStringView& uniformName = uniform.getName();

#ifdef DEBUG_MODE
        if (uniformName.length() == 0) {
            throw("Error, null uniform name");
        }
#endif

        // Skip if uniform is not present
#ifdef CHECK_UNIFORM_EXISTENCE
        int localIndex = -1;
        UniformInfoIter infoIter;
        bool hasName = hasUniform(uniformName, infoIter, &localIndex);
        if (!hasName) {
            if(!ignoreUnrecognized){
#ifdef DEBUG_MODE
                if (m_handle->getName() != "debug_skeleton") {
                    //for (const auto& key_value : m_uniformInfo) {
                    //    keys.push_back(key_value.first);
                    //}
                    std::vector<GStringView> keys;
                    for (const auto& kv : m_uniformInfo) {
                        keys.push_back(kv.first);
                    }
                    // Debug skeleton has some unused uniforms that raise a warning, fix this
                    Logger::LogWarning("Error, uniform not recognized: " + QString(uniformName));
                }
#endif
            }
            continue;
        }
#endif

        // Skip if uniform value is unchanged
#ifdef CACHE_UNIFORM_VALUES
        // This check was actually pretty expensive, so ifdef out for now
        Uniform& prevUniform = uniforms[localIndex];
        if (prevUniform == uniform) {
            continue;
        }
#endif

        const ShaderInputInfo& info = infoIter->second;
        //const std::type_info& typeInfo = uniform.typeInfo();
        //auto name = typeInfo.name();
        bool matchesInfo = uniform.matchesInfo(info);
        if (!matchesInfo) {
            // If uniform is a float and should be an int, was read in incorrectly from Json via QVariant
            //const std::type_index& classType = Uniform::s_uniformGLTypeMap.at(info.m_variableType);
            const std::type_index& classType = info.m_variableCType;
#ifdef DEBUG_MODE
            GString className = classType.name();
            GString uniformClassName = uniform.typeInfo().name();
#endif
            if (uniform.is<float>() && classType == typeid(int)) {
                uniform.set<int>((int)uniform.get<float>());
            }
            else {
                throw("Error, uniform " + QString(uniformName) + " is not the correct type for GLSL");
            }
        }

        // Update uniform in openGL
        if (uniform.is<int>()) {
            setUniformValueGL(uniform.getName(), uniform.get<int>());
        }
        else if (uniform.is<bool>()) {
            setUniformValueGL(uniform.getName(), (int)uniform.get<bool>());
        }
        else if (uniform.is<float>()) {
            setUniformValueGL(uniform.getName(), uniform.get<float>());
        }
        else if (uniform.is<Vector2f>()) {
            setUniformValueGL(uniform.getName(), uniform.get<Vector2f>());
        }
        else if (uniform.is<Vector3f>()) {
            setUniformValueGL(uniform.getName(), uniform.get<Vector3f>());
        }
        else if (uniform.is<Vector4f>()) {
            setUniformValueGL(uniform.getName(), uniform.get<Vector4f>());
        }
        else if (uniform.is<Matrix2x2>()) {
            setUniformValueGL(uniform.getName(), uniform.get<Matrix2x2>());
        }
        else if (uniform.is<Matrix3x3>()) {
            setUniformValueGL(uniform.getName(), uniform.get<Matrix3x3>());
        }
        else if (uniform.is<Matrix4x4>()) {
            setUniformValueGL(uniform.getName(), uniform.get<Matrix4x4>());
        }
        else if (uniform.is<std::vector<Matrix4x4>>()) {
            //const std::vector<Matrix4x4g>& value = uniform.get<std::vector<Matrix4x4g>>();
            setUniformValueGL(uniform.getName(), uniform.get<std::vector<Matrix4x4>>());
        }
        else if (uniform.is<std::vector<real_g>>()) {
            //const std::vector<real_g>& value = uniform.get<std::vector<real_g>>();
            setUniformValueGL(uniform.getName(), uniform.get<std::vector<real_g>>());
        }
        else if (uniform.is<Vec3List>()) {
            //const Vec3List& value = uniform.get<Vec3List>();
            setUniformValueGL(uniform.getName(), uniform.get<Vec3List>());
        }
        else if (uniform.is<Vec4List>()) {
            //const Vec4List& value = uniform.get<Vec4List>();
            setUniformValueGL(uniform.getName(), uniform.get<Vec4List>());
        }
        else {
            std::string tname = uniform.typeInfo().name();
            int id = typeid(std::vector<real_g>).hash_code();
            Q_UNUSED(id);
#ifdef DEBUG_MODE
            throw("Error, uniform " + QString(uniform.getName()) +
                " is of type " + QString::fromStdString(tname) + ". This uniform type is not supported: ");
#else
            logError("Error, uniform " + QString(uniform.getName()) +
                " is of type " + QString::fromStdString(tname)  +". This uniform type is not supported: ");
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
/////////////////////////////////////////////////////////////////////////////////////////////
//void ShaderProgram::setUniforms(const ShaderProgram & program)
//{
//    m_uniformQueue = program.m_uniforms;
//    m_uniformQueue.insert(m_uniformQueue.end(), program.m_uniformQueue.begin(),
//        program.m_uniformQueue.end());
//    updateUniforms();
//}
/////////////////////////////////////////////////////////////////////////////////////////////
// Template specialization for setting uniforms of an array of matrices
template<>
void ShaderProgram::setUniformValueGL(const GStringView& uniformName, const std::vector<Matrix4x4>& value)
{
    // Get ID of start of array
    GLuint uniformID = getUniformID(uniformName);

    // Send to each location in the array
    size_t size = value.size();
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    for (unsigned int i = 0; i < size; i++) {
        // Set uniform value
        gl.glUniformMatrix4fv(uniformID + i, 1, GL_FALSE, value[i].m_mtx[0].data()); // uniform ID, count, transpose, value
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
// Template specialization for setting uniforms of an array of vec3s
template<>
void ShaderProgram::setUniformValueGL(const GStringView& uniformName, const Vec3List & value)
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();

    // Get ID of start of array
    GLuint uniformID = getUniformID(uniformName);

    size_t size = value.size();
    for (unsigned int i = 0; i < size; i++) {
        // Set uniform value
        gl.glUniform3fv(uniformID + i, 1, &value[i][0]);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
template<>
void ShaderProgram::setUniformValueGL(const GStringView& uniformName, const Vec4List & value)
{    
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    //GLuint uniformID = getUniformID(uniformName);
    //m_gl.glUniform3fv(uniformID, value.size(), &value[0][0]); // uniform ID, count, value

    GLuint uniformID = getUniformID(uniformName);

    size_t size = value.size();
    for (unsigned int i = 0; i < size; i++) {
        //char elementName[128];
        //memset(elementName, 0, sizeof(elementName));
        //snprintf(elementName, sizeof(elementName), "%s[%d]", uniformName.toStdString().c_str(), i);

        // Set uniform value
        gl.glUniform4fv(uniformID + i, 1, &value[i][0]);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::bindAttribute(int attribute, const QString & variableName)
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    const char* variableChar = variableName.toStdString().c_str();
    gl.glBindAttribLocation(m_programID, attribute, variableChar);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::initializeShaderProgram()
{
    // Create program, attach shaders, link
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    m_programID = gl.glCreateProgram();

	for (const auto& shader : m_shaders) {
		attachShader(shader);
	}
    gl.glLinkProgram(m_programID);

//#ifdef DEBUG_MODE
//    bool error = m_gl.printGLError(QStringLiteral("Error linking program");
//    if (error) {
//        throw("Error linking program");
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
        QString errorMsg = QString::fromStdString(errorStr);
        Logger::LogError("Failed to link shader program: " + errorMsg);

#ifdef DEBUG_MODE
        throw("ShaderProgram::initializeShaderProgam:: Error, failed to link shader program");
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
    populateUniformBuffers();

    // Bind the shader for use
    bind();

#ifndef QT_NO_DEBUG_OUTPUT
    gl.printGLError("Failed to bind shader");
#endif

    return status;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::attachShader(const Shader & shader)
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    gl.glAttachShader(m_programID, shader.getID());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::detachShader(const Shader & shader)
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    gl.glDetachShader(m_programID, shader.getID());
}
/////////////////////////////////////////////////////////////////////////////////////////////
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
        GString nonArrayGName = GString(nonArrayName);
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
        throw("Error, size mismatch");
    }
#endif

    m_uniforms.resize(m_uniformInfo.size());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::populateUniformBuffers()
{
    // Populate buffer metadata
    getActiveSSBs(m_bufferInfo);

    // Validate light buffer if it is used in the shader
    int lightBufferIndex;
    if (hasBufferInfo(LIGHT_BUFFER_NAME, GL::BufferBlockType::kShaderStorage, &lightBufferIndex)) {
        Q_UNUSED(lightBufferIndex);
        // Set light buffer if it is used in shader
        //CoreEngine* engine = CoreEngine::engines().begin()->second;
        //ShaderStorageBuffer& lightBuffer = engine->mainRenderer()->renderContext().lightingSettings().lightBuffers().readBuffer();
        //m_buffers.push_back(&lightBuffer);
        m_bufferUsageFlags.setFlag(BufferUsageFlag::kLightBuffer, true);
        size_t lightSize = sizeof(Light);
        if (m_bufferInfo[lightBufferIndex].m_bufferDataSize % lightSize != 0) {
            throw("Error, data-size mismatch between Light and SSB");
        }
    }

    // Validate shadow buffer if it is used in the shader
    int shadowBufferIndex;
    if (hasBufferInfo(SHADOW_BUFFER_NAME, GL::BufferBlockType::kShaderStorage, &shadowBufferIndex)) {
        Q_UNUSED(shadowBufferIndex);
        // Set shadow buffer if it is used in shader
        //CoreEngine* engine = CoreEngine::engines().begin()->second;
        //ShaderStorageBuffer& shadowBuffer = engine->mainRenderer()->renderContext().lightingSettings().shadowBuffer();
        //m_buffers.push_back(&shadowBuffer);
        m_bufferUsageFlags.setFlag(BufferUsageFlag::kShadowBuffer, true);
        size_t shadowSize = sizeof(ShadowInfo);
        size_t bufferDataSize = m_bufferInfo[shadowBufferIndex].m_bufferDataSize;
        if (bufferDataSize % shadowSize != 0) {
            throw("Error, data-size mismatch between ShadowInfo and SSBO");
        }
    }

    // Associate with uniform buffers as necessary
    for (const auto& shader : m_shaders) {
        for (const ShaderBufferInfo& ss : shader.m_uniformBuffers) {
            std::shared_ptr<UBO> ubo;
            if (!UBO::get(ss.m_name)) {
                // Create UBO if it has not been initialized by another shader
                ubo = UBO::create(ss);
            }
            else {
                ubo = UBO::s_UBOMap.at(ss.m_name);
            }

            // Unfortunately, UBO must be defined identically in all shaders
            //else {
            //    // Recreate UBO if found with more fields
            //    ubo = UBO::s_UBOMap.at(ss.m_name);
            //    if (ubo->data().m_uniforms.size() > ss.m_fields.size()) {
            //        ubo->initialize(ss);
            //    }
            //}

            // Set as a core UBO if it should not be deleted on scenario reload
            if (isBuiltIn(m_handle->getName())) {
                ubo->setCore(true);
            }
            
            // Add UBO to shader map
            m_uniformBuffers[ss.m_name] = ubo;

            // Bind shader uniform block to the same binding point as the buffer
            bindUniformBlock(ss.m_name);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
size_t ShaderProgram::getUniformBlockIndex(const QString & blockName)
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    size_t index = gl.glGetUniformBlockIndex(m_programID, blockName.toStdString().c_str());
    return index;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::bindUniformBlock(const QString & blockName)
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    size_t blockIndex = getUniformBlockIndex(blockName);
    const std::shared_ptr<UBO>& ubo = m_uniformBuffers.at(blockName);
    gl.glUniformBlockBinding(m_programID, blockIndex, ubo->m_bindingPoint);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::getActiveUniforms(std::vector<ShaderInputInfo>& outInfo)
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    // See: https://www.khronos.org/registry/OpenGL-Refpages/es3.1/html/glGetProgramResource.xhtml
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
            properties.size(),
            &properties[0],
            values.size(),
            NULL, 
            &values[0]);

        // Get the name from the program
        nameData.resize(values[0]); //The length of the name.
        gl.glGetProgramResourceName(m_programID,
            GL_UNIFORM,
            uniform, // index of uniform
            nameData.size(),
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
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::getActiveSSBs(std::vector<ShaderBufferInfo>& outInfo)
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();

    // See: https://www.khronos.org/registry/OpenGL-Refpages/es3.1/html/glGetProgramResource.xhtml
    std::vector<GLchar> nameData(256);
    std::vector<GLenum> properties = {
        GL_NAME_LENGTH,
        GL_BUFFER_BINDING,
        GL_BUFFER_DATA_SIZE, //  If the final member of an active shader storage block is array with no declared size, the minimum buffer size is computed assuming the array was declared as an array with one element
        GL_NUM_ACTIVE_VARIABLES,
        GL_ACTIVE_VARIABLES
    };
    std::vector<GLint> values(properties.size());

    GLint numActiveBuffers = getNumActiveBuffers(GL::BufferBlockType::kShaderStorage);
    for (int ssb = 0; ssb < numActiveBuffers; ++ssb)
    {
        // Get specified properties from program
        gl.glGetProgramResourceiv(m_programID,
            GL_SHADER_STORAGE_BLOCK,
            ssb, // index of shader storage block
            properties.size(),
            &properties[0],
            values.size(),
            NULL,
            &values[0]);

        // Get the name from the program
        nameData.resize(values[0]); //The length of the name.
        gl.glGetProgramResourceName(m_programID,
            GL_SHADER_STORAGE_BLOCK,
            ssb, // index of ssb
            nameData.size(),
            NULL,
            &nameData[0]);
        QString name((char*)&nameData[0]);

        outInfo.push_back(ShaderBufferInfo());
        outInfo.back().m_name = name;
        outInfo.back().m_bufferType = GL::BufferBlockType::kShaderStorage;
        outInfo.back().m_bufferBinding = values[1];
        outInfo.back().m_bufferDataSize = values[2]; 
        outInfo.back().m_numActiveVariables = values[3];
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::getActiveAttributes(std::vector<ShaderInputInfo>& outInfo)
{
    Q_UNUSED(outInfo);
    throw("Error, not implemented");
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();

    // See: https://www.khronos.org/registry/OpenGL-Refpages/es3.1/html/glGetProgramResource.xhtml
    // Of interest, GL_BLOCK_INDEX, GL_OFFSET
    std::vector<GLchar> nameData(256);
    std::vector<GLenum> properties = {
        GL_NAME_LENGTH,
        GL_TYPE,
        GL_ARRAY_SIZE
    };
    std::vector<GLint> values(properties.size());

    GLint numActiveAttributes = getNumActiveAttributes();
    for (int attrib = 0; attrib < numActiveAttributes; ++attrib)
    {
        gl.glGetProgramResourceiv(m_programID,
            GL_PROGRAM_INPUT, // Since attributes are just vertex shader inputs
            attrib, // index of attribute
            properties.size(),
            &properties[0], 
            values.size(), 
            NULL, 
            &values[0]);

        nameData.resize(values[0]); //The length of the name.
        gl.glGetProgramResourceName(m_programID,
            GL_PROGRAM_INPUT, 
            attrib, // index of attribute
            nameData.size(), 
            NULL,
            &nameData[0]);
        std::string name((char*)&nameData[0], nameData.size() - 1);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
GLint ShaderProgram::getNumActiveUniforms()
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    GLint numActiveUniforms = 0;
    gl.glGetProgramInterfaceiv(m_programID,
        GL_UNIFORM,
        GL_ACTIVE_RESOURCES,
        &numActiveUniforms);
    return numActiveUniforms;
}
/////////////////////////////////////////////////////////////////////////////////////////////
GLint ShaderProgram::getNumActiveBuffers(GL::BufferBlockType bufferType)
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    GLint numActiveBuffers = 0;
    gl.glGetProgramInterfaceiv(m_programID,
        (size_t)bufferType,
        GL_ACTIVE_RESOURCES,
        &numActiveBuffers);
    return numActiveBuffers;
}
/////////////////////////////////////////////////////////////////////////////////////////////
GLint ShaderProgram::getNumActiveAttributes()
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    GLint numActiveAttributes = 0;
    gl.glGetProgramInterfaceiv(m_programID,
        GL_PROGRAM_INPUT,
        GL_ACTIVE_RESOURCES,
        &numActiveAttributes);
    return numActiveAttributes;
}


/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram* ShaderProgram::s_boundShader = nullptr;
/////////////////////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////////////////////
// End namespacing
}