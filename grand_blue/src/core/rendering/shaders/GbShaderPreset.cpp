#include "GbShaderPreset.h"

// Qt
#include <QDebug>

// internal
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../../processes/GbProcess.h"
#include "../../readers/GbJsonReader.h"
#include "../GbGLFunctions.h"
#include "../shaders/GbShaders.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// ShaderPreset
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderPreset::ShaderPreset(CoreEngine * core, const QJsonValue & json) :
    m_engine(core)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderPreset::ShaderPreset(CoreEngine* core, const QString& name):
    Object(name),
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
QJsonValue ShaderPreset::asJson() const
{
    QJsonObject object = Shadable::asJson().toObject();
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
        object.insert("shaderProgram", m_shaderProgram->getName().c_str());
    }

    if (m_prepassShaderProgram) {
        object.insert("prepassShaderProgram", m_prepassShaderProgram->getName().c_str());
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
        QJsonObject uniformObject = uniforms.value(key).toObject();
        Vec::EmplaceBack(m_uniforms, Uniform(uniformObject));

        // Set uniform name again, since GStringView needs a persistent string
        m_uniforms.back().setName(m_uniformNames.back());
    }

    if (object.contains("name")) {
        m_name = object["name"].toString();
    }
    else {
        // Legacy
        m_name = m_uuid.createUniqueName("preset_");
    }

    if (object.contains("shaderProgram")) {
        QString shaderProgramName = object["shaderProgram"].toString();
        m_shaderProgram = m_engine->resourceCache()->getHandleWithName(
            shaderProgramName, Resource::kShaderProgram)->resourceAs<ShaderProgram>();
        if (!m_shaderProgram) {
            throw("Error, shader program with name " + shaderProgramName + " not found");
        }
    }

    if (object.contains("prepassShaderProgram")) {
        QString shaderProgramName = object["prepassShaderProgram"].toString();
        m_prepassShaderProgram = m_engine->resourceCache()->getHandleWithName(
            shaderProgramName, Resource::kShaderProgram)->resourceAs<ShaderProgram>();
        if (!m_prepassShaderProgram) {
            throw("Error, prepass shader program with name " + shaderProgramName + " not found");
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}