#include "GbShaders.h"

// QT
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

// Internal
#include "../../readers/GbFileReader.h"
#include "../../GbSettings.h"
#include "GbUniformBufferObject.h"

namespace Gb {   

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
Shader::Shader(const QString& file, ShaderType type):
    m_type(type),
    m_filePath(file)
{
    m_name = FileReader::pathToName(m_filePath, false);
    initializeFromSourceFile();
}

/////////////////////////////////////////////////////////////////////////////////////////////
Shader::~Shader()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Shader::asJson() const
{
    QJsonObject object;
    object.insert("shaderType", int(m_type));
    object.insert("filePath", m_filePath);
    object.insert("name", m_name);
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();
    m_type = ShaderType(object["shaderType"].toInt());
    m_filePath = object["filePath"].toString();
    if (!object.contains("name")) {
        m_name = FileReader::pathToName(m_filePath, false);
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
    auto fileReader = Gb::FileReader(m_filePath);
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
        QString uniformType = lineMatch.captured(2); // Type 
        bool isArray = !lineMatch.captured(4).isNull();// Is array
        int arraySize;

        if (MISALIGNED_TYPES.contains(uniformType)) {
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
        UniformInfo::PrecisionQualifier precision;
        if (!lineMatch.captured(1).isNull()) {
            QString precisionStr = lineMatch.captured(1);
            if (precisionStr.contains("high"))
                precision = UniformInfo::kHigh;
            else if (precisionStr.contains("medium"))
                precision = UniformInfo::kMedium;
            else
                precision = UniformInfo::kLow;
        }
        else {
            precision = UniformInfo::kNone;
        }

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
    QString uniformType = match.captured(2); // Type 
    bool isArray = !match.captured(4).isNull();// Is array

    // Precision
    UniformInfo::PrecisionQualifier precision;
    if (!match.captured(1).isNull()) {
        QString precisionStr = match.captured(1);
        if (precisionStr.contains("high"))
            precision = UniformInfo::kHigh;
        else if (precisionStr.contains("medium"))
            precision = UniformInfo::kMedium;
        else
            precision = UniformInfo::kLow;
    }
    else {
        precision = UniformInfo::kNone;
    }

    // Check if uniform is a struct or not
    auto iter = std::find_if(m_structs.begin(), m_structs.end(),
        [&](const ShaderStruct& s) {return s.m_name == uniformType; });
    bool isStruct = iter != m_structs.end();
    if (isStruct) {
        // Add all the uniforms in the struct to the uniform list
        for (const UniformInfo& field : iter->m_fields) {
            QString combinedName = uniformName + "." + field.m_name;
            Vec::EmplaceBack(m_uniforms, std::move(combinedName), field.m_typeStr, field.m_isArray);
        }
    }
    else {
        // Add as a normal uniform
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
        QString uniformType = lineMatch.captured(2); // Type 
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
        UniformInfo::PrecisionQualifier precision;
        if (!lineMatch.captured(1).isNull()) {
            QString precisionStr = lineMatch.captured(1);
            if (precisionStr.contains("high")) 
                precision = UniformInfo::kHigh;
            else if (precisionStr.contains("medium"))
                precision = UniformInfo::kMedium;
            else
                precision = UniformInfo::kLow;
        }
        else {
            precision = UniformInfo::kNone;
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
std::shared_ptr<ShaderProgram> ShaderProgram::create(const QJsonValue & json)
{
    // Return ShaderProgram based on type
    std::shared_ptr<ShaderProgram> program;
    const QJsonObject object = json.toObject();
    ShaderProgramType type = ShaderProgramType(object["programType"].toInt());
    switch (type) {
    case kBasic:
        program = std::make_shared<BasicShaderProgram>(object);
        break;
    case kCubemap:
        program = std::make_shared<CubemapShaderProgram>(object);
        break;
    case kGeneric:
    default:
        program = std::make_shared<ShaderProgram>(object);
        break;    
    }

    if (program->isValid()) {
        return program;
    }
    else {
        return nullptr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram():
    Object()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(const ShaderProgram & shaderProgram):
    m_shaders(shaderProgram.m_shaders),
    m_programID(shaderProgram.m_programID),
    m_gl(shaderProgram.m_gl),
    m_uniformQueue(shaderProgram.m_uniformQueue),
    m_uniforms(shaderProgram.m_uniforms),
    m_uniformInfo(shaderProgram.m_uniformInfo),
    m_programType(shaderProgram.m_programType)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(const QJsonValue & json):
    Object()
{
    loadFromJson(json);
}

/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(const QString& vertfile, const QString& fragfile, ShaderProgramType type):
    Object(),
    m_programType(type)
{
	Map::Emplace(m_shaders, Shader::kVertex, vertfile, Shader::kVertex);
	Map::Emplace(m_shaders, Shader::kFragment, fragfile, Shader::kFragment);

    setName();

    initializeShaderProgram();
}

/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::~ShaderProgram()
{
    // Delete shaders linked to the program
    release();
	for (const auto& shaderPair : m_shaders) {
		m_gl.glDetachShader(m_programID, shaderPair.second.getID());
	}
	for (const auto& shaderPair : m_shaders) {
        m_gl.glDeleteShader(shaderPair.second.getID());
	}
    m_gl.glDeleteProgram(m_programID);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::isValid() const
{
    bool valid = true;
    for (const std::pair<Shader::ShaderType, Shader>& shaderPair : m_shaders) {
        valid &= shaderPair.second.isValid();
    }
    return valid;
}
/////////////////////////////////////////////////////////////////////////////////////////////
const Shader* ShaderProgram::getFragShader() const
{
    if (m_shaders.find(Shader::kFragment) != m_shaders.end()) {
        return &m_shaders.at(Shader::kFragment);
    }
    else {
        return nullptr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
const Shader* ShaderProgram::getVertShader() const
{
    if (m_shaders.find(Shader::kVertex) != m_shaders.end()) {
        return &m_shaders.at(Shader::kVertex);
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
    for (const std::pair<Shader::ShaderType, Shader>& shaderPair : m_shaders) {
        // Iterate over shaders and combine names if different, otherwise take filename without extension
        QString shaderName = shaderPair.second.getName();
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
bool ShaderProgram::hasUniform(const QString& uniformName) const
{
    return Map::HasKey(m_uniforms, uniformName);
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

    BOUND_SHADER = this;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::release()
{
    m_gl.glUseProgram(0);
    BOUND_SHADER = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ShaderProgram::asJson() const
{
    QJsonObject object;

    // Add shaders to json
    QJsonObject shaders;
    for (const auto& shaderPair : m_shaders) {
        shaders.insert(QString::number(shaderPair.first), shaderPair.second.asJson());
    }
    object.insert("shaders", shaders);

    // Set name
    object.insert("name", m_name);

    // Add current uniform values to JSON
    QJsonObject uniforms;
    for (const auto& uniformPair : m_uniforms) {
        if (uniformPair.second.isPersistent()) {
            if (uniformPair.second.valid()) {
                // Save all valid uniforms to JSON
                uniforms.insert(uniformPair.second.getName(), uniformPair.second.asJson());
            }
        }
    }
    object.insert("uniforms", uniforms);

    // Set program type
    QJsonObject programType;
    object.insert("programType", int(m_programType));

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();

    // Load program type
    m_programType = ShaderProgramType(object["programType"].toInt());

    // Load shaders from JSON
    QJsonObject shaders = object["shaders"].toObject();
    for (const auto& key : shaders.keys()) {
        Shader::ShaderType shaderType = Shader::ShaderType(key.toInt());
        QJsonValue shaderJson = object["shaders"].toObject()[key];
        Shader shader = Shader(shaderJson);
        m_shaders[shaderType] = shader;
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
    
    // Load uniform values into uniform queue for update
    QJsonObject uniforms = object["uniforms"].toObject();
    m_uniformQueue.clear();
    for (const auto& key : uniforms.keys()) {
        QJsonObject uniformJson = uniforms[key].toObject();
        Vec::EmplaceBack(m_uniformQueue, uniformJson);
    }

    // Update uniforms
    //updateUniforms();

}

/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::updateUniforms()
{
    QMutexLocker locker(&m_mutex);

    // Iterate through queue to update uniforms in local map and open GL
    std::unordered_map<QString, Uniform>& uniforms = m_uniforms;
    bool updated = false;
    for (Uniform& uniform: m_uniformQueue) {

        // Skip if uniform is invalid
        if (!uniform.valid()) continue;

        // Check that uniform type matches
        const QString& uniformName = uniform.getName();

        // Skip if uniform is not present
        if (!Map::HasKey(uniforms, uniformName)) {
#ifdef DEBUG_MODE
            logWarning("Error, uniform not recognized: " + uniformName);
#endif
            continue;
        }

        // Skip if uniform value is unchanged
        const Uniform& prevUniform = uniforms.at(uniformName);
        if (prevUniform == uniform) {
            continue;
        }

        UniformInfo& info = m_uniformInfo.at(uniformName);
        std::type_index typeInfo = uniform.typeInfo();
        //auto name = typeInfo.name();
        bool matchesInfo = uniform.matchesInfo(info);
        if (!matchesInfo) {
            // TODO: Implement a more performant fix for this if speed becomes an issue
            // If uniform is a float and should be an int, was read in incorrectly
            // from Json via QVariant
            const std::type_index& classType = Uniform::UNIFORM_TYPE_MAP.at(info.m_typeStr);
            if (uniform.is<float>() && classType == typeid(int)) {
                uniform.set<int>((int)uniform.get<float>());
            }
            else {
                size_t intType = typeid(int).hash_code();
                size_t boolType = typeid(bool).hash_code();
                Q_UNUSED(intType);
                Q_UNUSED(boolType);
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
        m_uniforms[uniformName] = std::move(uniform);
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
template<>
void ShaderProgram::setUniformValueGL(const QString& uniformName, const std::vector<Matrix4x4f>& value)
{
    // Old approach, doesn't work
    //GLuint uniformID = getUniformID(uniformName);
    //m_gl.glUniformMatrix4fv(uniformID, value.size(), GL_FALSE, value[0].m_mtx[0].data()); // uniform ID, count, transpose, value

    // Send to each location in the array
    size_t size = value.size();
    for (unsigned int i = 0; i < size; i++) {
        char elementName[128];
        memset(elementName, 0, sizeof(elementName));
        snprintf(elementName, sizeof(elementName), "%s[%d]", uniformName.toStdString().c_str(), i);

        // Set uniform value
        GLuint uniformID = getUniformID(QString(elementName));
        m_gl.glUniformMatrix4fv(uniformID, 1, GL_FALSE, value[i].m_mtx[0].data()); // uniform ID, count, transpose, value
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
template<>
void ShaderProgram::setUniformValueGL(const QString & uniformName, const Vec3List & value)
{
    //GLuint uniformID = getUniformID(uniformName);
    //m_gl.glUniform3fv(uniformID, value.size(), &value[0][0]); // uniform ID, count, value

    size_t size = value.size();
    for (unsigned int i = 0; i < size; i++) {
        char elementName[128];
        memset(elementName, 0, sizeof(elementName));
        snprintf(elementName, sizeof(elementName), "%s[%d]", uniformName.toStdString().c_str(), i);

        // Set uniform value
        GLuint uniformID = getUniformID(QString(elementName));
        m_gl.glUniform3fv(uniformID, 1, &value[i][0]);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
template<>
void ShaderProgram::setUniformValueGL(const QString & uniformName, const Vec4List & value)
{    
    //GLuint uniformID = getUniformID(uniformName);
    //m_gl.glUniform3fv(uniformID, value.size(), &value[0][0]); // uniform ID, count, value

    size_t size = value.size();
    for (unsigned int i = 0; i < size; i++) {
        char elementName[128];
        memset(elementName, 0, sizeof(elementName));
        snprintf(elementName, sizeof(elementName), "%s[%d]", uniformName.toStdString().c_str(), i);

        // Set uniform value
        GLuint uniformID = getUniformID(QString(elementName));
        m_gl.glUniform4fv(uniformID, 1, &value[i][0]);
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

	for (const auto& shaderPair : m_shaders) {
		attachShader(shaderPair.second);
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
    for (const auto& shaderPair : m_shaders) {
        for (const UniformInfo& info : shaderPair.second.m_uniforms) {
            const QString& name = info.m_name;
            Map::Emplace(m_uniforms, name, name);
            m_uniformInfo[name] = info;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::populateUniformBuffers()
{
    for (const auto& shaderPair : m_shaders) {
        for (const ShaderStruct& ss : shaderPair.second.m_uniformBuffers) {
            if (!UBO::get(ss.m_name)) {
                // Create UBO if it has not been initialized by another shader
                UBO::create(ss);
            }
            
            // Add UBO to shader map
            m_uniformBuffers[ss.m_name] = UBO::UBO_MAP.at(ss.m_name);

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
ShaderProgram* ShaderProgram::BOUND_SHADER = nullptr;
/////////////////////////////////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Simple color shader program
/////////////////////////////////////////////////////////////////////////////////////////////
BasicShaderProgram::BasicShaderProgram() :
    ShaderProgram(":/shaders/basic.vert", ":/shaders/basic.frag", kBasic)
{
    // Shader is already bound from construction, so set shader mode to 0 by default
    setUniformValue("shaderMode", 0);
    //setUniformValue("useNormalMap", false);
}
/////////////////////////////////////////////////////////////////////////////////////////////
BasicShaderProgram::BasicShaderProgram(const QJsonValue & json):
    ShaderProgram(json)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
BasicShaderProgram::~BasicShaderProgram()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////
//void BasicShaderProgram::setMode(int mode)
//{
//    //m_mode = mode;
//    setUniformValue("shaderMode", mode);
//    updateUniforms();
//}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Cubemap shader program
CubemapShaderProgram::CubemapShaderProgram() :
    ShaderProgram(":/shaders/cubemap.vert", ":/shaders/cubemap.frag", kCubemap)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
CubemapShaderProgram::CubemapShaderProgram(const QJsonValue & json):
    ShaderProgram(json)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
CubemapShaderProgram::~CubemapShaderProgram()
{
}


/////////////////////////////////////////////////////////////////////////////////////////////
// End namespacing
}