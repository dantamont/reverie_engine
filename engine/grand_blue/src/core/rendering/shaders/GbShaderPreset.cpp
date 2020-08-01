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
QJsonValue ShaderPreset::asJson() const
{
    QJsonObject object = Serializable::asJson().toObject();
    object.insert("name", m_name);
    object.insert("renderer", m_renderer.asJson());

    if (m_shaderProgram) {
        object.insert("shaderProgram", m_shaderProgram->getName());
    }

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShaderPreset::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();

    if (object.contains("renderer")) {
        QJsonObject rendererObject = object["renderer"].toObject();
        m_renderer.loadFromJson(rendererObject);
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
}


/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}