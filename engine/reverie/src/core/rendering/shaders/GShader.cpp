#include "GShader.h"

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


namespace rev {   
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
ShaderInputInfo::ShaderInputInfo(const GString & name, const ShaderVariableType & type, InputType inputType) :
    m_name(name),
    m_variableType(type),
    m_inputType(inputType),
    m_variableCType(Uniform::s_uniformGLTypeMap.contains(type) ?
        Uniform::s_uniformGLTypeMap.at(type) : typeid(nullptr))
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderInputInfo::ShaderInputInfo(const GString & name, const ShaderVariableType & type, InputType inputType, bool isArray) :
    ShaderInputInfo(name, type, inputType)
{
    m_flags.setFlag(kIsArray, isArray);
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderInputInfo::ShaderInputInfo(const GString & name, const ShaderVariableType & type, InputType inputType, bool isArray, int id) :
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
    {"canvas_gui", Shader::ShaderType::kVertex}, // TODO: Fix, loaded manually
    {"canvas_billboard", Shader::ShaderType::kVertex}, // TODO: Fix, loaded manually
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
GString Shader::CombineEffectFragmentShaders(std::vector<const Shader*>& shaders)
{
    // Assumptions:
    // *Input texture coordinates are described by "in vec2 texCoords;"
    // *Texture samplers are called screenTexture, depthTexture, and stencilTexture

    static GString boilerPlate("#ifdef GL_ES\n precision mediump int;\n precision mediump float;\n#endif\n");

    GString outSource = boilerPlate;
    
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
    tsl::robin_map<GString, int> functionMap;
    for (size_t i = 0; i < functions.size(); i++) {
        ShaderFunction& function = functions[i];

        // Skip main, append to list of main functions
        if (function.m_name == QStringLiteral("main")) {
            mainFunctions.push_back(function);
            continue;
        }

        if (Map::HasKey(functionMap, function.m_name)) {
            // Renaming duplicate functions
            const GString& oldName = function.m_name;
            const GString& oldDefinitionStart = function.m_definitionStart;
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
        const GString& localOutputName = outputVec[i].name();
        function.m_body.replace(localOutputName, outputName);

        // Replace texture sampling with reference to fColor so that
        // shader effects chain together instead of overwriting one another
        QRegularExpression re(screenTexRegStr);
        QRegularExpressionMatchIterator j = re.globalMatch(function.m_body.c_str());
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
GString Shader::CombineEffectVertexShaders(std::vector<const Shader*>& shaders)
{
    // Assumptions:
    // Vertex shaders do not make any modifications to gl_position, and do not
    // modify the texCoords output variable

    // Initialize quad shader's basic components
    GString outSource;
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
    tsl::robin_map<GString, ShaderInputInfo> outputs = {
        {texCoords,
        {texCoords, ShaderVariableType::kVec2, ShaderInputInfo::InputType::kOut }}
    }; // Ensure that texCoords are an output to the fragment shader
    std::vector<ShaderInputInfo> inputVec;
    std::vector<ShaderInputInfo> outputVec;
    tsl::robin_map<GString, ShaderInputInfo> uniforms;
    tsl::robin_map<GString, ShaderBufferInfo> buffers;
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
        if (ignoredUniforms.contains(uniformPair.first.c_str())) {
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
    tsl::robin_map<GString, int> functionMap;
    for (size_t i = 0; i < functions.size(); i++) {
        ShaderFunction& function = functions[i];

        // Skip main, append to list of main functions
        if (function.m_name == QStringLiteral("main")) {
            mainFunctions.push_back(function);
            continue;
        }

        if (Map::HasKey(functionMap, function.m_name)) {
            // Renaming duplicate functions
            const GString& oldName = function.m_name;
            const GString& oldDefinitionStart = function.m_definitionStart;
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
    m_name = FileReader::PathToName(m_path.c_str(), false);
    if (!deferConstruction) {
        initializeFromSourceFile();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
Shader::~Shader()
{
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();

#ifdef DEBUG_MODE
    bool error = gl.printGLError("Error before deleting shader");
    if (error) {
        Logger::LogError("Error deleting shader " + getName());
    }
#endif

    if (m_shaderID > -1) {
        gl.glDeleteShader(m_shaderID);
    }

#ifdef DEBUG_MODE
    error = gl.printGLError("Error deleting shader");
    if (error) {
        Logger::LogError("Error deleting shader" + getName());
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Shader::asJson(const SerializationContext& context) const
{
    QJsonObject object = Loadable::asJson(context).toObject();
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
        m_name = FileReader::PathToName(m_path.c_str(), false);
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
    rev::FileReader fileReader(m_path.c_str());
    QStringList fileLines = fileReader.getFileLines();
    if (!fileLines.size()) {
        m_isValid = false;
        return false;
    }
    else {
        m_isValid = true;
    }

    // Prepend header based on GL version
    auto* settings = new rev::Settings::INISettings();
    int glMajorVersion = settings->getMajorVersion();
    int glMinorVersion = settings->getMinorVersion();
    m_source = "#version " + std::to_string(glMajorVersion) + 
        std::to_string(glMinorVersion) + "0";
    if (settings->getRenderingMode() == rev::Settings::kGL_ES) {
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
    GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    m_shaderID = gl.glCreateShader(s_shaderTypeToGLMap[(int)m_type]);

    const char* sourceCStr = m_source.c_str();
    gl.glShaderSource(m_shaderID,
        1,
        &sourceCStr,
        NULL);

    gl.glCompileShader(m_shaderID);

    // Check that shader compiled correctly
    // What this actually does is force a shader compilation, which would otherwise
    // happen on the first render call
    // TODO: Look into glGetProgramBinary
    int status;
    gl.glGetShaderiv(m_shaderID, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        // If not compiled correctly, print info log
        GLint maxLength = 0;
        gl.glGetShaderiv(m_shaderID, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<char> errorChar(maxLength);
        gl.glGetShaderInfoLog(m_shaderID, maxLength, &maxLength, errorChar.data());
        std::string errorStr(std::begin(errorChar), std::end(errorChar));
        QString errorMsg = QString::fromStdString(errorStr);
        logCritical("Error, could not compile " + s_shaderTypeToStringMap[(int)m_type] + " shader");
        logCritical(errorMsg);

#ifdef DEBUG_MODE
        throw("Error, shader invalid");
#endif
    }
#ifdef DEBUG_MODE
    gl.printGLError("Error initializing shader");
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
            std::string nameStd = uniformName.toStdString();
            GString combinedName = GString(nameStd.c_str()) + "." + field.name();
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
    auto* settings = new rev::Settings::INISettings();
    int glMajorVersion = settings->getMajorVersion();
    int glMinorVersion = settings->getMinorVersion();
    if (!source.contains(QStringLiteral("#version"))) {
        // Prepend header to source if not yet added
        m_source = "#version " + std::to_string(glMajorVersion) +
            std::to_string(glMinorVersion) + "0";
        if (settings->getRenderingMode() == rev::Settings::kGL_ES) {
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
                function.m_body += GString(c);
            }

            function.m_definition += GString(c);

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
            function.m_definition += GString(c);

            if (done) {
                break;
            }
        }

        outFunctions[function.m_name.c_str()] = function;
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
// End namespacing
}