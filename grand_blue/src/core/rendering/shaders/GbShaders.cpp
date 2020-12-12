#include "GbShaders.h"

// QT
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

// Internal
#include "../../readers/GbFileReader.h"
#include "../../GbSettings.h"
#include "../buffers/GbUniformBufferObject.h"
#include "../lighting/GbLightSettings.h"
#include "../lighting/GbShadowMap.h"
#include "../lighting/GbLightClusterGrid.h"
#include "../../GbCoreEngine.h"
#include "../renderer/GbRenderContext.h"
#include "../renderer/GbMainRenderer.h"

#define GL_GLEXT_PROTOTYPES 

//#define CACHE_UNIFORM_VALUES
#define CHECK_UNIFORM_EXISTENCE

namespace Gb {   
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// ShaderInputInfo
/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderInputInfo::IsValidGLType(int typeInt)
{
    switch (ShaderVariableType(typeInt)) {
    case ShaderVariableType::kBool:
    case ShaderVariableType::kInt:
    case ShaderVariableType::kFloat:
    case ShaderVariableType::kDouble:
    case ShaderVariableType::kVec2:
    case ShaderVariableType::kVec3:
    case ShaderVariableType::kVec4:
    case ShaderVariableType::kMat2:
    case ShaderVariableType::kMat3:
    case ShaderVariableType::kMat4:
    case ShaderVariableType::kSamplerCube:
    case ShaderVariableType::kSampler2D:
        return true;
    default:
        throw("GL type is not valid, need to account for this type");
        return false;
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderInputInfo::ShaderInputInfo() :
    m_variableCType(typeid(nullptr))
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderInputInfo::ShaderInputInfo(const ShaderInputInfo & info) :
    m_name(info.m_name),
    m_variableType(info.m_variableType),
    m_variableCType(info.m_variableCType),
    m_variableTypeStr(info.m_variableTypeStr),
    m_inputType(info.m_inputType),
    m_flags(info.m_flags),
    m_uniformID(info.m_uniformID),
    m_localIndex(info.m_localIndex),
    m_arraySize(info.m_arraySize),
    m_precision(info.m_precision)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderInputInfo::ShaderInputInfo(const QString & name, const ShaderVariableType & type, InputType inputType) :
    m_name(name),
    m_variableType(type),
    m_inputType(inputType),
    m_variableCType(Uniform::s_uniformGLTypeMap.contains(type) ?
        Uniform::s_uniformGLTypeMap.at(type) : typeid(nullptr))
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderInputInfo::ShaderInputInfo(const QString & name, const ShaderVariableType & type, InputType inputType, bool isArray) :
    ShaderInputInfo(name, type, inputType)
{
    m_flags.setFlag(kIsArray, isArray);
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderInputInfo::ShaderInputInfo(const QString & name, const ShaderVariableType & type, InputType inputType, bool isArray, int id) :
    ShaderInputInfo(name, type, inputType, isArray)
{
    m_uniformID = id;
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderInputInfo::~ShaderInputInfo()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
QString ShaderInputInfo::asGLSL() const
{
    QString glsl;
    if (!inBlockOrBuffer()) {
        switch (m_inputType) {
        case InputType::kIn:
            glsl += QStringLiteral("in ");
            break;
        case InputType::kOut:
            glsl += QStringLiteral("out ");
            break;
        case InputType::kUniform:
            glsl += QStringLiteral("uniform ");
            break;
        case InputType::BufferField:
            break;
        case InputType::kNone:
        default:
            throw("Error, invalid input type");
            break;
        }
    }

    QString typeStr;
    if (m_variableType == ShaderVariableType::kCustomStruct) {
        typeStr = m_variableTypeStr;
    }
    else {
        typeStr = Uniform::s_uniformStrTypeMap[m_variableType];
    }
    glsl += typeStr + " ";
    glsl += m_name.c_str();

    if (isArray()) {
        if (m_arraySize > -1) {
            glsl += "[" + QString::number(m_arraySize) + "]";
        }
        else {
            glsl += "[]";
        }
    }

    glsl += ";";

    return glsl;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// ShaderBufferInfo
/////////////////////////////////////////////////////////////////////////////////////////////
QString ShaderBufferInfo::asGLSL() const
{
    // Initialize with layout qualifier
    QString glsl = QStringLiteral("layout (");
    QString layoutStr;
    switch (m_blockLayout) {
    case GL::ShaderBlockLayout::kPacked:
        layoutStr = QStringLiteral("packed");
        break;
    case GL::ShaderBlockLayout::kShared:
        layoutStr = QStringLiteral("shared");
        break;
    case GL::ShaderBlockLayout::kStd140:
        layoutStr = QStringLiteral("std140");
        break;
    case GL::ShaderBlockLayout::kStd430:
        layoutStr = QStringLiteral("std430");
        break;
    default:
        throw("Error, block layout not recognized");
        break;
    }
    glsl += layoutStr;

    // Add binding if applicable
    if (m_bufferBinding > -1) {
        glsl += ", binding = " + QString::number(m_bufferBinding);
    }

    glsl += ") ";

    // Add memory qualifier if applicable
    switch (m_memoryQualifier) {
    case GL::BufferMemoryQualifier::kNone:
        break;
    case GL::BufferMemoryQualifier::kCoherent:
        glsl += QStringLiteral("coherent ");
        break;
    case GL::BufferMemoryQualifier::kVolatile:
        glsl += QStringLiteral("volatile ");
        break;
    case GL::BufferMemoryQualifier::kRestrict:
        glsl += QStringLiteral("restrict ");
        break;
    case GL::BufferMemoryQualifier::kReadOnly:
        glsl += QStringLiteral("readonly ");
        break;
    case GL::BufferMemoryQualifier::kWriteOnly:
        glsl += QStringLiteral("writeonly ");
        break;
    default:
        throw("Error, unrecognized memory qualifier");
        break;
    }

    // Add buffer type
    switch (m_bufferType) {
    case GL::BufferBlockType::kUniformBuffer:
        glsl += QStringLiteral("uniform ");
        break;
    case GL::BufferBlockType::kShaderStorage:
        glsl += QStringLiteral("buffer ");
        break;
    }

    // Add buffer name
    glsl += m_name + "\n{\n";

    // Add fields
    for (const ShaderInputInfo& field : m_fields) {
        glsl += field.asGLSL() + "\n";
    }

    // Close off buffer and add variable name (may be null)
    glsl += "\n} " + m_variableName + ";\n";

    return glsl;
}



/////////////////////////////////////////////////////////////////////////////////////////////
// Shader
/////////////////////////////////////////////////////////////////////////////////////////////
GString Shader::s_worldMatrixUniformName = "worldMatrix";

/////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::pair<QString, Shader::ShaderType>> Shader::s_builtins = {
    {"simple", Shader::ShaderType::kVertex},
    {TEXT_SHADER_NAME, Shader::ShaderType::kVertex},
    {"basic", Shader::ShaderType::kVertex},
    {"basic_cluster", Shader::ShaderType::kVertex},
    {"lines", Shader::ShaderType::kVertex},
    {"cubemap", Shader::ShaderType::kVertex},
    {"axes", Shader::ShaderType::kVertex},
    {"points", Shader::ShaderType::kVertex},
    {"debug_skeleton", Shader::ShaderType::kVertex},
    {"quad", Shader::ShaderType::kVertex},
    {"prepass", Shader::ShaderType::kVertex},
    {"prepass_deferred", Shader::ShaderType::kVertex},
    {"prepass_shadowmap", Shader::ShaderType::kVertex},
    {"light_cluster_grid", Shader::ShaderType::kCompute},
    {"light_culling", Shader::ShaderType::kCompute},
    {"active_clusters", Shader::ShaderType::kCompute},
    {"active_clusters_compact", Shader::ShaderType::kCompute},
    {"ssao", Shader::ShaderType::kFragment}, // TODO: Fix up, SSAO shader is loaded manually
    {"ssao_blur", Shader::ShaderType::kFragment}
};

/////////////////////////////////////////////////////////////////////////////////////////////
std::vector<int> Shader::s_shaderTypeToGLMap
{   
    GL_VERTEX_SHADER,   // 0
    GL_FRAGMENT_SHADER, // 1
    GL_GEOMETRY_SHADER, // 2
    GL_COMPUTE_SHADER   // 3
};

/////////////////////////////////////////////////////////////////////////////////////////////
std::vector<QString> Shader::s_shaderTypeToStringMap
{   QStringLiteral("Vertex"),  // 0
    QStringLiteral("Fragment") // 1
}
;
/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::CombineEffectFragmentShaders(std::vector<const Shader*>& shaders)
{
    // Assumptions:
    // *Input texture coordinates are described by "in vec2 texCoords;"
    // *Texture samplers are called screenTexture, depthTexture, and stencilTexture

    static QString boilerPlate = QStringLiteral("#ifdef GL_ES\n precision mediump int;\n precision mediump float;\n#endif\n");

    QString outSource = boilerPlate;
    
    QString texCoords = QStringLiteral("texCoords");
    tsl::robin_map<GString, ShaderInputInfo> inputs = { 
        {texCoords, 
        {texCoords, ShaderVariableType::kVec2, ShaderInputInfo::InputType::kIn }} 
    }; // Ensure that texCoords are an input to the fragment shader
    tsl::robin_map<GString, ShaderInputInfo> outputs;
    std::vector<ShaderInputInfo> inputVec;
    std::vector<ShaderInputInfo> outputVec;
    tsl::robin_map<GString, ShaderInputInfo> uniforms;
    std::vector<ShaderFunction> functions;
    std::vector<ShaderFunction> mainFunctions;

    // Iterate through shaders to pull metadata
    for (const Shader* shaderPtr : shaders) {
        const Shader& shader = *shaderPtr;
        size_t outputSize = outputVec.size();
        shader.getIO(inputVec, outputVec);
        shader.getFunctions(functions);

        // Add uniforms to map
        for (const ShaderInputInfo& uniformInfo : shader.uniformInfo()) {
            uniforms[uniformInfo.name()] = uniformInfo;
        }

        if (outputVec.size() - outputSize != 1) {
            throw("Error, fragment shader had  more than one output");
        }
    }

    // Move inputs and outputs to maps
    for (const ShaderInputInfo& input: inputVec) {
        inputs[input.name()] = input;
    }
    for (const ShaderInputInfo& output : outputVec) {
        outputs[output.name()] = output;
    }

    // Construct shader
    QString space = QStringLiteral(" ");
    QString sc = QStringLiteral(";");
    QString newLine = QStringLiteral("\n");

    // Add inputs
    for (const auto& inputPair : inputs) {
        const ShaderInputInfo& input = inputPair.second;
        outSource += input.asGLSL() + newLine;
    }

    // Add output
    static QString outputName = QStringLiteral("fColor");
    outSource += QStringLiteral("out vec4 fColor;\n\n");

    // Add uniforms
    // TODO: Don't hard-code texture uniform names
    outSource += "layout (binding=0) uniform sampler2D screenTexture;\n";
    outSource += "layout (binding=1) uniform sampler2D depthTexture;\n";
    outSource += "layout (binding=2) uniform usampler2D stencilTexture;\n";
    outSource += "layout (binding=3) uniform sampler2D sceneTexture;\n";
    outSource += "layout (binding=4) uniform sampler2D checkpointTexture;\n";
    std::vector<GString> ignoredUniforms = {"screenTexture", 
        "depthTexture", 
        "stencilTexture",
        "sceneTexture",
        "checkpointTexture" };
    for (const auto& uniformPair: uniforms) {
        auto it = std::find_if(ignoredUniforms.begin(), ignoredUniforms.end(),
            [&](const GString& g_str) {
            return g_str == uniformPair.first;
        });
        if (it != ignoredUniforms.end()) {
            continue;
        }

        outSource += uniformPair.second.asGLSL() + newLine;
    }

    outSource += newLine + newLine;

    // Add functions
    tsl::robin_map<QString, int> functionMap;
    for (size_t i = 0; i < functions.size(); i++) {
        ShaderFunction& function = functions[i];

        // Skip main, append to list of main functions
        if (function.m_name == QStringLiteral("main")) {
            mainFunctions.push_back(function);
            continue;
        }

        if (Map::HasKey(functionMap, function.m_name)) {
            // Renaming duplicate functions
            QString oldName = function.m_name;
            QString oldDefinitionStart = function.m_definitionStart;
            function.m_name += QStringLiteral("_1");
            function.m_definitionStart.replace(oldName, function.m_name);
            function.m_definition.replace(oldDefinitionStart, function.m_definitionStart);
        }

        // Append function to source
        outSource += function.m_definition + newLine + newLine;

        functionMap[function.m_name] = i;
    }

    // Construct main loop
    QString screenTexRegStr = QStringLiteral("(texture)[\\s\\t\\n]*\\([\\s\\t\\n]*(screenTexture)[\\s\\t\\n]*\\,[\\s\\t\\n]*(texCoords)[\\s\\t\\n]*\\)[\\s\\t\\n]*");
    outSource += QStringLiteral("void main(){\n\n");
    outSource += "\tfColor = texture(screenTexture, texCoords);\n\n";
    for (size_t i = 0; i < mainFunctions.size(); i++) {
        ShaderFunction& function = mainFunctions[i];

        // Replace output variable name so that it is consistent
        QString localOutputName = outputVec[i].name();
        function.m_body.replace(localOutputName, outputName);

        // Replace texture sampling with reference to fColor so that
        // shader effects chain together instead of overwriting one another
        QRegularExpression re(screenTexRegStr);
        QRegularExpressionMatchIterator j = re.globalMatch(function.m_body);
        while (j.hasNext()) {
            QRegularExpressionMatch match = j.next();
            QString toReplace = match.captured(0);
            function.m_body.replace(toReplace, QStringLiteral("fColor"));
        }

        outSource += function.m_body + newLine + newLine;
    }
    outSource += "\n}";

    return outSource;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::CombineEffectVertexShaders(std::vector<const Shader*>& shaders)
{
    // Assumptions:
    // Vertex shaders do not make any modifications to gl_position, and do not
    // modify the texCoords output variable

    // Initialize quad shader's basic components
    QString outSource;
    QString quadShaderAttributes = 
        QStringLiteral("layout(location = 0) in vec3 position;\n") +
        QStringLiteral("layout(location = 1) in vec4 color;\n") +
        QStringLiteral("layout(location = 2) in vec2 uvCoord;\n");

    //QString quadShaderOut = QStringLiteral("out vec2 texCoords;\n\n");
    QString quadShaderUniforms = QStringLiteral("uniform vec3 offsets;\n") +
        QStringLiteral("uniform vec2 scale;\n\n");
    QString quadMainStart = QStringLiteral("void main()\n{\n");
    
    QString quadMainContents = QStringLiteral("    gl_Position = vec4(position.x * scale.x + (offsets.x), position.y * scale.y + (offsets.y), offsets.z, 1.0f);\n") +
        QStringLiteral("    texCoords = uvCoord;\n");

    QString quadMainEnd = "}  ";

    // Add attributes and default out to source
    outSource += quadShaderAttributes;

    QString texCoords = QStringLiteral("texCoords");
    tsl::robin_map<QString, ShaderInputInfo> outputs = {
        {texCoords,
        {texCoords, ShaderVariableType::kVec2, ShaderInputInfo::InputType::kOut }}
    }; // Ensure that texCoords are an output to the fragment shader
    std::vector<ShaderInputInfo> inputVec;
    std::vector<ShaderInputInfo> outputVec;
    tsl::robin_map<QString, ShaderInputInfo> uniforms;
    tsl::robin_map<QString, ShaderBufferInfo> buffers;
    std::vector<ShaderFunction> functions;
    std::vector<ShaderFunction> mainFunctions;

    // Iterate through shaders to pull metadata
    for (const Shader* shaderPtr : shaders) {
        const Shader& shader = *shaderPtr;
        //size_t outputSize = outputVec.size();
        shader.getIO(inputVec, outputVec);
        shader.getFunctions(functions);

        // Add uniforms to map
        for (const ShaderInputInfo& uniformInfo : shader.uniformInfo()) {
            uniforms[uniformInfo.name()] = uniformInfo;
        }
        // Add buffers to map
        for (const ShaderBufferInfo& bufferInfo : shader.m_uniformBuffers) {
            buffers[bufferInfo.m_name] = bufferInfo;
        }
        for (const ShaderBufferInfo& bufferInfo : shader.m_buffers) {
            buffers[bufferInfo.m_name] = bufferInfo;
        }
    }

    // Move outputs to map
    for (const ShaderInputInfo& output : outputVec) {
        outputs[output.name()] = output;
    }

    // Construct shader
    QString space = QStringLiteral(" ");
    QString sc = QStringLiteral(";");
    QString newLine = QStringLiteral("\n");

    // Add output
    for (const auto& outputPair : outputs) {
        const ShaderInputInfo& output = outputPair.second;
        outSource += output.asGLSL() + newLine;
    }

    // Add uniforms
    outSource += quadShaderUniforms;
    QStringList ignoredUniforms = { QStringLiteral("offsets"), QStringLiteral("scale") };
    for (const auto& uniformPair : uniforms) {
        if (ignoredUniforms.contains(uniformPair.first)) {
            continue;
        }

        outSource += uniformPair.second.asGLSL();
    }

    outSource += newLine;

    // Add buffers
    for (const auto& bufferPair : buffers) {
        outSource += bufferPair.second.asGLSL() + newLine + newLine;
    }

    // Add functions
    tsl::robin_map<QString, int> functionMap;
    for (size_t i = 0; i < functions.size(); i++) {
        ShaderFunction& function = functions[i];

        // Skip main, append to list of main functions
        if (function.m_name == QStringLiteral("main")) {
            mainFunctions.push_back(function);
            continue;
        }

        if (Map::HasKey(functionMap, function.m_name)) {
            // Renaming duplicate functions
            QString oldName = function.m_name;
            QString oldDefinitionStart = function.m_definitionStart;
            function.m_name += QStringLiteral("_1");
            function.m_definitionStart.replace(oldName, function.m_name);
            function.m_definition.replace(oldDefinitionStart, function.m_definitionStart);
        }

        // Append function to source
        outSource += function.m_definition + newLine + newLine;

        functionMap[function.m_name] = i;
    }

    // Construct main loop
    outSource += quadMainStart + quadMainContents;
    for (size_t i = 0; i < mainFunctions.size(); i++) {
        ShaderFunction& function = mainFunctions[i];
        outSource += function.m_body + newLine + newLine;
    }
    outSource += quadMainEnd;


    return outSource;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Shader::Shader():
    m_isValid(false)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Shader::Shader(const QJsonValue & json):
    m_isValid(false)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Shader::Shader(ShaderType type):
    m_type(type),
    m_isValid(false)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
//Shader::Shader(const QString & file, ShaderType type):
//    Shader(file, type, false)
//{
//}

/////////////////////////////////////////////////////////////////////////////////////////////
Shader::Shader(const QString& file, ShaderType type, bool deferConstruction):
    Loadable(file),
    m_type(type),
    m_isValid(false)
{
    m_name = FileReader::PathToName(m_path, false);
    if (!deferConstruction) {
        initializeFromSourceFile();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
Shader::~Shader()
{
    if (m_shaderID > -1) {
        m_gl.glDeleteShader(m_shaderID);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Shader::asJson() const
{
    QJsonObject object = Loadable::asJson().toObject();
    object.insert("shaderType", int(m_type));
    object.insert("filePath", m_path.c_str());
    object.insert("name", m_name.c_str());
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Loadable::loadFromJson(json, context);
    const QJsonObject& object = json.toObject();
    m_type = ShaderType(object["shaderType"].toInt());
    if (!object.contains("name")) {
        m_name = FileReader::PathToName(m_path, false);
    }
    else {
        m_name = object["name"].toString();
    }
    //initializeFromSourceFile();
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
    m_source += "\n";
    m_source += "#line 0 \n";

    // Append source code
    m_source += fileLines.join("\n").toStdString();

    // Initialize shader in GL
    initializeGL();

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::initializeGL()
{
    // Load shader in open GL
    m_shaderID = m_gl.glCreateShader(s_shaderTypeToGLMap[(int)m_type]);

    const char* sourceCStr = m_source.c_str();
    m_gl.glShaderSource(m_shaderID,
        1,
        &sourceCStr,
        NULL);

    m_gl.glCompileShader(m_shaderID);

    // Check that shader compiled correctly
    // What this actually does is force a shader compilation, which would otherwise
    // happen on the first render call
    // TODO: Look into glGetProgramBinary
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
        logCritical("Error, could not compile " + s_shaderTypeToStringMap[(int)m_type] + " shader");
        logCritical(errorMsg);

#ifdef DEBUG_MODE
        throw("Error, shader invalid");
#endif
    }
#ifdef DEBUG_MODE
    m_gl.printGLError("Error initializing shader");
#endif

    // Remove comments from the source
    removeComments(m_source);

    // Parse source for all info
    parseForDefines();
    parseForUniforms();
    parseForBuffers();

    //getFunctions();
    //std::vector<ShaderInputInfo> io;
    //getIO(io, io);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::parseForBuffers()
{
    // See: https://regexr.com/
    QString source = QString::fromStdString(m_source);

    // Parse for buffer sub-strings
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
    
    // Get buffer type to assign to correct vector
    ShaderBufferInfo* bufferInfo;
    const QString& bufferTypeQualifier = match.captured(4);
    if (bufferTypeQualifier.contains("uniform")) {
        m_uniformBuffers.push_back(ShaderBufferInfo());
        bufferInfo = &m_uniformBuffers.back();
        bufferInfo->m_bufferType = GL::BufferBlockType::kUniformBuffer;
    }
    else if (bufferTypeQualifier.contains("buffer")) {
        m_buffers.push_back(ShaderBufferInfo());
        bufferInfo = &m_buffers.back();
        bufferInfo->m_bufferType = GL::BufferBlockType::kShaderStorage;
    }
    else {
        throw("Invalid buffer type");
    }

    // Obtain buffer attributes ------------------------------------------------
    const QString& bufferLayout = match.captured(1);
    const QString& bufferBinding = match.captured(2);
    const QString& memQualifier = match.captured(3);
    const QString& internalName = match.captured(5);
    const QString& bufferMembers = match.captured(6);
    const QString& variableName = match.captured(7);

    // Buffer Layout
    if (bufferLayout == QStringLiteral("std140")) {
        bufferInfo->m_blockLayout = GL::ShaderBlockLayout::kStd140;
    }
    else if (bufferLayout == QStringLiteral("std430")) {
        bufferInfo->m_blockLayout = GL::ShaderBlockLayout::kStd430;
    }
    else if (bufferLayout == QStringLiteral("shared")) {
        bufferInfo->m_blockLayout = GL::ShaderBlockLayout::kShared;
    }
    else {
        throw("Error, unrecognized buffer layout");
    }

    // Binding
    if (!bufferBinding.isEmpty()) {
        bufferInfo->m_bufferBinding = match.captured(1).toInt();
    }

    // Memory qualifier
    if (!memQualifier.isEmpty()) {
        if (memQualifier == QStringLiteral("coherent")) {
            bufferInfo->m_memoryQualifier = GL::BufferMemoryQualifier::kCoherent;
        }
        else if (memQualifier == QStringLiteral("volatile")) {
            bufferInfo->m_memoryQualifier = GL::BufferMemoryQualifier::kVolatile;
        }
        else if (memQualifier == QStringLiteral("restrict")) {
            bufferInfo->m_memoryQualifier = GL::BufferMemoryQualifier::kRestrict;
        }
        else if (memQualifier == QStringLiteral("readonly")) {
            bufferInfo->m_memoryQualifier = GL::BufferMemoryQualifier::kReadOnly;
        }
        else if (memQualifier == QStringLiteral("writeonly")) {
            bufferInfo->m_memoryQualifier = GL::BufferMemoryQualifier::kWriteOnly;
        }
        else {
            throw("Error, invalid memory qualifier");
        }
    }

    // Internal buffer name
    bufferInfo->m_name = internalName;

    // Buffer variable name
    if (!variableName.isEmpty()) {
        bufferInfo->m_variableName = variableName;
    }

    // Obtain buffer fields
    //QString bufferStr = match.captured(0);
    QRegularExpression fieldLine(
        QStringLiteral("([\\w]*p[\\s\\t]+)?([\\w\\d]+)[\\s\\t]*([\\w\\d]+)[\\s\\t\\n\\r]*(?:[\\s\\t\\n\\r]*\\[([\\w\\d]*)[\\s\\t\\n\\r]*\\])?[\\s\\t\\n\\r]*(?=[;])"));
    QRegularExpressionMatchIterator i = fieldLine.globalMatch(bufferMembers);
    while (i.hasNext()) {
        QRegularExpressionMatch lineMatch = i.next();

        QString uniformName = lineMatch.captured(3);// Name
        QString uniformTypeStr = lineMatch.captured(2); // Type 
        bool isArray = !lineMatch.captured(4).isNull();// Is array
        int arraySize = -1;

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
                // Size may not be specified for buffer array
                if (!sizeStr.isEmpty()) {
                    bool ok;
                    arraySize = sizeStr.toInt(&ok);
                    if (!ok) throw("Error, array size is invalid");
                }
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

        // Type and optional type string
        ShaderVariableType uniformType;
        if (Map::HasKey(Uniform::s_uniformTypeStrMap, uniformTypeStr)) {
            uniformType = Uniform::s_uniformTypeStrMap[uniformTypeStr];
        }
        else {
            uniformType = ShaderVariableType::kCustomStruct;
        }

        // Create input info
        Vec::EmplaceBack(bufferInfo->m_fields, uniformName, uniformType, ShaderInputInfo::InputType::BufferField, isArray);
        ShaderInputInfo& field = bufferInfo->m_fields.back();
        if (field.m_variableType == ShaderVariableType::kCustomStruct) {
            field.m_variableTypeStr = uniformTypeStr;
        }
        
        // Set array size
        field.m_arraySize = arraySize;

        // Set flag that input is in buffer
        field.m_flags.setFlag(ShaderInputInfo::ShaderInputFlag::kInBlockOrBuffer, true);
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
            QString combinedName = uniformName + "." + field.name();
            Vec::EmplaceBack(m_uniforms, std::move(combinedName), field.m_variableType, ShaderInputInfo::InputType::kUniform, field.isArray());
        }
    }
    else {
        // Add as a normal uniform
        ShaderVariableType uniformType = Uniform::s_uniformTypeStrMap[uniformTypeStr];
        Vec::EmplaceBack(m_uniforms, uniformName, uniformType, ShaderInputInfo::InputType::kUniform, isArray);
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
        ShaderVariableType uniformType = Uniform::s_uniformTypeStrMap[uniformTypeStr];
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

        Vec::EmplaceBack(ss.m_fields, uniformName, uniformType, ShaderInputInfo::InputType::BufferField, isArray);
        ss.m_fields.back().m_arraySize = arraySize;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::initializeFromSource(const QString & source)
{
    m_isValid = true;

    // Prepend header based on GL version
    m_source.clear();
    auto* settings = new Gb::Settings::INISettings();
    int glMajorVersion = settings->getMajorVersion();
    int glMinorVersion = settings->getMinorVersion();
    if (!source.contains(QStringLiteral("#version"))) {
        // Prepend header to source if not yet added
        m_source = "#version " + std::to_string(glMajorVersion) +
            std::to_string(glMinorVersion) + "0";
        if (settings->getRenderingMode() == Gb::Settings::kGL_ES) {
            m_source += " es";
        }
        m_source += "\n";
        m_source += "#line 0 \n\n";
    }

    // Append source to shader
    m_source += source.toStdString();

    // Initialize shader in GL
    initializeGL();
}
/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::source() const
{
    return QString::fromStdString(m_source);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::getFunctions(std::vector<ShaderFunction>& outFunctions) const
{
    QString source = QString::fromStdString(m_source);

    // Parse for functions
    QRegularExpression re(FUNCTION_START_REGEX);
    QRegularExpressionMatchIterator i = re.globalMatch(source);
    while (i.hasNext()) {
        outFunctions.push_back(ShaderFunction());
        ShaderFunction& function = outFunctions.back();

        QRegularExpressionMatch match = i.next();
        function.m_definition = match.captured(0);
        function.m_definitionStart = match.captured(0);
        function.m_returnType = Uniform::s_uniformTypeStrMap[match.captured(1)];
        function.m_name = match.captured(2);
        function.m_body = "";

        // Find beginning of function in source, and iterate through brackets to find closing
        // Regex is not suited for finding matching sets of brackets
        int functionIndex = match.capturedEnd(); // Get index directly after capture in source
        if (functionIndex == -1) {
            throw("Error, function not found in source");
        }

        size_t openCount = 1;
        bool done = false;
        for (int i = functionIndex; i < source.length(); i++) {
            QChar c = source.at(i);

            if (c == QStringLiteral("{")) {
                openCount++;
            }
            else if (c == QStringLiteral("}")) {
                openCount--;
                if (openCount == 0) {
                    // If closing bracket matches function open, then we're done
                    done = true;
                }
            }

            if (!done) {
                function.m_body += c;
            }

            function.m_definition += c;

            if (done) {
                break;
            }
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::getFunctions(tsl::robin_map<QString, ShaderFunction>& outFunctions) const
{
    QString source = QString::fromStdString(m_source);

    // Parse for functions
    QRegularExpression re(FUNCTION_START_REGEX);
    QRegularExpressionMatchIterator i = re.globalMatch(source);
    while (i.hasNext()) {
        ShaderFunction function;

        QRegularExpressionMatch match = i.next();
        function.m_definition = match.captured(0);
        function.m_returnType = Uniform::s_uniformTypeStrMap[match.captured(1)];
        function.m_name = match.captured(2);

        // Find beginning of function in source, and iterate through brackets to find closing
        // Regex is not suited for finding matching sets of brackets
        int functionIndex = match.capturedEnd(); // Get index directly after capture in source
        if (functionIndex == -1) {
            throw("Error, function not found in source");
        }

        size_t openCount = 1;
        bool done = false;
        for (int i = functionIndex; i < source.length(); i++) {
            QChar c = source.at(i);

            if (c == QStringLiteral("{")) {
                openCount++;
            }
            else if (c == QStringLiteral("}")) {
                openCount--;
                if (openCount == 0) {
                    // If closing bracket matches function open, then we're done
                    done = true;
                }
            }
            function.m_definition += c;

            if (done) {
                break;
            }
        }

        outFunctions[function.m_name] = function;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shader::getIO(std::vector<ShaderInputInfo>& inputs, std::vector<ShaderInputInfo>& outputs) const
{
    QString source = QString::fromStdString(m_source);

    // Parse for inputs
    QRegularExpression re(IN_SHADER_REGEX);
    QRegularExpressionMatchIterator i = re.globalMatch(source);
    while (i.hasNext()) {
        inputs.push_back(ShaderInputInfo());
        ShaderInputInfo& io = inputs.back();
        io.m_inputType = ShaderInputInfo::InputType::kIn;

        QRegularExpressionMatch match = i.next();
        io.m_variableType = Uniform::s_uniformTypeStrMap[match.captured(2)];
        io.m_name = match.captured(3);

        // If is an array
        if (!match.captured(4).isNull()) {
            io.m_flags.setFlag(ShaderInputInfo::ShaderInputFlag::kIsArray, true);

            if (!match.captured(5).isEmpty()) {
                // Set array parameters in in/out
                io.m_arraySize = match.captured(5).toDouble();
            }
        }
    }

    // Parse for outputs
    re = QRegularExpression(OUT_SHADER_REGEX);
    i = re.globalMatch(source);
    while (i.hasNext()) {
        outputs.push_back(ShaderInputInfo());
        ShaderInputInfo& io = outputs.back();
        io.m_inputType = ShaderInputInfo::InputType::kOut;

        QRegularExpressionMatch match = i.next();
        io.m_variableType = Uniform::s_uniformTypeStrMap[match.captured(2)];
        io.m_name = match.captured(3);

        // If is an array
        if (!match.captured(4).isNull()) {
            io.m_flags.setFlag(ShaderInputInfo::ShaderInputFlag::kIsArray, true);

            if (!match.captured(5).isEmpty()) {
                // Set array parameters in in/out
                io.m_arraySize = match.captured(5).toDouble();
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::UNIFORM_REGEX = QStringLiteral("uniform[\\s\\t\\r\\n]*([\\w]*p[\\s\\t\\r\\n]+)?([\\w\\d]+)[\\s\\t\\r\\n]*([\\w\\d]+)[\\s\\t\\r\\n]*(\\[[\\w\\d]+\\])?(?=[;])");
// WIP, still need to capture all layout arguments properly
// (?:layout[\s\t\r\n]*\((?:[\s\t\r\n]*(\w+)[\s\t\r\n]*=[\s\t\r\n]*([\d]+)[\s\t\r\n]*,*)+\)[\s\t\r\n]+)?uniform[\s\t\r\n]*([\w]*p[\s\t\r\n]+)?([\w\d]+)[\s\t\r\n]*([\w\d]+)[\s\t\r\n]*(\[[\w\d]+\])?(?=[;])

/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::UNIFORM_BUFFER_REGEX = QStringLiteral("layout[\\s\\t\\n\\r]*\\([\\w\\d]+\\)[\\s\\t\\n\\r]*uniform[\\s\\t\\n\\r]+([\\w\\d]+)[\\s\\t\\n\\r]*[\\s\\t\\n\\r]*\\{([\\w\\d\\s\\t\\n\\r;\\/\\,\\[\\]]*)\\}");

/////////////////////////////////////////////////////////////////////////////////////////////
// Group 1: block layout
// Group 2: buffer binding
// Group 3: buffer memory qualifiers, e.g. readonly
// Group 4: buffer or uniform qualifier
// Group 5: internal buffer name, e.g. LightBuffer
// Group 6: members
// Group 7: GLSL prefix name, e.g. lightBuffer
QString Shader::BUFFER_REGEX = QStringLiteral("layout[\\s\\t\\n\\r]*\\(([\\w\\d]+)[\\s\\t\\n\\r]*,*[\\s\\t\\n\\r]*(?:binding)*[\\s\\t\\n\\r]*=*[\\s\\t\\n\\r]*(\\d*)\\)[\\s\\t\\n\\r]*([\\w\\d]*)[\\s\\t\\n\\r]*(uniform|buffer)[\\s\\t\\n\\r]+([\\w\\d]+)[\\s\\t\\n\\r]*[\\s\\t\\n\\r]*\\{([\\w\\d\\s\\t\\n\\r;\\/\\,\\[\\]]*)\\}[\\s\\t\\n\\r]*([\\w\\d]*)[\\s\\t\\n\\r]*;");

/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::STRUCT_REGEX = QStringLiteral("struct[\\s\\t\\n\\r]*([\\w\\d]+)[\\s\\t\\n\\r]*\\{([\\w\\s\\d\\t\\n\\r;\\/\\,\\.\\[\\]]*)\\}");

/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::COMMENT_REGEX = QStringLiteral("\\/\\/(.)*(?=[\\r\\n])");

/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::DEFINE_REGEX = QStringLiteral("#[\\s\\t\\r\\n]*define[\\s\\t\\r\\n]*([\\w\\d]+)[\\s\\t\\r\\n]*([\\w\\d]+)");

/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::FUNCTION_START_REGEX = QStringLiteral("(?:[\\n\\s\\t]+)([\\w\\d]+)[\\s\\t]+\\n*(((?!if|else|do|while|for).)+)[\\s\\t\\n]*\\([\\s\\t\\n]*.*[\\s\\t\\n]*\\)[\\s\\t\\n]*\\{");

/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::OUT_SHADER_REGEX = QStringLiteral("(out)[\\s\\t]+\\n*([\\w\\d]+)[\\s\\t]+\\n*([\\w\\d]+)[\\s\\t\\n]*(\\[[\\s\\t\\n]*(\\w\\d)*[\\s\\t\\n]*\\])?[\\s\\t\\n]*;");

/////////////////////////////////////////////////////////////////////////////////////////////
QString Shader::IN_SHADER_REGEX = QStringLiteral("(in)[\\s\\t]+\\n*([\\w\\d]+)[\\s\\t]+\\n*([\\w\\d]+)[\\s\\t\\n]*(\\[[\\s\\t\\n]*(\\w\\d)*[\\s\\t\\n]*\\])?[\\s\\t\\n]*;");




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
bool ShaderProgram::isBuiltIn(const QString & name)
{
    auto iter = std::find_if(Shader::s_builtins.begin(),
        Shader::s_builtins.end(),
        [&](const std::pair<QString, Shader::ShaderType>& shaderPair) {
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
    m_gl(shaderProgram.m_gl),
    m_uniformQueue(shaderProgram.m_uniformQueue),
    m_uniforms(shaderProgram.m_uniforms),
    m_uniformInfo(shaderProgram.m_uniformInfo)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(const QJsonValue & json):
    Resource()
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(const QString& vertfile, const QString& fragfile) :
    Resource()
{
    if (!FileReader::fileExists(vertfile)) {
        throw("Error, vertex shader has invalid filepath and is required to link shader program");
    }
    if (!FileReader::fileExists(fragfile)) {
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

    setName();

    initializeShaderProgram();
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(const QString& name, 
    const QString & vertSource,
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

    m_name = name;

    initializeShaderProgram();
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(const QString& compFile) :
    Resource()
{
    if (!FileReader::fileExists(compFile)) {
        throw("Error, compute shader has invalid filepath and is required to link shader program");
    }

    Vec::Emplace(m_shaders, m_shaders.end(), compFile, Shader::ShaderType::kCompute, false);

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
ShaderProgram::ShaderProgram(const QString & vertfile,
    const QString & fragfile, 
    const QString & geometryFile) :
    Resource()
{
    // Check shader files
    if (!FileReader::fileExists(vertfile)) {
        throw("Error, vertex shader has invalid filepath and is required to link shader program");
    }
    if (!FileReader::fileExists(fragfile)) {
        throw("Error, fragment shader has invalid filepath and is required to link shader program");
    }

    // Create any shaders
    Vec::Emplace(m_shaders, m_shaders.end(), vertfile, Shader::ShaderType::kVertex, true);
    Vec::Emplace(m_shaders, m_shaders.end(), fragfile, Shader::ShaderType::kFragment, true);
    if (!geometryFile.isEmpty()) {
        if (FileReader::fileExists(geometryFile)) {
            Vec::Emplace(m_shaders, m_shaders.end(), geometryFile, Shader::ShaderType::kGeometry, true);
        }
    }

    // Initialize individual shaders in GL
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
    m_gl.glUseProgram(m_programID);

    // Bind buffers
    //for (GLBuffer* buffer : m_buffers) {
    //    if (buffer) {
    //        buffer->bindToPoint();
    //    }
    //}

    if (m_bufferUsageFlags.testFlag(BufferUsageFlag::kLightBuffer)) {
        // TODO: Pass in context so this isn't necessary
        CoreEngine* engine = CoreEngine::engines().begin()->second;
        RenderContext& context = engine->mainRenderer()->renderContext();
        ShaderStorageBuffer& lightBuffer = context.lightingSettings().lightBuffers().readBuffer();
        lightBuffer.bindToPoint();
    }

    if (m_bufferUsageFlags.testFlag(BufferUsageFlag::kShadowBuffer)) {
        // TODO: Pass in context so this isn't necessary
        CoreEngine* engine = CoreEngine::engines().begin()->second;
        RenderContext& context = engine->mainRenderer()->renderContext();
        ShaderStorageBuffer& shadowBuffer = context.lightingSettings().shadowBuffers().readBuffer();
        shadowBuffer.bindToPoint();
    }

    s_boundShader = this;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::release()
{
    m_gl.glUseProgram(0);

    //for (GLBuffer* buffer : m_buffers) {
    //    if (buffer) {
    //        buffer->releaseFromPoint();
    //    }
    //}

    if (m_bufferUsageFlags.testFlag(BufferUsageFlag::kLightBuffer)) {
        // TODO: Pass in context so this isn't necessary
        CoreEngine* engine = CoreEngine::engines().begin()->second;
        RenderContext& context = engine->mainRenderer()->renderContext();
        ShaderStorageBuffer& lightBuffer = context.lightingSettings().lightBuffers().readBuffer();
        lightBuffer.releaseFromPoint();
    }

    if (m_bufferUsageFlags.testFlag(BufferUsageFlag::kShadowBuffer)) {
        // TODO: Pass in context so this isn't necessary
        CoreEngine* engine = CoreEngine::engines().begin()->second;
        RenderContext& context = engine->mainRenderer()->renderContext();
        ShaderStorageBuffer& shadowBuffer = context.lightingSettings().shadowBuffers().readBuffer();
        shadowBuffer.releaseFromPoint();
    }

    s_boundShader = nullptr;
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
QJsonValue ShaderProgram::asJson() const
{
    QJsonObject object;

    // Add shaders to json
    QJsonObject shaders;
    for (const auto& shader : m_shaders) {
        shaders.insert(QString::number((size_t)shader.m_type), shader.asJson());
    }
    object.insert("shaders", shaders);

    // Set name
    object.insert("name", m_name.c_str());

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderProgram::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    const QJsonObject& object = json.toObject();

    // Load shaders from JSON
    QJsonObject shaders = object["shaders"].toObject();
    std::vector<QString> sortedKeys = shaders.keys().toVector().toStdVector();
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
    m_gl.glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);

#ifdef DEBUG_MODE
    bool error = m_gl.printGLError("Failed to dispatch compute");
    if (error) {
        throw("Error, failed to dispatch compute");
    }
#endif

    // Ensure that writes are recognized by other OpenGL operations
    // TODO: Use this for other SSB writes, although those are fine if accessed within same shader
    // See: https://www.khronos.org/opengl/wiki/Memory_Model (incoherent memory access)
    m_gl.glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

#ifdef DEBUG_MODE
    error = m_gl.printGLError("Failed at memory barrier");
    if (error) {
        throw("Error, failed at memory barrier");
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
Vector3i ShaderProgram::getMaxWorkGroupCounts()
{
    Vector3i counts;
    m_gl.glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &counts[0]);
    m_gl.glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &counts[1]);
    m_gl.glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &counts[2]);

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
        if (!uniform.isValid()) { 
            logWarning("Received invalid uniform " + uniform.getName());
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
                if (m_name != "debug_skeleton") {
                    //for (const auto& key_value : m_uniformInfo) {
                    //    keys.push_back(key_value.first);
                    //}
                    std::vector<GStringView> keys;
                    for (const auto& kv : m_uniformInfo) {
                        keys.push_back(kv.first);
                    }
                    // Debug skeleton has some unused uniforms that raise a warning, fix this
                    logWarning("Error, uniform not recognized: " + QString(uniformName));
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
    for (unsigned int i = 0; i < size; i++) {
        // Set uniform value
        m_gl.glUniformMatrix4fv(uniformID + i, 1, GL_FALSE, value[i].m_mtx[0].data()); // uniform ID, count, transpose, value
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
// Template specialization for setting uniforms of an array of vec3s
template<>
void ShaderProgram::setUniformValueGL(const GStringView& uniformName, const Vec3List & value)
{
    // Get ID of start of array
    GLuint uniformID = getUniformID(uniformName);

    size_t size = value.size();
    for (unsigned int i = 0; i < size; i++) {
        // Set uniform value
        m_gl.glUniform3fv(uniformID + i, 1, &value[i][0]);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
template<>
void ShaderProgram::setUniformValueGL(const GStringView& uniformName, const Vec4List & value)
{    
    //GLuint uniformID = getUniformID(uniformName);
    //m_gl.glUniform3fv(uniformID, value.size(), &value[0][0]); // uniform ID, count, value

    GLuint uniformID = getUniformID(uniformName);

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

//#ifdef DEBUG_MODE
//    bool error = m_gl.printGLError(QStringLiteral("Error linking program");
//    if (error) {
//        throw("Error linking program");
//    }
//#endif

    // Print error if shaders failed to load
    int status;
    m_gl.glGetProgramiv(m_programID, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        logCritical("Caught error, Failed to link shader program");

        GLint maxNumUniformVector4s;
        m_gl.glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxNumUniformVector4s);

        GLint maxLength = 0;
        m_gl.glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<char> errorChar(maxLength);
        m_gl.glGetProgramInfoLog(m_programID, maxLength, &maxLength, errorChar.data());
        std::string errorStr(std::begin(errorChar), std::end(errorChar));
        QString errorMsg = QString::fromStdString(errorStr);
        logError("Failed to link shader program: " + errorMsg);

#ifdef DEBUG_MODE
        throw("Error, failed to link shader program");
#endif
    }

    // Validate program
    m_gl.glValidateProgram(m_programID);
    m_gl.glGetProgramiv(m_programID, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE)
    {
        logCritical("Caught error, failed to validate shader program");
        m_gl.printGLError("Failed to validate shader program");
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
void ShaderProgram::detachShader(const Shader & shader)
{
    m_gl.glDetachShader(m_programID, shader.getID());
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
        QString nonArrayName = QString(info.name()).split("[")[0];

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
            if (isBuiltIn(m_name)) {
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
GLint ShaderProgram::getNumActiveBuffers(GL::BufferBlockType bufferType)
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