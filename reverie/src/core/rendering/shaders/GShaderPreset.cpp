#include "GShaderPreset.h"

// Qt
#include <QDebug>

// internal
#include "../../GCoreEngine.h"
#include "../../resource/GResourceCache.h"
#include "../../processes/GProcess.h"
#include "../../readers/GJsonReader.h"
#include "../GGLFunctions.h"
#include "../shaders/GShaderProgram.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// ShaderPreset
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ShaderPreset> ShaderPreset::GetBuiltin(const GString & name)
{
    auto iter = std::find_if(s_builtins.begin(), s_builtins.end(),
        [&](const std::shared_ptr<ShaderPreset>& preset) {
        return preset->getName() == name;
    });

    bool hasPreset = iter != s_builtins.end();
    if (hasPreset) {
        return *iter;
    }
    else {
        return nullptr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderPreset::InitializeBuiltins(CoreEngine* engine)
{
    s_builtins.clear();

    static GString canvasGui = "canvas_gui";
    static GString canvasBillboard = "canvas_billboard";

    // Create canvas gui preset and set shader
    std::shared_ptr<ShaderPreset> canvasGuiPreset = std::make_shared<ShaderPreset>(engine, canvasGui);
    auto canvasGuiShader = engine->resourceCache()->getHandleWithName(
        canvasGui, ResourceType::kShaderProgram)->resourceAs<ShaderProgram>();
    canvasGuiPreset->setShaderProgram(canvasGuiShader);
    s_builtins.push_back(canvasGuiPreset);

    // Create billboard preset and set shader 
    std::shared_ptr<ShaderPreset> canvasBillboardPreset = std::make_shared<ShaderPreset>(engine, canvasBillboard);
    auto canvasBillboardShader = engine->resourceCache()->getHandleWithName(
        canvasBillboard, ResourceType::kShaderProgram)->resourceAs<ShaderProgram>();
    canvasBillboardPreset->setShaderProgram(canvasBillboardShader);
    s_builtins.push_back(canvasBillboardPreset);
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderPreset::ShaderPreset(CoreEngine * core, const QJsonValue & json) :
    m_engine(core)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderPreset::ShaderPreset(CoreEngine* core, const GString& name):
    Nameable(name),
    m_engine(core)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderPreset::~ShaderPreset()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderPreset::queueUniforms()
{
    for (const Uniform& uniform : m_uniforms) {
        m_shaderProgram->setUniformValue(uniform, false);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ShaderPreset::asJson(const SerializationContext& context) const
{
    QJsonObject object = Shadable::asJson(context).toObject();
    object.insert("name", m_name.c_str());
    //object.insert("renderer", m_renderer.asJson());

    // Cache uniforms used by the preset
    QJsonObject uniforms;
    for (const Uniform& uniform : m_uniforms) {
#ifdef DEBUG_MODE
        if (uniform.getName().isEmpty()) {
            throw("Error, no uniform name");
        }
#endif
        uniforms.insert(uniform.getName(), uniform.asJson());
    }
    object.insert("uniforms", uniforms);

    if (m_shaderProgram) {
        object.insert("shaderProgram", m_shaderProgram->handle()->getName().c_str());
    }

    if (m_prepassShaderProgram) {
        object.insert("prepassShaderProgram", m_prepassShaderProgram->handle()->getName().c_str());
    }

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderPreset::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    QJsonObject object = json.toObject();

    if (object.contains("renderer")) {
        // Legacy, deprecated
        QJsonObject rendererObject = object["renderer"].toObject();
        Shadable::loadFromJson(rendererObject);
    }
    else {
        Shadable::loadFromJson(object);
    }

    // Load uniforms used by the preset
    m_uniforms.clear();
    m_uniformNames.clear();
    const QJsonObject& uniforms = object["uniforms"].toObject();
    for (const QString& key : uniforms.keys()) {
        m_uniformNames.push_back(key); // Copy as GString
#ifdef DEBUG_MODE
        if (uniforms.value(key).isNull()) {
            throw("Error, uniform not found");
        }
        else if (!uniforms.value(key).isObject()) {
            throw("Error, uniform value is not an object");
        }
#endif
        QJsonObject uniformObject = uniforms.value(key).toObject();

        Vec::EmplaceBack(m_uniforms, Uniform(uniformObject));

        // Set uniform name again, since GStringView needs a persistent string
        m_uniforms.back().setName(m_uniformNames.back());
    }

    if (object.contains("name")) {
        m_name = object["name"].toString();
    }

    if (object.contains("shaderProgram")) {
        QString shaderProgramName = object["shaderProgram"].toString();
        std::shared_ptr<ResourceHandle> handle = m_engine->resourceCache()->getHandleWithName(
            shaderProgramName, ResourceType::kShaderProgram);
        if (!handle) {
            // FIXME: 2/9/2021 Noticed when switching scenarios, but not duplicable
            throw("Error, handle to shader program " + shaderProgramName + " not found");
        }
        m_shaderProgram = handle->resourceAs<ShaderProgram>();
        if (!m_shaderProgram) {
            throw("Error, shader program with name " + shaderProgramName + " not found");
        }
    }

    if (object.contains("prepassShaderProgram")) {
        QString shaderProgramName = object["prepassShaderProgram"].toString();
        m_prepassShaderProgram = m_engine->resourceCache()->getHandleWithName(
            shaderProgramName, ResourceType::kShaderProgram)->resourceAs<ShaderProgram>();
        if (!m_prepassShaderProgram) {
            throw("Error, prepass shader program with name " + shaderProgramName + " not found");
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::shared_ptr<ShaderPreset>> ShaderPreset::s_builtins;

/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}