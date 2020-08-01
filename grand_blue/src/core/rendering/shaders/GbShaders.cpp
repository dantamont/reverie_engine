#include "GbShaders.h"

// QT
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

// Internal
#include "../../readers/GbFileReader.h"
#include "../../GbSettings.h"
#include "GbUniformBufferObject.h"
#include "../lighting/GbLight.h"
#include "../../GbCoreEngine.h"
#include "../renderer/GbRenderContext.h"
#include "../renderer/GbMainRenderer.h"

#define GL_GLEXT_PROTOTYPES 

namespace Gb {   
/////////////////////////////////////////////////////////////////////////////////////////////
QStringList Shader::Builtins = {
        "simple",
        "text",
        "basic",
        "lines",
        "cubemap",
        "axes",
        "points",
        "debug_skeleton",
        "quad",
        "prepass"
};

/////////////////////////////////////////////////////////////////////////////////////////////
std::unordered_map<Shader::ShaderType, int> Shader::SHADER_TYPE_TO_GL_MAP(
    { {Shader::kVertex, GL_VERTEX_SHADER},
      {Shader::kFragment, GL_FRAGMENT_SHADER}
    }
);

/////////////////////////////////////////////////////////////////////////////////////////////
std::unordered_map<Shader::ShaderType, QString> Shader::SHADER_TYPE_TO_STRING_MAP(
    { {Shader::kVertex, "Vertex"},
      {Shader::kFragment, "Fragment"}
    }
);
/////////////////////////////////////////////////////////////////////////////////////////////
Shader::Shader(const QJsonValue & json)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Shader::Shader(const QString & file, ShaderType type):
    Shader(file, type, false)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
Shader::Shader(const QString& file, ShaderType type, bool deferConstruction):
    Loadable(file),
    m_type(type)
{
    m_name = FileReader::pathToName(m_path, false);
    if(!deferConstruction)
        initializeFromSourceFile();
}

/////////////////////////////////////////////////////////////////////////////////////////////
Shader::~Shader()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Shader::asJson() const
{
    QJsonObject object = Loadable::asJson().toObject();
    object.insert("shaderType", int(m_type));
    object.insert("filePath", m_path);
    object.insert("name", m_name);
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::loadFromJson(const QJsonValue & json)
{
    Loadable::loadFromJson(json);
    const QJsonObject& object = json.toObject();
    m_type = ShaderType(object["shaderType"].toInt());
    if (!object.contains("name")) {
        m_name = FileReader::pathToName(m_path, false);
    }
    else {
        m_name = object["name"].toString();
    }
    initializeFromSourceFile();
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool Shader::initializeFromSourceFile()
{
    // Read in shader source code
    auto fileReader = Gb::FileReader(m_path);
    QStringList fileLines = fileReader.getFileLines();
    if (!fileLines.size()) {
        m_isValid = false;
        return false;
    }
    else {
        m_isValid = true;
    }

    // Prepend header based on GL version
    auto* settings = new Gb::Settings::INISettings();
    int glMajorVersion = settings->getMajorVersion();
    int glMinorVersion = settings->getMinorVersion();
    m_source = "#version " + std::to_string(glMajorVersion) + 
        std::to_string(glMinorVersion) + "0";
    if (settings->getRenderingMode() == Gb::Settings::kGL_ES) {
        m_source += " es";
    }
    m_source += "\n\n";

    // Append source code
    m_source += fileLines.join("\n").toStdString();

    // Load shader in open GL
    m_shaderID = m_gl.glCreateShader(SHADER_TYPE_TO_GL_MAP.at(m_type));

    const char* sourceCStr = m_source.c_str();
    m_gl.glShaderSource(m_shaderID,
        1, 
        &sourceCStr,
        NULL);

    m_gl.glCompileShader(m_shaderID);

    // Check that shader compiled correctly
    int status;
    m_gl.glGetShaderiv(m_shaderID, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        // If not compiled correctly, print info log
        GLint maxLength = 0;
        m_gl.glGetShaderiv(m_shaderID, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<char> errorChar(maxLength);
        m_gl.glGetShaderInfoLog(m_shaderID, maxLength, &maxLength, errorChar.data());
        std::string errorStr(std::begin(errorChar), std::end(errorChar));
        QString errorMsg = QString::fromStdString(errorStr);
        logCritical("Error, could not compile " + SHADER_TYPE_TO_STRING_MAP.at(m_type) + " shader");
        logCritical(errorMsg);
    }
#ifndef QT_NO_DEBUG_OUTPUT
    m_gl.printGLError("Error initializing shader");
#endif

    // Remove comments from the source
    removeComments(m_source);

    // Parse source for all info
    parseForDefines();
    parseForUniforms();
    parseForBuffers();

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::parseForBuffers()
{
    // See: https://regexr.com/
    QString source = QString::fromStdString(m_source);

    // Parse for struct sub-strings
    QRegularExpression re(BUFFER_REGEX);
    QRegularExpressionMatchIterator i = re.globalMatch(source);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        parseBuffer(match);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::parseBuffer(const QRegularExpressionMatch & match)
{
    static QStringList MISALIGNED_TYPES = { "vec2", "vec3", "mat2", "mat3" };
    m_uniformBuffers.push_back(ShaderStruct());
    ShaderStruct& ss = m_uniformBuffers.back();

    QString structStr = match.captured(0);
    QRegularExpression uniformLine("([\\w]*p[\\s\\t]+)?([\\w\\d]+)[\\s\\t]*([\\w\\d]+)[\\s\\t]*(?:\\[([\\w\\d]+)\\])?(?=[;])");
    QRegularExpressionMatchIterator i = uniformLine.globalMatch(structStr);
    QStringList uniformLines;

    // Struct name
    ss.m_name = match.captured(1);
    while (i.hasNext()) {
        QRegularExpressionMatch lineMatch = i.next();

        QString uniformName = lineMatch.captured(3);// Name
        QString uniformTypeStr = lineMatch.captured(2); // Type 
        bool isArray = !lineMatch.captured(4).isNull();// Is array
        int arraySize;

        if (MISALIGNED_TYPES.contains(uniformTypeStr)) {
            throw("Error, type is misaligned for std140, which is not supported by UBOs");
        }

        // Array size
        if (isArray) {
            QString sizeStr = lineMatch.captured(4);
            if (Map::HasKey(m_defines, sizeStr)) {
                arraySize = m_defines.at(sizeStr);
            }
            else {
                bool ok;
                arraySize = sizeStr.toInt(&ok);
                if (!ok) throw("Error, array size is invalid");
            }
        }

        // Precision
        ShaderInputInfo::PrecisionQualifier precision;
        if (!lineMatch.captured(1).isNull()) {
            QString precisionStr = lineMatch.captured(1);
            if (precisionStr.contains("high"))
                precision = ShaderInputInfo::PrecisionQualifier::kHigh;
            else if (precisionStr.contains("medium"))
                precision = ShaderInputInfo::PrecisionQualifier::kMedium;
            else
                precision = ShaderInputInfo::PrecisionQualifier::kLow;
        }
        else {
            precision = ShaderInputInfo::PrecisionQualifier::kNone;
        }

        ShaderInputType uniformType = Uniform::UNIFORM_TYPE_STR_MAP[uniformTypeStr];
        Vec::EmplaceBack(ss.m_fields, uniformName, uniformType, isArray);
        ss.m_fields.back().m_arraySize = arraySize;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::removeComments(QString& str)
{
    str.remove(QRegularExpression(COMMENT_REGEX));
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::removeComments(std::string & str)
{
    QString qstr = QString::fromStdString(str);
    removeComments(qstr);
    str = qstr.toStdString();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::parseForUniforms()
{
    // See: https://regexr.com/
    QString source = QString::fromStdString(m_source);

    // Search for all shader-defined structs
    parseForStructs();

    QRegularExpression uniform(UNIFORM_REGEX);
    QRegularExpressionMatchIterator i = uniform.globalMatch(source);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        parseUniform(match);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::parseUniform(const QRegularExpressionMatch & match)
{
    QString uniformName = match.captured(3);// Name
    QString uniformTypeStr = match.captured(2); // Type 
    bool isArray = !match.captured(4).isNull();// Is array

    // Precision
    ShaderInputInfo::PrecisionQualifier precision;
    if (!match.captured(1).isNull()) {
        QString precisionStr = match.captured(1);
        if (precisionStr.contains("high"))
            precision = ShaderInputInfo::PrecisionQualifier::kHigh;
        else if (precisionStr.contains("medium"))
            precision = ShaderInputInfo::PrecisionQualifier::kMedium;
        else
            precision = ShaderInputInfo::PrecisionQualifier::kLow;
    }
    else {
        precision = ShaderInputInfo::PrecisionQualifier::kNone;
    }

    // Check if uniform is a struct or not
    auto iter = std::find_if(m_structs.begin(), m_structs.end(),
        [&](const ShaderStruct& s) {return s.m_name == uniformTypeStr; });
    bool isStruct = iter != m_structs.end();
    if (isStruct) {
        // Add all the uniforms in the struct to the uniform list
        for (const ShaderInputInfo& field : iter->m_fields) {
            QString combinedName = uniformName + "." + field.m_name;
            Vec::EmplaceBack(m_uniforms, std::move(combinedName), field.m_inputType, field.isArray());
        }
    }
    else {
        // Add as a normal uniform
        ShaderInputType uniformType = Uniform::UNIFORM_TYPE_STR_MAP[uniformTypeStr];
        Vec::EmplaceBack(m_uniforms, uniformName, uniformType, isArray);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::parseForDefines()
{
    // See: https://regexr.com/
    QString source = QString::fromStdString(m_source);

    QRegularExpression re(DEFINE_REGEX);
    QRegularExpressionMatchIterator i = re.globalMatch(source);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        parseDefine(match);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::parseDefine(const QRegularExpressionMatch & match)
{
    m_defines[match.captured(1)] = match.captured(2).toInt();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::parseForStructs()
{
    // See: https://regexr.com/
    QString source = QString::fromStdString(m_source);

    // Parse for struct sub-strings
    QRegularExpression re(STRUCT_REGEX);
    QRegularExpressionMatchIterator i = re.globalMatch(source);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        parseStruct(match);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::parseStruct(const QRegularExpressionMatch& match)
{
    m_structs.push_back(ShaderStruct());
    ShaderStruct& ss = m_structs.back();

    QString structStr = match.captured(0);
    QRegularExpression uniformLine("([\\w]*p[\\s\\t]+)?([\\w\\d]+)[\\s\\t]*([\\w\\d]+)[\\s\\t]*(?:\\[([\\w\\d]+)\\])?(?=[;])");
    QRegularExpressionMatchIterator i = uniformLine.globalMatch(structStr);
    QStringList uniformLines;

    // Struct name
    ss.m_name = match.captured(1);
    
    while (i.hasNext()) {
        QRegularExpressionMatch lineMatch = i.next();
        
        QString uniformName = lineMatch.captured(3);// Name
        QString uniformTypeStr = lineMatch.captured(2); // Type 
        ShaderInputType uniformType = Uniform::UNIFORM_TYPE_STR_MAP[uniformTypeStr];
        bool isArray = !lineMatch.captured(4).isNull();// Is array
        size_t arraySize = -1;

        // Array size
        if (isArray) {
            QString sizeStr = lineMatch.captured(4);
            if (Map::HasKey(m_defines, sizeStr)) {
                arraySize = m_defines.at(sizeStr);
            }
            else {
                bool ok;
                arraySize = sizeStr.toInt(&ok);
                if (!ok) throw("Error, array size is invalid");
            }
        }

        // Precision
        ShaderInputInfo::PrecisionQualifier precision;
        if (!lineMatch.captured(1).isNull()) {
            QString precisionStr = lineMatch.captured(1);
            if (precisionStr.contains("high")) 
                precision = ShaderInputInfo::PrecisionQualifier::kHigh;
            else if (precisionStr.contains("medium"))
                precision = ShaderInputInfo::PrecisionQualifier::kMedium;
            else
                precision = ShaderInputInfo::PrecisionQualifier::kLow;
        }
        else {
            precision = ShaderInputInfo::PrecisionQualifier::kNone;
        }

        Vec::EmplaceBack(ss.m_fields, uniformName, uniformType, isArray);
        ss.m_fields.back().m_arraySize = arraySize;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::UNIFORM_REGEX = QStringLiteral("uniform[\\s\\t\\r\\n]*([\\w]*p[\\s\\t\\r\\n]+)?([\\w\\d]+)[\\s\\t\\r\\n]*([\\w\\d]+)[\\s\\t\\r\\n]*(\\[[\\w\\d]+\\])?(?=[;])");

/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::BUFFER_REGEX = QStringLiteral("layout[\\s\\t\\n\\r]*\\([\\w\\d]+\\)[\\s\\t\\n\\r]*uniform[\\s\\t\\n\\r]+([\\w\\d]+)[\\s\\t\\n\\r]*[\\s\\t\\n\\r]*\\{([\\w\\d\\s\\t\\n\\r;\\/\\,\\[\\]]*)\\}");

/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::STRUCT_REGEX = QStringLiteral("struct[\\s\\t\\n\\r]*([\\w\\d]+)[\\s\\t\\n\\r]*\\{([\\w\\s\\d\\t\\n\\r;\\/\\,\\.\\[\\]]*)\\}");

/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::COMMENT_REGEX = QStringLiteral("\\/\\/(.)*(?=[\\r\\n])");

/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::DEFINE_REGEX = QStringLiteral("#[\\s\\t\\r\\n]*define[\\s\\t\\r\\n]*([\\w\\d]+)[\\s\\t\\r\\n]*([\\w\\d]+)");




/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// ShaderProgram
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ShaderProgram::createHandle(CoreEngine * engine,
    const QJsonValue & json)
{
    auto handle = ResourceHandle::create(engine,
        Resource::kShaderProgram);
    //handle->setResourceType(Resource::kShaderProgram);
    handle->setUserGenerated(true);

    auto shaderProgram = std::make_shared<ShaderProgram>(json);
    handle->setName(shaderProgram->getName());
    handle->setResource(shaderProgram, false);
    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(): Resource(kShaderProgram)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(const ShaderProgram & shaderProgram):
    Resource(kShaderProgram),
    m_shaders(shaderProgram.m_shaders),
    m_programID(shaderProgram.m_programID),
    m_gl(shaderProgram.m_gl),
    m_uniformQueue(shaderProgram.m_uniformQueue),
    m_uniforms(shaderProgram.m_uniforms),
    m_uniformInfo(shaderProgram.m_uniformInfo)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(const QJsonValue & json):
    Resource(kShaderProgram)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(const QString& vertfile, const QString& fragfile) :
    Resource(kShaderProgram)
{
    //m_shaders.resize(4);
    Vec::Emplace(m_shaders, m_shaders.end(), vertfile, Shader::kVertex, false);
    Vec::Emplace(m_shaders, m_shaders.end(), fragfile, Shader::kFragment, false);

    // Initialize individual shaders
    for (auto& shader : m_shaders) {
        if (!shader.isValid()) {
            shader.initializeFromSourceFile();
        }
    }

    setName();

    initializeShaderProgram();
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::~ShaderProgram()
{
    // Delete shaders linked to the program
    release();
	for (const auto& shader: m_shaders) {
		m_gl.glDetachShader(m_programID, shader.getID());
	}
	for (const auto& shaderPair : m_shaders) {
        m_gl.glDeleteShader(shaderPair.getID());
	}
    m_gl.glDeleteProgram(m_programID);
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
const Shader* ShaderProgram::getFragShader() const
{
    size_t fragIndex = (size_t)Shader::kFragment;
    if (m_shaders.size() > fragIndex) {
        return &m_shaders.at(fragIndex);
    }
    else {
        return nullptr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
const Shader* ShaderProgram::getVertShader() const
{
    size_t vertIndex = (size_t)Shader::kVertex;
    if (m_shaders.size() > vertIndex) {
        return &m_shaders.at(vertIndex);
    }
    else {
        return nullptr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::setName()
{
    m_name = "";
    QString prevShaderName;
    for (const Shader& shaderPair : m_shaders) {
        // Iterate over shaders and combine names if different, otherwise take filename without extension
        QString shaderName = shaderPair.getName();
        if (shaderName != prevShaderName) {
            if (!prevShaderName.isEmpty()) {
                m_name += "_";
            }
            prevShaderName = shaderName;
            m_name += shaderName;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::hasUniform(const QString& uniformName, int* localIndex) const
{
    bool hasUniform = Map::HasKey(m_uniformInfo, uniformName);
    if (hasUniform) {
        if (localIndex) {
            *localIndex = m_uniformInfo.at(uniformName).m_localIndex;
        }
    }
//    else {
//#ifdef DEBUG_MODE
//        logWarning("Shader does not have the specified uniform");
//#endif
//    }
    return hasUniform;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::hasCachedUniform(const QString & uniformName, std::vector<Uniform>::const_iterator & iter) const
{
    iter = std::find_if(m_uniformQueue.begin(), m_uniformQueue.end(),
        [&](const Uniform& uniform) {
        return uniform.getName() == uniformName;
    });
    return iter != m_uniformQueue.end();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::bind()
{
    // Optimization, don't bind in GL if already bound
    if (isBound()) return;

    // Bind shader program in GL
    m_gl.glUseProgram(m_programID);

    // Bind SSBs
    if (m_lightBuffer) {
        m_lightBuffer->bindToPoint(0);
    }

    s_boundShader = this;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::release()
{
    m_gl.glUseProgram(0);

    if (m_lightBuffer) {
        m_lightBuffer->releaseFromPoint(0);
    }

    s_boundShader = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::postConstruction()
{
    Resource::postConstruction();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::clearUniforms()
{
    // Need to preserve size of uniforms vector
    size_t size = m_uniforms.size();
    m_uniforms.clear();
    m_uniforms.resize(size);

    m_uniformQueue.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ShaderProgram::asJson() const
{
    QJsonObject object;

    // Add shaders to json
    QJsonObject shaders;
    for (const auto& shader : m_shaders) {
        shaders.insert(QString::number(shader.m_type), shader.asJson());
    }
    object.insert("shaders", shaders);

    // Set name
    object.insert("name", m_name);

    // Uniforms blow up JSON size, and don't need to persist
    //// Add current uniform values to JSON
    //QJsonObject uniforms;
    //for (const auto& uniformPair : m_uniforms) {
    //    if (uniformPair.second.isPersistent()) {
    //        if (uniformPair.second.valid()) {
    //            // Save all valid uniforms to JSON
    //            uniforms.insert(uniformPair.second.getName(), uniformPair.second.asJson());
    //        }
    //    }
    //}
    //object.insert("uniforms", uniforms);

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();

    // Load shaders from JSON
    QJsonObject shaders = object["shaders"].toObject();
    m_shaders.resize(shaders.keys().size());
    for (const auto& key : shaders.keys()) {
        Shader::ShaderType shaderType = Shader::ShaderType(key.toInt());
        QJsonValue shaderJson = object["shaders"].toObject()[key];
        Shader shader = Shader(shaderJson);
        m_shaders[(int)shaderType] = shader;
    }

    // Set name
    if (object.contains("name")) {
        m_name = object["name"].toString();
    }
    else {
        setName();
    }

    // Initialize shader program in GL
    initializeShaderProgram();
    
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
/////////////////////////////////////////////////////////////////////////////////////////////
GLuint ShaderProgram::getUniformID(const QString & uniformName)
{
    int id = m_uniformInfo.at(uniformName).m_uniformID;

    // If uniform name found in uniform info (will be found unless is an array uniform)
    //if (id < 0) {
    //    // If id not assigned, assign
    //    id = m_gl.glGetUniformLocation(m_programID, uniformName.toStdString().c_str());
    //    if (id > 0) {
    //        m_uniformInfo.at(uniformName).m_uniformID = id;
    //    }
    //}
    return id;
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::hasBuffer(const QString & name, GL::BufferType type, int* outIndex)
{
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
bool ShaderProgram::updateUniforms()
{
    QMutexLocker locker(&m_mutex);

    // Return if not constructed
    if (!m_handle->isConstructed()) return false;

    // Iterate through queue to update uniforms in local map and open GL
    std::vector<Uniform>& uniforms = m_uniforms;
    bool updated = false;

    if (!m_uniformQueue.size()) {
        return updated;
    }

    for (Uniform& uniform: m_uniformQueue) {

        // Skip if uniform is invalid
        if (!uniform.isValid()) continue;

        // Check that uniform type matches
        const QString& uniformName = uniform.getName();

        // Skip if uniform is not present
        int localIndex = -1;
        bool hasName = hasUniform(uniformName, &localIndex);
        if (localIndex < 0) {
#ifdef DEBUG_MODE
            if (m_name != QStringLiteral("debug_skeleton")) {
                // Debug skeleton has some unused uniforms that raise a warning, fix this
                logWarning("Error, uniform not recognized: " + uniformName);
            }
#endif
            continue;
        }

        // Skip if uniform value is unchanged
        //if (Map::HasKey(uniforms, uniformName)) {
        const Uniform& prevUniform = uniforms[localIndex];
        if (prevUniform == uniform) {
            //logError(uniform.getName());
            //logWarning("prev uniform: " + QString(prevUniform));
            //logWarning("new uniform: " + QString(uniform));
            //logError("----");
            continue;
        }
        //}

        ShaderInputInfo& info = m_uniformInfo.at(uniformName);
        //const std::type_info& typeInfo = uniform.typeInfo();
        //auto name = typeInfo.name();
        bool matchesInfo = uniform.matchesInfo(info);
        if (!matchesInfo) {
            // TODO: Implement a more performant fix for this if speed becomes an issue
            // If uniform is a float and should be an int, was read in incorrectly
            // from Json via QVariant
            const std::type_index& classType = Uniform::UNIFORM_GL_TYPE_MAP.at(info.m_inputType);
            if (uniform.is<float>() && classType == typeid(int)) {
                uniform.set<int>((int)uniform.get<float>());
            }
            else {
                throw("Error, uniform " + uniformName + " is not the correct type for GLSL");
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
        else if (uniform.is<Matrix2x2f>()) {
            setUniformValueGL(uniform.getName(), uniform.get<Matrix2x2f>());
        }
        else if (uniform.is<Matrix3x3f>()) {
            setUniformValueGL(uniform.getName(), uniform.get<Matrix3x3f>());
        }
        else if (uniform.is<Matrix4x4f>()) {
            setUniformValueGL(uniform.getName(), uniform.get<Matrix4x4f>());
        }
        else if (uniform.is<std::vector<Matrix4x4f>>()) {
            const std::vector<Matrix4x4g>& value = uniform.get<std::vector<Matrix4x4g>>();
            Q_UNUSED(value);
            setUniformValueGL(uniform.getName(), uniform.get<std::vector<Matrix4x4f>>());
        }
        else if (uniform.is<std::vector<real_g>>()) {
            const std::vector<real_g>& value = uniform.get<std::vector<real_g>>();
            Q_UNUSED(value);
            setUniformValueGL(uniform.getName(), uniform.get<std::vector<real_g>>());
        }
        else if (uniform.is<Vec3List>()) {
            const Vec3List& value = uniform.get<Vec3List>();
            Q_UNUSED(value);
            setUniformValueGL(uniform.getName(), uniform.get<Vec3List>());
        }
        else if (uniform.is<Vec4List>()) {
            const Vec4List& value = uniform.get<Vec4List>();
            Q_UNUSED(value);
            setUniformValueGL(uniform.getName(), uniform.get<Vec4List>());
        }
        else {
            std::string tname = uniform.typeInfo().name();
            int id = typeid(std::vector<real_g>).hash_code();
            Q_UNUSED(id);
#ifdef DEBUG_MODE
            throw("Error, uniform " + uniform.getName() +
                " is of type " + QString::fromStdString(tname) + ". This uniform type is not supported: ");
#else
            logError("Error, uniform " + uniform.getName() +
                " is of type " + QString::fromStdString(tname)  +". This uniform type is not supported: ");
#endif
        }

        // Update uniform in local map
        uniforms[localIndex] = std::move(uniform);
        updated = true;
    }

    // Print error if failed to set uniform
//#ifdef DEBUG_MODE
//    bool hadError = m_gl.printGLError("Error setting uniform ");
//    if (hadError) {
//        logError("Error occured setting uniform");
//    }
//#endif

    m_uniformQueue.clear();

    return updated;
}
/////////////////////////////////////////////////////////////////////////////////////////////
// Template specialization for setting uniforms of an array of matrices
template<>
void ShaderProgram::setUniformValueGL(const QString& uniformName, const std::vector<Matrix4x4f>& value)
{
    // Old approach, doesn't work
    //GLuint uniformID = getUniformID(uniformName);
    //m_gl.glUniformMatrix4fv(uniformID, value.size(), GL_FALSE, value[0].m_mtx[0].data()); // uniform ID, count, transpose, value

    // Get ID of start of array
    GLuint uniformID = getUniformID(QString(uniformName));

    // Send to each location in the array
    // TODO: Optimize, don't use string for each uniform
    size_t size = value.size();
    for (unsigned int i = 0; i < size; i++) {
        //char elementName[128];
        //memset(elementName, 0, sizeof(elementName));
        //snprintf(elementName, sizeof(elementName), "%s[%d]", uniformName.toStdString().c_str(), i);

        // Set uniform value
        m_gl.glUniformMatrix4fv(uniformID + i, 1, GL_FALSE, value[i].m_mtx[0].data()); // uniform ID, count, transpose, value
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
// Template specialization for setting uniforms of an array of vec3s
template<>
void ShaderProgram::setUniformValueGL(const QString & uniformName, const Vec3List & value)
{
    //GLuint uniformID = getUniformID(uniformName);
    //m_gl.glUniform3fv(uniformID, value.size(), &value[0][0]); // uniform ID, count, value

    // Get ID of start of array
    GLuint uniformID = getUniformID(QString(uniformName));

    size_t size = value.size();
    for (unsigned int i = 0; i < size; i++) {
        //char elementName[128];
        //memset(elementName, 0, sizeof(elementName));
        //snprintf(elementName, sizeof(elementName), "%s[%d]", uniformName.toStdString().c_str(), i);

        // Set uniform value
        //GLuint uniformID = getUniformID(QString(elementName));
        m_gl.glUniform3fv(uniformID + i, 1, &value[i][0]);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
template<>
void ShaderProgram::setUniformValueGL(const QString & uniformName, const Vec4List & value)
{    
    //GLuint uniformID = getUniformID(uniformName);
    //m_gl.glUniform3fv(uniformID, value.size(), &value[0][0]); // uniform ID, count, value

    GLuint uniformID = getUniformID(QString(uniformName));

    size_t size = value.size();
    for (unsigned int i = 0; i < size; i++) {
        //char elementName[128];
        //memset(elementName, 0, sizeof(elementName));
        //snprintf(elementName, sizeof(elementName), "%s[%d]", uniformName.toStdString().c_str(), i);

        // Set uniform value
        m_gl.glUniform4fv(uniformID + i, 1, &value[i][0]);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::bindAttribute(int attribute, const QString & variableName)
{
    const char* variableChar = variableName.toStdString().c_str();
    m_gl.glBindAttribLocation(m_programID, attribute, variableChar);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::initializeShaderProgram()
{
    // Create program, attach shaders, link
    m_programID = m_gl.glCreateProgram();

	for (const auto& shader : m_shaders) {
		attachShader(shader);
	}
    m_gl.glLinkProgram(m_programID);

    // Print error if shaders failed to load
    int status;
    m_gl.glGetProgramiv(m_programID, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        logCritical("Caught error, Failed to link shader program");

        GLint maxLength = 0;
        m_gl.glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<char> errorChar(maxLength);
        m_gl.glGetProgramInfoLog(m_programID, maxLength, &maxLength, errorChar.data());
        std::string errorStr(std::begin(errorChar), std::end(errorChar));
        QString errorMsg = QString::fromStdString(errorStr);
        logError("Failed to link shader program: " + errorMsg);
    }

    // Validate program
    m_gl.glValidateProgram(m_programID);
    m_gl.glGetProgramiv(m_programID, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE)
    {
        logCritical("Caught error, failed to validate shader program");
        m_gl.printGLError("Failed to validate shader program");
    }

    // Populate uniform map with valid keys
    populateUniforms();

    // Populate uniform buffer map
    populateUniformBuffers();

    // Bind the shader for use
    bind();

#ifndef QT_NO_DEBUG_OUTPUT
    m_gl.printGLError("Failed to bind shader");
#endif

    return status;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::attachShader(const Shader & shader)
{
    m_gl.glAttachShader(m_programID, shader.getID());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::populateUniforms()
{
    // New approach, read directly from OpenGL
    std::vector<ShaderInputInfo> uniformInfo;
    getActiveUniforms(uniformInfo);

    // Old approach, deprecated, for now just update values from new info
    //size_t count = 0;
    //for (auto& shader : m_shaders) {
    //    for (ShaderInputInfo& info : shader.m_uniforms) {
    //        const QString& name = info.m_name;

    //        // Set ID of the uniform
    //        info.m_uniformID = getUniformIDGL(name);

    //        //if (info.m_uniformID < 0) {
    //        //    throw("Error, failed to load uniform ID");
    //        //}

    //        // Set uniform info in shader program
    //        m_uniformInfo[name] = info;
    //        count++;
    //    }
    //}

    // New uniform info generation approach
    for (const auto& info : uniformInfo) {
        // Remove index from array name for map indexing
        QString nonArrayName = info.m_name.split("[")[0];

        // Add both non-array and array names to info, both are valid for opengl
        // e.g., arrayOfValues[0] vs. arrayOfValues
        m_uniformInfo[nonArrayName] = info;
        if (nonArrayName != info.m_name) {
            m_uniformInfo[info.m_name] = info;
        }

        //ShaderInputInfo& thisInfo = m_uniformInfo[info.m_name];
        //thisInfo.m_uniformID = info.m_uniformID;
        //thisInfo.m_inputType = info.m_inputType;
    }

    m_uniforms.resize(m_uniformInfo.size());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::populateUniformBuffers()
{
    // Populate buffer metadata
    getActiveSSBs(m_bufferInfo);

    int lightBufferIndex;
    if (hasBuffer(LIGHT_BUFFER_NAME, GL::BufferType::kShaderStorage, &lightBufferIndex)) {
        // Set light buffer if it is used in shader
        CoreEngine* engine = CoreEngine::engines().begin()->second;
        m_lightBuffer = &engine->mainRenderer()->renderContext().lightingSettings().lightBuffer();
        size_t lightSize = sizeof(Light);
        if (m_bufferInfo[lightBufferIndex].m_bufferDataSize % lightSize != 0) {
            throw("Error, data-size mismatch between Light and SSB");
        }
    }

    // Associate with uniform buffers as necessary
    for (const auto& shader : m_shaders) {
        for (const ShaderStruct& ss : shader.m_uniformBuffers) {
            if (!UBO::get(ss.m_name)) {
                // Create UBO if it has not been initialized by another shader
                auto ubo = UBO::create(ss);
            }

            // Set as a core UBO if it should not be deleted on scenario reload
            auto ubo = UBO::UBO_MAP.at(ss.m_name);
            if (Shader::Builtins.contains(m_name)) {
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
    size_t index = m_gl.glGetUniformBlockIndex(m_programID, blockName.toStdString().c_str());
    return index;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::bindUniformBlock(const QString & blockName)
{
    size_t blockIndex = getUniformBlockIndex(blockName);
    const std::shared_ptr<UBO>& ubo = m_uniformBuffers.at(blockName);
    m_gl.glUniformBlockBinding(m_programID, blockIndex, ubo->m_bindingPoint);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::getActiveUniforms(std::vector<ShaderInputInfo>& outInfo)
{
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
        m_gl.glGetProgramResourceiv(m_programID,
            GL_UNIFORM,
            uniform, // index of uniform
            properties.size(),
            &properties[0],
            values.size(),
            NULL, 
            &values[0]);

        // Get the name from the program
        nameData.resize(values[0]); //The length of the name.
        m_gl.glGetProgramResourceName(m_programID,
            GL_UNIFORM,
            uniform, // index of uniform
            nameData.size(),
            NULL,
            &nameData[0]);
        QString name((char*)&nameData[0]);

        outInfo.push_back(ShaderInputInfo());
        outInfo.back().m_name = name;
        outInfo.back().m_uniformID = values[4];
        outInfo.back().m_localIndex = uniform;
        outInfo.back().m_inputType = ShaderInputType(values[1]);
        outInfo.back().m_flags.setFlag(ShaderInputInfo::kIsArray, 
            values[2] < 2 ? false : true);
        outInfo.back().m_flags.setFlag(ShaderInputInfo::kInBlockOrBuffer, values[3] > -1);
        outInfo.back().m_arraySize = values[2];

    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::getActiveSSBs(std::vector<ShaderBufferInfo>& outInfo)
{
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

    GLint numActiveBuffers = getNumActiveBuffers(GL::BufferType::kShaderStorage);
    for (int ssb = 0; ssb < numActiveBuffers; ++ssb)
    {
        // Get specified properties from program
        m_gl.glGetProgramResourceiv(m_programID,
            GL_SHADER_STORAGE_BLOCK,
            ssb, // index of shader storage block
            properties.size(),
            &properties[0],
            values.size(),
            NULL,
            &values[0]);

        // Get the name from the program
        nameData.resize(values[0]); //The length of the name.
        m_gl.glGetProgramResourceName(m_programID,
            GL_SHADER_STORAGE_BLOCK,
            ssb, // index of ssb
            nameData.size(),
            NULL,
            &nameData[0]);
        QString name((char*)&nameData[0]);

        outInfo.push_back(ShaderBufferInfo());
        outInfo.back().m_name = name;
        outInfo.back().m_bufferType = GL::BufferType::kShaderStorage;
        outInfo.back().m_bufferBinding = values[1];
        outInfo.back().m_bufferDataSize = values[2]; 
        outInfo.back().m_numActiveVariables = values[3];
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::getActiveAttributes(std::vector<ShaderInputInfo>& outInfo)
{
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
        m_gl.glGetProgramResourceiv(m_programID,
            GL_PROGRAM_INPUT, // Since attributes are just vertex shader inputs
            attrib, // index of attribute
            properties.size(),
            &properties[0], 
            values.size(), 
            NULL, 
            &values[0]);

        nameData.resize(values[0]); //The length of the name.
        m_gl.glGetProgramResourceName(m_programID,
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
    GLint numActiveUniforms = 0;
    m_gl.glGetProgramInterfaceiv(m_programID,
        GL_UNIFORM,
        GL_ACTIVE_RESOURCES,
        &numActiveUniforms);
    return numActiveUniforms;
}
/////////////////////////////////////////////////////////////////////////////////////////////
GLint ShaderProgram::getNumActiveBuffers(GL::BufferType bufferType)
{
    GLint numActiveBuffers = 0;
    m_gl.glGetProgramInterfaceiv(m_programID,
        (size_t)bufferType,
        GL_ACTIVE_RESOURCES,
        &numActiveBuffers);
    return numActiveBuffers;
}
/////////////////////////////////////////////////////////////////////////////////////////////
GLint ShaderProgram::getNumActiveAttributes()
{
    GLint numActiveAttributes = 0;
    m_gl.glGetProgramInterfaceiv(m_programID,
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