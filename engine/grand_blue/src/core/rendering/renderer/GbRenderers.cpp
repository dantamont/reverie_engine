#include "GbRenderers.h"

#include <typeinfo> 

#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../models/GbModel.h"
#include "../geometry/GbBuffers.h"
#include "../shaders/GbShaders.h"
#include "../materials/GbMaterial.h"
#include "../materials/GbCubeMap.h"
#include "../../components/GbCanvasComponent.h"
#include "../../components/GbModelComponent.h"

namespace Gb {   

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

Renderer::Renderer():
    OpenGLFunctions(),
    m_shaderProgram(nullptr)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Renderer::Renderer(CoreEngine* engine) :
    OpenGLFunctions(),
    m_engine(engine),
    m_shaderProgram(nullptr)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Renderer::~Renderer()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderer::draw(const std::vector<Renderable*>& renderables)
{
    if (!m_shaderProgram) {
        logWarning("Warning, no default shader program assigned to render m_data");
    }
    else if (!m_engine->resourceCache()->hasShaderProgram(m_shaderProgram->getName())) {
        m_shaderProgram = nullptr;
        logWarning("Warning, renderer's shader program removed from resource cache");
    }
    else {
        // Render with default shader program
        draw(renderables, m_shaderProgram);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderer::draw(const std::vector<Renderable*>& renderables, 
    const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    // Toggle GL Settings
    m_renderSettings.bind();

    // Bind shader
    if (m_renderSettings.hasShaderFlag(RenderSettings::kBind)) {
        shaderProgram->bind();
    }

    // Set shader uniforms
    bindUniforms(shaderProgram);
    shaderProgram->updateUniforms();

    // Render models
    std::vector<Renderable*>::const_iterator iR;
    for (iR = renderables.begin(); iR != renderables.end(); iR++) {
        (*iR)->draw(shaderProgram, &m_renderSettings);
    }

    // Release shader uniforms
    releaseUniforms(shaderProgram);

    // Release shader
    if (m_renderSettings.hasShaderFlag(RenderSettings::kRelease)) {
        shaderProgram->release();
    }

    // Untoggle GL settings
    m_renderSettings.release();
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Renderer::asJson() const
{
    QJsonObject object;
    object.insert("renderSettings", m_renderSettings.asJson());
    if (m_shaderProgram) {
        object.insert("defaultShaderProgram", m_shaderProgram->getName());
    }
    object.insert("sortingLayer", m_renderLayer.asJson());
    
    // Cache uniforms used by the renderer
    QJsonObject uniforms;
    for (const auto& uniformPair : m_uniforms) {
        uniforms.insert(uniformPair.first, uniformPair.second.asJson());
    }
    object.insert("uniforms", uniforms);

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderer::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();
    m_renderSettings = RenderSettings(object["renderSettings"]);
    m_renderLayer = SortingLayer(object["sortingLayer"]);

    // Load default shader used by the renderer
    if (object.contains("defaultShaderProgram")) {
        QString shaderProgramName = object["defaultShaderProgram"].toString();
        m_shaderProgram = m_engine->resourceCache()->getShaderProgramByName(shaderProgramName);
        if (!m_shaderProgram) {
            throw("Error, shader program with name " + shaderProgramName + " not found");
        }
    }

    // Load uniforms used by the renderer
    const QJsonObject& uniforms = object["uniforms"].toObject();
    for (const QString& key : uniforms.keys()) {
        QJsonObject uniformObject = uniforms.value(key).toObject();
        Map::Emplace(m_uniforms, key, uniformObject);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void Renderer::bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    Renderable::bindUniforms(shaderProgram);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderer::releaseUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    Q_UNUSED(shaderProgram)
}

/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}